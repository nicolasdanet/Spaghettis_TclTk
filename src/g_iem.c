
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"
#include "g_iem.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *panel_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol *iemgui_expandDollar (t_iem *iem, t_symbol *s)
{
    t_symbol *t = canvas_expandDollar (iem, s); return (t == NULL ? utils_empty() : t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int iemgui_colorRGB (int argc, t_atom *argv)
{
    unsigned int r = (unsigned int)atom_getFloatAtIndex (0, argc, argv);
    unsigned int g = (unsigned int)atom_getFloatAtIndex (1, argc, argv);
    unsigned int b = (unsigned int)atom_getFloatAtIndex (2, argc, argv);
    
    r = PD_CLAMP (r, 0, 255);
    g = PD_CLAMP (g, 0, 255);
    b = PD_CLAMP (b, 0, 255);
        
    return (int)(IEM_COLOR_MASK & ((r << 16) | (g << 8) | b));
}

static t_symbol *iemgui_colorEncode (int color)
{
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "#%06x", color);
    return gensym (t);
}

/* Ensure compatibility with the original serialization format. */
/* By the way legacy predefined colors are not supported. */
/* Only the 6 MSB are kept for each component. */

// RRRRRRRRGGGGGGGGBBBBBBBB 
// RRRRRR..GGGGGG..BBBBBB..
// RRRRRRGGGGGGBBBBBB

static int iemgui_colorDecodeFromInteger (int color)
{
    PD_ASSERT (color < 0);
    
    int n = 0;
    
    color = (-1 -color);
    
    n |= ((color & 0x3f000) << 6);
    n |= ((color & 0xfc0) << 4);
    n |= ((color & 0x3f) << 2);

    return n;
}

static int iemgui_colorDecode (t_atom *a)
{
    if (IS_FLOAT (a))  { return iemgui_colorDecodeFromInteger ((int)GET_FLOAT (a)); }
    if (IS_SYMBOL (a)) {
        t_symbol *s = atom_getSymbol (a);
        if (s->s_name[0] == '#') { return strtol (s->s_name + 1, NULL, 16); }
    }
    
    PD_BUG;
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* For convenience floats are loaded as integers (mainly in order to enumerate things). */

static t_symbol *iemgui_fetchName (int i, t_atom *argv)
{
    if (IS_SYMBOL (argv + i))     { return (atom_getSymbol (argv + i)); }
    else if (IS_FLOAT (argv + i)) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, PD_STRING, "%d", (int)atom_getFloat (argv + i));
        return gensym (t);
    } else {
        return utils_empty();
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Unexpanded names cannot be fetch at instantiation due to already substituted arguments. */
/* Consequently it must be done by looking up again the object's text buffer. */

static void iemgui_fetchUnexpanded (t_iem *iem, t_symbol **s, int i, t_symbol *fallback)
{
    if (!*s) {
        t_error err = PD_ERROR;
        t_buffer *b = cast_object (iem)->te_buffer;
        if (i < buffer_size (b)) {
            char t[PD_STRING] = { 0 };
            PD_ASSERT (i >= 0);
            if (!(err = atom_toString (buffer_atoms (b) + i, t, PD_STRING))) { *s = gensym (t); }
        }
        if (err) {
            *s = (fallback ? fallback : utils_empty());
        }
    }
}

static void iemgui_fetchUnexpandedNames (t_iem *iem, t_iemnames *s)
{
    int i = iem->iem_cacheIndex + 1;        /* It start now with the name of the object prepended. */
    
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedSend,    i + 0, iem->iem_send);
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedReceive, i + 1, iem->iem_receive);
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedLabel,   i + 2, iem->iem_label);
        
    s->n_unexpandedSend    = iem->iem_unexpandedSend;
    s->n_unexpandedReceive = iem->iem_unexpandedReceive;
    s->n_unexpandedLabel   = iem->iem_unexpandedLabel;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_serializeColors (t_iem *iem, t_iemcolors *c)
{
    c->c_symColorBackground = iemgui_colorEncode (iem->iem_colorBackground);
    c->c_symColorForeground = iemgui_colorEncode (iem->iem_colorForeground);
    c->c_symColorLabel      = iemgui_colorEncode (iem->iem_colorLabel);
}

