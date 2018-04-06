
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol *iemgui_expandDollar (t_glist *glist, t_symbol *s)
{
    t_symbol *t = dollar_expandSymbol (s, glist); 
    
    return (t == NULL ? symbol_nil() : t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Ensure compatibility with the original serialization format. */
/* By the way legacy predefined colors are not supported. */
/* Only the 6 MSB are kept for each component. */

// RRRRRRRRGGGGGGGGBBBBBBBB 
// RRRRRR..GGGGGG..BBBBBB..
// RRRRRRGGGGGGBBBBBB

static t_color iemgui_colorDecodeFromInteger (int color)
{
    PD_ASSERT (color < 0);
    
    t_color n = 0;
    
    color = (-1 -color);
    
    n |= ((color & 0x3f000) << 6);
    n |= ((color & 0xfc0) << 4);
    n |= ((color & 0x3f) << 2);

    return n;
}

static t_color iemgui_colorDecode (t_atom *a)
{
    if (IS_FLOAT (a))  { return iemgui_colorDecodeFromInteger ((int)GET_FLOAT (a)); }
    if (IS_SYMBOL (a)) { return color_withEncoded (atom_getSymbol (a));       }
    
    PD_BUG;
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* For convenience, for labels only, a float is loaded as integers (mainly in order to enumerate things). */
/* It is prohibited for send and receive. */
/* Unused but kept for compatibility. */

static t_symbol *iemgui_fetchName (int i, t_atom *argv, int isNumberAllowed)
{
    if (IS_SYMBOL (argv + i))     { return atom_getSymbol (argv + i); }
    else if (IS_FLOAT (argv + i)) {
    //
    if (isNumberAllowed) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, PD_STRING, "%d", (int)atom_getFloat (argv + i));
        return gensym (t);
    }
    //
    }

    return symbol_nil();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Unexpanded names cannot be fetch at instantiation due to already substituted arguments. */
/* Consequently it must be done by looking up again the object's text buffer. */

static void iemgui_fetchUnexpanded (t_iem *iem, t_symbol **s, int i, int isNumberAllowed, t_symbol *fallback)
{
    if (!*s) {
        t_error err = PD_ERROR;
        t_buffer *b = object_getBuffer (cast_object (iem));
        if (i >= 0 && i < buffer_getSize (b)) {
            char t[PD_STRING] = { 0 };
            t_atom *a = buffer_getAtomAtIndex (b, i);
            if (isNumberAllowed || !IS_FLOAT (a)) {
                if (!(err = atom_toString (a, t, PD_STRING))) { *s = gensym (t); }
            }
        }
        if (err) {
            *s = (fallback ? fallback : symbol_nil());
        }
    }
}

static void iemgui_fetchUnexpandedNames (t_iem *iem, t_iemnames *s)
{
    int i = iem->iem_cacheIndex + 1;    /* Buffer start now with the name of the object prepended. */
    
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedSend,    i + 0, 0, iem->iem_send);
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedReceive, i + 1, 0, iem->iem_receive);
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedLabel,   i + 2, 1, iem->iem_label);
        
    s->n_unexpandedSend    = iem->iem_unexpandedSend;
    s->n_unexpandedReceive = iem->iem_unexpandedReceive;
    s->n_unexpandedLabel   = iem->iem_unexpandedLabel;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void iemgui_serializeColors (t_iem *iem, t_iemcolors *c)
{
    c->c_symColorBackground = color_toEncoded (iem->iem_colorBackground);
    c->c_symColorForeground = color_toEncoded (iem->iem_colorForeground);
    c->c_symColorLabel      = color_toEncoded (iem->iem_colorLabel);
}

int iemgui_serializeFontStyle (t_iem *iem)
{
    return (int)iem->iem_fontStyle;
}

int iemgui_serializeLoadbang (t_iem *iem)
{
    return (iem->iem_loadbang ? 1 : 0);
}

void iemgui_serializeNames (t_iem *iem, t_iemnames *n)
{
    iemgui_fetchUnexpandedNames (iem, n);
    
    n->n_unexpandedSend    = symbol_dollarToHash (n->n_unexpandedSend);
    n->n_unexpandedReceive = symbol_dollarToHash (n->n_unexpandedReceive);
    n->n_unexpandedLabel   = symbol_dollarToHash (n->n_unexpandedLabel);
}

void iemgui_deserializeColors (t_iem *iem, t_atom *background, t_atom *foreground, t_atom *label)
{
    iem->iem_colorForeground = COLOR_IEM_FOREGROUND;
    iem->iem_colorLabel      = COLOR_IEM_LABEL;
    
    if (pd_class (iem) == panel_class) { iem->iem_colorBackground = COLOR_IEM_PANEL; }
    else {
        iem->iem_colorBackground = COLOR_IEM_BACKGROUND;
    }
    
    if (background) { iem->iem_colorBackground = iemgui_colorDecode (background); }
    if (foreground) { iem->iem_colorForeground = iemgui_colorDecode (foreground); }
    if (label)      { iem->iem_colorLabel      = iemgui_colorDecode (label);      }
}