void iemgui_deserializeColors (t_iem *iem, t_atom *background, t_atom *foreground, t_atom *label)
{
    iem->iem_colorForeground = IEM_COLOR_FOREGROUND;
    iem->iem_colorLabel      = IEM_COLOR_LABEL;
    
    if (pd_class (iem) == panel_class) { iem->iem_colorBackground = IEM_COLOR_BACKGROUND_DARK; }
    else {
        iem->iem_colorBackground = IEM_COLOR_BACKGROUND_LIGHT;
    }
    
    if (background) { iem->iem_colorBackground = iemgui_colorDecode (background); }
    if (foreground) { iem->iem_colorForeground = iemgui_colorDecode (foreground); }
    if (label)      { iem->iem_colorLabel      = iemgui_colorDecode (label);      }
}

void iemgui_deserializeFontStyle (t_iem *iem, int n)
{
    iem->iem_fontStyle = (char)n;
}

int iemgui_serializeFontStyle (t_iem *iem)
{
    return (int)iem->iem_fontStyle;
}

void iemgui_deserializeLoadbang (t_iem *iem, int n)
{
    iem->iem_loadbang = ((n & 1) != 0);
}

int iemgui_serializeLoadbang (t_iem *iem)
{
    return (iem->iem_loadbang ? 1 : 0);
}

void iemgui_deserializeNamesByIndex (t_iem *iem, int i, t_atom *argv)
{
    iem->iem_send    = (argv ? iemgui_fetchName (i + 0, argv) : utils_empty());
    iem->iem_receive = (argv ? iemgui_fetchName (i + 1, argv) : utils_empty());
    iem->iem_label   = (argv ? iemgui_fetchName (i + 2, argv) : utils_empty());
    
    iem->iem_unexpandedSend    = NULL;
    iem->iem_unexpandedReceive = NULL;
    iem->iem_unexpandedLabel   = NULL;
    
    iem->iem_cacheIndex = i;    /* Cache this index for later lookup. */
}

void iemgui_serializeNames (t_iem *iem, t_iemnames *n)
{
    iemgui_fetchUnexpandedNames (iem, n);
    
    n->n_unexpandedSend    = dollar_toHash (n->n_unexpandedSend);
    n->n_unexpandedReceive = dollar_toHash (n->n_unexpandedReceive);
    n->n_unexpandedLabel   = dollar_toHash (n->n_unexpandedLabel);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_checkSendReceiveLoop (t_iem *iem)
{
    iem->iem_goThrough = 1;
    
    if (iem->iem_canSend && iem->iem_canReceive) {
        if (iem->iem_send == iem->iem_receive) { iem->iem_goThrough = 0; }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_setSend (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromHash (utils_substituteIfEmpty (s, 0));
    iem->iem_unexpandedSend = t;
    iem->iem_send = iemgui_expandDollar (iem->iem_owner, t);
    iem->iem_canSend = (s == utils_empty()) ? 0 : 1;
    iemgui_checkSendReceiveLoop (iem);
}

void iemgui_setReceive (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromHash (utils_substituteIfEmpty (s, 0));
    if (iem->iem_canReceive) { pd_unbind (cast_pd (iem), iem->iem_receive); }
    iem->iem_unexpandedReceive = t;
    iem->iem_receive = iemgui_expandDollar (iem->iem_owner, t);
    iem->iem_canReceive = (s == utils_empty()) ? 0 : 1;
    if (iem->iem_canReceive) { pd_bind (cast_pd (iem), iem->iem_receive); }
    iemgui_checkSendReceiveLoop (iem);
}

void iemgui_setLabel (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromHash (utils_substituteIfEmpty (s, 0));
    iem->iem_unexpandedLabel = t;
    iem->iem_label = iemgui_expandDollar (iem->iem_owner, t);

    if (canvas_isMapped (iem->iem_owner)) {
        sys_vGui (".x%lx.c itemconfigure %lxLABEL -text {%s}\n",
                        canvas_getView (iem->iem_owner),
                        x,
                        iem->iem_label != utils_empty() ? iem->iem_label->s_name : "");
    }
}

void iemgui_setLabelPosition (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) {
    //
    iem->iem_labelX = (int)atom_getFloatAtIndex (0, argc, argv);
    iem->iem_labelY = (int)atom_getFloatAtIndex (1, argc, argv);
    
    if (canvas_isMapped (iem->iem_owner)) {
        sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                        canvas_getView (iem->iem_owner),
                        x,
                        text_getPositionX (cast_object (x), iem->iem_owner) + iem->iem_labelX,
                        text_getPositionY (cast_object (x), iem->iem_owner) + iem->iem_labelY);
    }
    //
    }
}

void iemgui_setLabelFont (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) {
    //
    int f = (int)atom_getFloatAtIndex (1, argc, argv);
    f = PD_MAX (f, IEM_MINIMUM_FONTSIZE);
    iem->iem_fontSize = f;
    if (canvas_isMapped (iem->iem_owner)) {
        sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d]\n",
                        canvas_getView (iem->iem_owner), 
                        x,
                        font_getHostFontSize (iem->iem_fontSize));
    }
    //
    }
}

void iemgui_setBackgroundColor (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    iem->iem_colorBackground = iemgui_colorRGB (argc, argv);
    
    if (canvas_isMapped (iem->iem_owner)) { (*iem->iem_draw) (x, iem->iem_owner, IEM_DRAW_CONFIG); }
}

void iemgui_setForegroundColor (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    iem->iem_colorForeground = iemgui_colorRGB (argc, argv);
    
    if (canvas_isMapped (iem->iem_owner)) { (*iem->iem_draw) (x, iem->iem_owner, IEM_DRAW_CONFIG); }
}

void iemgui_setLabelColor (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    iem->iem_colorLabel = iemgui_colorRGB (argc, argv);
    
    if (canvas_isMapped (iem->iem_owner)) { (*iem->iem_draw) (x, iem->iem_owner, IEM_DRAW_CONFIG); }
}

void iemgui_setPosition (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) {
    //
    iem->iem_obj.te_xCoordinate = (int)atom_getFloatAtIndex (0, argc, argv);
    iem->iem_obj.te_yCoordinate = (int)atom_getFloatAtIndex (1, argc, argv);
    
    iemgui_boxChanged (x, iem);
    //
    }
}

void iemgui_movePosition (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) {
    //
    iem->iem_obj.te_xCoordinate += (int)atom_getFloatAtIndex (0, argc, argv);
    iem->iem_obj.te_yCoordinate += (int)atom_getFloatAtIndex (1, argc, argv);
    
    iemgui_boxChanged (x, iem);
    //
    }
}