void iemgui_deserializeFontStyle (t_iem *iem, int n)
{
    iem->iem_fontStyle = (char)n;
}

void iemgui_deserializeLoadbang (t_iem *iem, int n)
{
    iem->iem_loadbang = ((n & 1) != 0);
}

void iemgui_deserializeNames (t_iem *iem, int i, t_atom *argv)
{
    iem->iem_send    = (argv ? iemgui_fetchName (i + 0, argv, 0) : symbol_nil());
    iem->iem_receive = (argv ? iemgui_fetchName (i + 1, argv, 0) : symbol_nil());
    iem->iem_label   = (argv ? iemgui_fetchName (i + 2, argv, 1) : symbol_nil());
    
    iem->iem_unexpandedSend    = NULL;
    iem->iem_unexpandedReceive = NULL;
    iem->iem_unexpandedLabel   = NULL;
    
    iem->iem_cacheIndex = i;    /* Cache this index for later lookup. */
}

void iemgui_deserializeDefault (t_iem *iem)
{
    iemgui_deserializeNames (iem, -PD_INT_MAX, NULL);
    iemgui_deserializeColors (iem, NULL, NULL, NULL);
}   

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void iemgui_checkSendReceiveLoop (t_iem *iem)
{
    iem->iem_goThrough = 1;
    
    if (iem->iem_canSend && iem->iem_canReceive) {
        if (iem->iem_send == iem->iem_receive) { iem->iem_goThrough = 0; }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void iemgui_setSend (void *x, t_symbol *s)
{
    t_iem *iem  = cast_iem (x);
    t_symbol *t = symbol_hashToDollar (symbol_emptyAsNil (s));
    iem->iem_unexpandedSend = t;
    iem->iem_send = iemgui_expandDollar (iem->iem_owner, t);
    iem->iem_canSend = symbol_isNil (s) ? 0 : 1;
    iemgui_checkSendReceiveLoop (iem);
}

void iemgui_setReceive (void *x, t_symbol *s)
{
    t_iem *iem  = cast_iem (x);
    t_symbol *t = symbol_hashToDollar (symbol_emptyAsNil (s));
    if (iem->iem_canReceive) { pd_unbind (cast_pd (iem), iem->iem_receive); }
    iem->iem_unexpandedReceive = t;
    iem->iem_receive = iemgui_expandDollar (iem->iem_owner, t);
    iem->iem_canReceive = symbol_isNil (s) ? 0 : 1;
    if (iem->iem_canReceive) { pd_bind (cast_pd (iem), iem->iem_receive); }
    iemgui_checkSendReceiveLoop (iem);
}

#if PD_WITH_LEGACY

void iemgui_setLabel (void *x, t_symbol *s)
{
    t_iem *iem  = cast_iem (x);
    t_symbol *t = symbol_hashToDollar (symbol_emptyAsNil (s));
    
    iem->iem_unexpandedLabel = t;
    iem->iem_label = iemgui_expandDollar (iem->iem_owner, t);
}

void iemgui_setLabelPosition (void *x, t_symbol *s, int argc, t_atom *argv)
{
    t_iem *iem = cast_iem (x);
    
    if (argc > 1) {
    //
    iem->iem_labelX = (int)atom_getFloatAtIndex (0, argc, argv);
    iem->iem_labelY = (int)atom_getFloatAtIndex (1, argc, argv);
    //
    }
}

void iemgui_setLabelFont (void *x, t_symbol *s, int argc, t_atom *argv)
{
    t_iem *iem = cast_iem (x);
    
    if (argc) {
    //
    iem->iem_fontSize = (int)atom_getFloatAtIndex (argc > 1 ? 1 : 0, argc, argv);
    //
    }
}

#endif // PD_WITH_LEGACY

void iemgui_setBackgroundColor (void *x, t_symbol *s, int argc, t_atom *argv)
{
    t_iem *iem = cast_iem (x);
    
    iem->iem_colorBackground = color_withRGB (argc, argv);
    
    if (glist_isOnScreen (iem->iem_owner)) { (*iem->iem_fnDraw) (x, iem->iem_owner, IEM_DRAW_CONFIG); }
}

void iemgui_setForegroundColor (void *x, t_symbol *s, int argc, t_atom *argv)
{
    t_iem *iem = cast_iem (x);
    
    iem->iem_colorForeground = color_withRGB (argc, argv);
    
    if (glist_isOnScreen (iem->iem_owner)) { (*iem->iem_fnDraw) (x, iem->iem_owner, IEM_DRAW_CONFIG); }
}

void iemgui_setPosition (void *x, t_symbol *s, int argc, t_atom *argv)
{
    t_iem *iem = cast_iem (x);
    
    if (argc > 1) {
    //
    object_setSnappedX (cast_object (iem), atom_getFloatAtIndex (0, argc, argv));
    object_setSnappedY (cast_object (iem), atom_getFloatAtIndex (1, argc, argv));
    
    iemgui_boxChanged (x);
    //
    }
}

void iemgui_movePosition (void *x, t_symbol *s, int argc, t_atom *argv)
{
    t_iem *iem = cast_iem (x);
    
    if (argc > 1) {
    //
    int m = atom_getFloatAtIndex (0, argc, argv);
    int n = atom_getFloatAtIndex (1, argc, argv);
    
    object_setSnappedX (cast_object (iem), object_getX (cast_object (iem)) + m);
    object_setSnappedY (cast_object (iem), object_getY (cast_object (iem)) + n);

    iemgui_boxChanged (x);
    //
    }
}

void iemgui_boxChanged (void *x)
{
    t_iem *iem = cast_iem (x);
    
    if (glist_isOnScreen (iem->iem_owner)) {
    //
    (*iem->iem_fnDraw) (x, iem->iem_owner, IEM_DRAW_UPDATE);
    (*iem->iem_fnDraw) (x, iem->iem_owner, IEM_DRAW_CONFIG);
    (*iem->iem_fnDraw) (x, iem->iem_owner, IEM_DRAW_MOVE);
    
    glist_updateLinesForObject (iem->iem_owner, cast_object (x));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void iemgui_dummy (void *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void iemgui_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_iem *x = cast_iem (z);

    object_setSnappedX (cast_object (x), object_getX (cast_object (x)) + deltaX);
    object_setSnappedY (cast_object (x), object_getY (cast_object (x)) + deltaY);
    
    (*x->iem_fnDraw) ((void *)z, glist, IEM_DRAW_MOVE);
    
    glist_updateLinesForObject (glist, cast_object (z));
}

void iemgui_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_iem *x = cast_iem (z);

    x->iem_isSelected = isSelected;
    
    (*x->iem_fnDraw) ((void *)z, glist, IEM_DRAW_SELECT);
}

void iemgui_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_iem *x = cast_iem (z);

    if (isVisible) { (*x->iem_fnDraw) ((void *)z, glist, IEM_DRAW_NEW); }
    else {
        (*x->iem_fnDraw) ((void *)z, glist, IEM_DRAW_ERASE);
    }
    
    if (!isVisible) { gui_jobRemove ((void *)z); }
}

void iemgui_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    gui_jobRemove ((void *)z);
    
    glist_objectDeleteLines (glist, cast_object (z));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void iemgui_serialize (t_iem *iem, t_iemnames *n, t_iemcolors *c)
{
    iemgui_fetchUnexpandedNames (iem, n);
    iemgui_serializeColors (iem, c);
}

int iemgui_fromDialog (t_iem *iem, int argc, t_atom *argv)
{
    int isDirty   = 0;
    
    int t1        = iem->iem_loadbang;
    t_color t2    = iem->iem_colorForeground;
    t_color t3    = iem->iem_colorBackground;
    t_symbol *t4  = iem->iem_send;
    t_symbol *t5  = iem->iem_receive;
    
    {
    //
    int loadbang                = (int)atom_getFloatAtIndex (5,  argc, argv);
    t_color backgroundColor     = (int)atom_getFloatAtIndex (9,  argc, argv);
    t_color foregroundColor     = (int)atom_getFloatAtIndex (10, argc, argv);
    int canSend                 = 1;
    int canReceive              = 1;

    t_symbol *s1 = symbol_hashToDollar (iemgui_fetchName (7, argv, 0));
    t_symbol *s2 = symbol_hashToDollar (iemgui_fetchName (8, argv, 0));

    iem->iem_unexpandedSend     = s1;
    iem->iem_unexpandedReceive  = s2;
    
    s1 = iemgui_expandDollar (iem->iem_owner, s1);
    s2 = iemgui_expandDollar (iem->iem_owner, s2);
    
    if (symbol_isNil (s1)) { canSend = 0;    }
    if (symbol_isNil (s2)) { canReceive = 0; }
    
    if (iem->iem_canReceive) { pd_unbind (cast_pd (iem), iem->iem_receive); }
    
    if (canReceive) {
        iem->iem_receive = s2; pd_bind (cast_pd (iem), iem->iem_receive);
    }

    iem->iem_canSend            = canSend;
    iem->iem_canReceive         = canReceive;
    iem->iem_loadbang           = (loadbang == 1);
    iem->iem_colorForeground    = color_checked (foregroundColor);
    iem->iem_colorBackground    = color_checked (backgroundColor);
    iem->iem_send               = s1;
    
    iemgui_checkSendReceiveLoop (iem);
    //
    }
    
    isDirty |= (t1 != iem->iem_loadbang);
    isDirty |= (t2 != iem->iem_colorForeground);
    isDirty |= (t3 != iem->iem_colorBackground);
    isDirty |= (t4 != iem->iem_send);
    isDirty |= (t5 != iem->iem_receive);
    
    return isDirty;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