void iemgui_boxChanged (void *x, t_iem *iem)
{
    if (canvas_isMapped (iem->iem_owner)) {
    //
    (*iem->iem_draw) (x, iem->iem_owner, IEM_DRAW_CONFIG);
    (*iem->iem_draw) (x, iem->iem_owner, IEM_DRAW_MOVE);
    canvas_updateLinesByObject (iem->iem_owner, cast_object (x));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Trampoline fun. */

void iemjump_send (void *x, t_symbol *s)
{
    iemgui_setSend (x, cast_iem (x), s);
}

void iemjump_receive (void *x, t_symbol *s)
{
    iemgui_setReceive (x, cast_iem (x), s);
}

void iemjump_label (void *x, t_symbol *s)
{
    iemgui_setLabel (x, cast_iem (x), s);
}

void iemjump_labelPosition (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelPosition (x, cast_iem (x), s, argc, argv);
}

void iemjump_labelFont (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelFont (x, cast_iem (x), s, argc, argv);
}

void iemjump_backgroundColor (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setBackgroundColor (x, cast_iem (x), s, argc, argv);
}

void iemjump_foregroundColor (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setForegroundColor (x, cast_iem (x), s, argc, argv);
}

void iemjump_labelColor (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelColor (x, cast_iem (x), s, argc, argv);
}

void iemjump_position (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setPosition (x, cast_iem (x), s, argc, argv);
}

void iemjump_move (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_movePosition (x, cast_iem (x), s, argc, argv);
}

void iemjump_dummy (void *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_iem *x = cast_iem (z);

    x->iem_obj.te_xCoordinate += deltaX;
    x->iem_obj.te_yCoordinate += deltaY;
    
    (*x->iem_draw) ((void *)z, glist, IEM_DRAW_MOVE);
    
    canvas_updateLinesByObject (glist, cast_object (z));
}

void iemgui_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_iem *x = cast_iem (z);

    x->iem_isSelected = isSelected;
    
    (*x->iem_draw) ((void *)z, glist, IEM_DRAW_SELECT);
}

void iemgui_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_iem *x = cast_iem (z);

    if (isVisible) { (*x->iem_draw) ((void *)z, glist, IEM_DRAW_NEW); }
    else {
        (*x->iem_draw) ((void *)z, glist, IEM_DRAW_ERASE);
        interface_guiQueueRemove ((void *)z);
    }
}

void iemgui_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    canvas_deleteLinesByObject (glist, cast_object (z));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_serialize (t_iem *iem, t_iemnames *n, t_iemcolors *c)
{
    iemgui_fetchUnexpandedNames (iem, n);
    iemgui_serializeColors (iem, c);
}

void iemgui_fromDialog (t_iem *iem, int argc, t_atom *argv)
{
    int loadbang                = (int)atom_getFloatAtIndex (5,  argc, argv);
    int labelX                  = (int)atom_getFloatAtIndex (10, argc, argv);
    int labelY                  = (int)atom_getFloatAtIndex (11, argc, argv);
    int fontSize                = (int)atom_getFloatAtIndex (12, argc, argv);
    int backgroundColor         = (int)atom_getFloatAtIndex (13, argc, argv);
    int foregroundColor         = (int)atom_getFloatAtIndex (14, argc, argv);
    int labelColor              = (int)atom_getFloatAtIndex (15, argc, argv);
    int canSend                 = 1;
    int canReceive              = 1;

    t_symbol *s1 = dollar_fromHash (iemgui_fetchName (7, argv));
    t_symbol *s2 = dollar_fromHash (iemgui_fetchName (8, argv));
    t_symbol *s3 = dollar_fromHash (iemgui_fetchName (9, argv));

    iem->iem_unexpandedSend     = s1;
    iem->iem_unexpandedReceive  = s2;
    iem->iem_unexpandedLabel    = s3;
    
    s1 = iemgui_expandDollar (iem->iem_owner, s1);
    s2 = iemgui_expandDollar (iem->iem_owner, s2);
    s3 = iemgui_expandDollar (iem->iem_owner, s3);
    
    if (s1 == utils_empty()) { canSend = 0;    }
    if (s2 == utils_empty()) { canReceive = 0; }
    
    if (canReceive) {
        if (iem->iem_canReceive) { pd_unbind (cast_pd (iem), iem->iem_receive); }
        iem->iem_receive = s2;
        pd_bind (cast_pd (iem), iem->iem_receive);
    }

    iem->iem_canSend            = canSend;
    iem->iem_canReceive         = canReceive;
    iem->iem_loadbang           = (loadbang != 0);
    iem->iem_labelX             = labelX;
    iem->iem_labelY             = labelY;
    iem->iem_fontSize           = PD_MAX (fontSize, IEM_MINIMUM_FONTSIZE);
    iem->iem_colorForeground    = IEM_COLOR_MASK & foregroundColor;
    iem->iem_colorBackground    = IEM_COLOR_MASK & backgroundColor;
    iem->iem_colorLabel         = IEM_COLOR_MASK & labelColor;
    iem->iem_send               = s1;
    iem->iem_label              = s3;
    
    iemgui_checkSendReceiveLoop (iem);
    
    canvas_dirty (iem->iem_owner, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
