
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Weak pointer machinery. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define GMASTER_NONE    0
#define GMASTER_GLIST   1
#define GMASTER_ARRAY   2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _gmaster {
    union {
        t_glist     *gm_glist;
        t_array     *gm_array;
    } gm_un;
    int             gm_type;
    int             gm_count;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_gpointer gpointer_empty;     /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_gmaster *gmaster_createWithGlist (t_glist *glist)
{
    t_gmaster *master = (t_gmaster *)PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (glist);
    
    master->gm_type         = GMASTER_GLIST;
    master->gm_un.gm_glist  = glist;
    master->gm_count        = 0;
    
    return master;
}

t_gmaster *gmaster_createWithArray (t_array *array)
{
    t_gmaster *master = (t_gmaster *)PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (array);
    
    master->gm_type         = GMASTER_ARRAY;
    master->gm_un.gm_array  = array;
    master->gm_count        = 0;
    
    return master;
}

void gmaster_reset (t_gmaster *master)
{
    PD_ASSERT (master->gm_count >= 0);
    
    master->gm_type = GMASTER_NONE; if (master->gm_count == 0) { PD_MEMORY_FREE (master); }
}

static void gmaster_increment (t_gmaster *master)
{
    master->gm_count++;
}

static void gmaster_decrement (t_gmaster *master)
{
    int count = --master->gm_count;
    
    PD_ASSERT (count >= 0);
    
    if (count == 0 && master->gm_type == GMASTER_NONE) { PD_MEMORY_FREE (master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int gpointer_isSet (t_gpointer *gp)
{
    return (gp->gp_refer != NULL);
}

static int gpointer_isNull (t_gpointer *gp)
{
    return (gp->gp_un.gp_scalar == NULL);
}

static int gpointer_isValidProceed (t_gpointer *gp, int nullPointerIsValid)
{
    if (gpointer_isSet (gp)) {
    //
    if (!nullPointerIsValid && gpointer_isNull (gp)) { return 0; }
    else {
    //
    t_gmaster *master = gp->gp_refer;
        
    if (master->gm_type == GMASTER_ARRAY) {
        if (master->gm_un.gm_array->a_uniqueIdentifier == gp->gp_uniqueIdentifier)   { return 1; }
        
    } else if (master->gm_type == GMASTER_GLIST) {
        if (glist_getIdentifier (master->gm_un.gm_glist) == gp->gp_uniqueIdentifier) { return 1; }
        
    } else {
        PD_ASSERT (master->gm_type == GMASTER_NONE);
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_gpointer *gpointer_getEmpty (void)
{
    PD_ASSERT (!gpointer_isSet (&gpointer_empty));
    
    return &gpointer_empty;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Point to head of glist. */

void gpointer_setAsNull (t_gpointer *gp, t_glist *glist)
{
    gpointer_unset (gp);
    
    PD_ASSERT (glist);
    
    gp->gp_un.gp_scalar     = NULL;
    gp->gp_refer            = glist_getMaster (glist);
    gp->gp_uniqueIdentifier = glist_getIdentifier (glist);
    gp->gp_index            = 0;

    gmaster_increment (gp->gp_refer);
}

/* Point to a scalar. */

void gpointer_setAsScalar (t_gpointer *gp, t_scalar *scalar)
{
    gpointer_unset (gp);
    
    PD_ASSERT (scalar);
    
    gp->gp_un.gp_scalar     = scalar;
    gp->gp_refer            = glist_getMaster (scalar->sc_owner);
    gp->gp_uniqueIdentifier = glist_getIdentifier (scalar->sc_owner);
    gp->gp_index            = 0;
    
    gmaster_increment (gp->gp_refer);
}

/* Point to an element (i.e. a chunk of t_word) from an array. */

void gpointer_setAsWord (t_gpointer *gp, t_array *array, int n)
{
    int t = array_getSize (array) - 1;
    
    n = PD_CLAMP (n, 0, t);
    
    gpointer_unset (gp);
    
    gp->gp_un.gp_w          = array_getElementAtIndex (array, n);
    gp->gp_refer            = array->a_holder;
    gp->gp_uniqueIdentifier = array->a_uniqueIdentifier;
    gp->gp_index            = n;

    gmaster_increment (gp->gp_refer);
}

void gpointer_setByCopy (t_gpointer *gp, t_gpointer *toCopy)
{
    gpointer_unset (gp);
    
    *gp = *toCopy;
    
    if (gp->gp_refer) { gmaster_increment (gp->gp_refer); }
}

void gpointer_unset (t_gpointer *gp)
{
    if (gpointer_isSet (gp)) { gmaster_decrement (gp->gp_refer); }
    
    gpointer_init (gp);
}

int gpointer_isValid (t_gpointer *gp)
{
    return gpointer_isValidProceed (gp, 0); 
}

int gpointer_isValidOrNull (t_gpointer *gp)
{
    return gpointer_isValidProceed (gp, 1); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int gpointer_isScalar (t_gpointer *gp)
{
    return (gp->gp_refer->gm_type == GMASTER_GLIST);
}

int gpointer_isWord (t_gpointer *gp)
{
    return (gp->gp_refer->gm_type == GMASTER_ARRAY);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_scalar *gpointer_getScalar (t_gpointer *gp)
{
    PD_ASSERT (gpointer_isScalar (gp)); return (gp->gp_un.gp_scalar);
}

t_word *gpointer_getWord (t_gpointer *gp)
{
    PD_ASSERT (gpointer_isWord (gp)); return (gp->gp_un.gp_w);
}

int gpointer_getIndex (t_gpointer *gp)
{
    PD_ASSERT (gpointer_isWord (gp)); return (gp->gp_index);
}

t_glist *gpointer_getParentForScalar (t_gpointer *gp)
{
    PD_ASSERT (gp->gp_refer->gm_type == GMASTER_GLIST);
    
    return (gp->gp_refer->gm_un.gm_glist);
}

t_array *gpointer_getParentForWord (t_gpointer *gp)
{
    PD_ASSERT (gp->gp_refer->gm_type == GMASTER_ARRAY);
    
    return (gp->gp_refer->gm_un.gm_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_word *gpointer_getElement (t_gpointer *gp)
{
    if (gpointer_isWord (gp))       { return gpointer_getWord (gp); } 
    else if (!gpointer_isNull (gp)) { return scalar_getElement (gpointer_getScalar (gp)); }
    
    return NULL;
}

t_glist *gpointer_getView (t_gpointer *gp)
{
    if (gpointer_isScalar (gp)) { return gpointer_getParentForScalar (gp); }
    else {
        return gpointer_getParentForScalar (array_getTopParent (gpointer_getParentForWord (gp)));
    }
}

t_symbol *gpointer_getTemplateIdentifier (t_gpointer *gp)
{
    t_gmaster *master = gp->gp_refer;
    
    PD_ASSERT (gpointer_isValidOrNull (gp));
    
    if (master->gm_type == GMASTER_GLIST) {
        if (!gpointer_isNull (gp)) { return scalar_getTemplateIdentifier (gpointer_getScalar (gp)); }
        
    } else {
        return array_getTemplateIdentifier (master->gm_un.gm_array);
    }
    
    return &s_;
}

t_template *gpointer_getTemplate (t_gpointer *gp)
{
    t_template *tmpl = template_findByIdentifier (gpointer_getTemplateIdentifier (gp));
    
    PD_ASSERT (tmpl);
    
    return (tmpl);
}

static t_scalar *gpointer_getBase (t_gpointer *gp)
{
    t_scalar *scalar = NULL;
    
    if (gpointer_isScalar (gp)) { scalar = gpointer_getScalar (gp); }
    else {
        scalar = gpointer_getScalar (array_getTopParent (gpointer_getParentForWord (gp)));
    }
    
    return scalar;
}

int gpointer_isInstanceOf (t_gpointer *gp, t_symbol *templateIdentifier)
{
    if (templateIdentifier == template_getWildcard())                   { return 1; }
    else if (templateIdentifier == gpointer_getTemplateIdentifier (gp)) { return 1; }
    
    return 0;
}

int gpointer_isValidInstanceOf (t_gpointer *gp, t_symbol *templateIdentifier)
{
    if (!gpointer_isValid (gp))                                         { return 0; }
    else if (!gpointer_isInstanceOf (gp, templateIdentifier))           { return 0; }
    else if (!gpointer_getTemplate (gp))                                { return 0; }
    
    return 1;
}

t_garray *gpointer_getGraphicArray (t_gpointer *gp)
{
    t_glist *glist = gpointer_getView (gp);
    
    if (glist_isArray (glist)) { return glist_getArray (glist); }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gpointer_redraw (t_gpointer *gp)
{
    scalar_redraw (gpointer_getBase (gp), gpointer_getView (gp));
}

void gpointer_erase (t_gpointer *gp)
{
    t_glist *view = gpointer_getView (gp);
    
    if (glist_isOnScreen (view)) { gobj_visibilityChanged (cast_gobj (gpointer_getBase (gp)), view, 0); }
}

void gpointer_draw (t_gpointer *gp)
{
    t_glist *view = gpointer_getView (gp);
    
    glist_redrawRequired (view);
    
    if (glist_isOnScreen (view)) { gobj_visibilityChanged (cast_gobj (gpointer_getBase (gp)), view, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gpointer_notify (t_gpointer *gp, t_symbol *s, int argc, t_atom *argv)
{
    t_template *x = gpointer_getTemplate (gp);
    t_atom *a = NULL;
    int i, n = argc + 1;
    t_gpointer copy; gpointer_init (&copy);
    
    PD_ATOMS_ALLOCA (a, n);
    
    gpointer_setByCopy (&copy, gp);
    SET_POINTER (a, &copy);
    for (i = 0; i < argc; i++) { *(a + i + 1) = *(argv + i); }
    if (x->tp_instance) { struct_notify (x->tp_instance, s, n, a); }
    gpointer_unset (&copy);
    
    PD_ATOMS_FREEA (a, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *gpointer_asRepresentation (t_gpointer *gp)
{
    t_symbol *t = sym___arrobe__invalid;
    
    if (gp) {
        if (gpointer_isValid (gp))            { t = sym___arrobe__pointer; }
        else if (gpointer_isValidOrNull (gp)) { t = sym___arrobe__head;    }
    }
    
    return t;
}

t_gpointer *gpointer_fromRepresentation (t_symbol *s)
{
    /* Not implemented yet. */
    
    PD_ASSERT (s != sym___arrobe__invalid);
    PD_ASSERT (s != sym___arrobe__pointer);
    PD_ASSERT (s != sym___arrobe__head);
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error gpointer_getFieldAsString (t_gpointer *gp, t_symbol *fieldName, char *dest, int size)
{
    t_error err = PD_ERROR_NONE;
    
    PD_ASSERT (size > 0);
    
    if (gpointer_fieldIsFloat (gp, fieldName)) {
        err = string_addSprintf (dest, size, "%.9g", gpointer_getFloat (gp, fieldName));
        
    } else if (gpointer_fieldIsSymbol (gp, fieldName)) {
        err = string_addSprintf (dest, size, "%s", gpointer_getSymbol (gp, fieldName)->s_name);
        
    } else if (gpointer_fieldIsArray (gp, fieldName)) {
        err = string_addSprintf (dest, size, "%d", array_getSize (gpointer_getArray (gp, fieldName)));
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int gpointer_getValues (t_gpointer *gp, t_heapstring *h)
{
    t_template *tmpl = gpointer_getTemplate (gp);
    
    int i, k = template_getSize (tmpl);
    
    t_symbol *s = symbol_stripTemplateIdentifier (gpointer_getTemplateIdentifier (gp));
    
    heapstring_addSprintf (h, "%s", s->s_name);
 
    for (i = 0; i < k; i++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
    
    heapstring_addSprintf (h, " {%s}", fieldName->s_name);                                          // --

    if (template_fieldIsFloat (tmpl, fieldName)) {
        heapstring_addSprintf (h, " {%.9g}", gpointer_getFloat (gp, fieldName));                    // --
    
    } else if (template_fieldIsSymbol (tmpl, fieldName)) {
        t_symbol *t = symbol_dollarToHash (gpointer_getSymbol (gp, fieldName));
        heapstring_addSprintf (h, " {%s}", t->s_name);                                              // --
        
    } else if (template_fieldIsArray (tmpl, fieldName)) {
        heapstring_addSprintf (h, " {%d}", array_getSize (gpointer_getArray (gp, fieldName)));      // --
    }
    //
    }
    
    return k;
}

/* Change notifications are sent only for interactive modifications. */

void gpointer_setValues (t_gpointer *gp, int argc, t_atom *argv, int notify)
{
    t_template *tmpl = gpointer_getTemplate (gp);
    
    int k = 0;
    
    gpointer_erase (gp);
    
    while (argc > 1 && (atom_getSymbol (argv) == sym_field)) {
    //
    t_symbol *fieldName = atom_getSymbol (argv + 1);
    
    argc -= 2; argv += 2;
    
    {
    //
    int hasValue = (argc > 1 && (atom_getSymbol (argv) == sym_value));

    if (template_hasField (tmpl, fieldName)) {
    //
    if (template_fieldIsFloat (tmpl, fieldName)) {
        t_float f = hasValue ? atom_getFloat (argv + 1) : 0.0;
        k |= (gpointer_getFloat (gp, fieldName) != f);
        gpointer_setFloat (gp, fieldName, f);
        
    } else if (template_fieldIsSymbol (tmpl, fieldName)) {
        t_symbol *s = hasValue ? symbol_hashToDollar (atom_getSymbol (argv + 1)) : &s_;
        k |= (gpointer_getSymbol (gp, fieldName) != s);
        gpointer_setSymbol (gp, fieldName, s);
        
    } else if (template_fieldIsArrayAndValid (tmpl, fieldName)) {
        int n = (int)(hasValue ? atom_getFloat (argv + 1) : 0.0);
        t_array *array = gpointer_getArray (gp, fieldName);
        int size = array_getSize (array);
        n = PD_MAX (1, n);
        if (size != n) {
            array_resize (array, n);
            k = 1;
        }
    }
    //
    }

    if (hasValue) { argc -= 2; argv += 2; }
    //
    }
    //
    }
    
    gpointer_draw (gp);
    
    if (notify && k) { gpointer_notify (gp, sym_change, 0, NULL); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Fields getter and setter as key/value pairs. */

void gpointer_setFields (t_gpointer *gp, int argc, t_atom *argv)
{
    if (argc > 1) {
    //
    t_atom *t = NULL;
    int n = argc * 2;
    int i = 0;
    int j = 0;
    
    PD_ATOMS_ALLOCA (t, n);
    
    for (i = 0; i < argc; i += 2, j += 4) {
        SET_SYMBOL (t + j + 0, sym_field); atom_copyAtom (argv + i + 0, t + j + 1);
        SET_SYMBOL (t + j + 2, sym_value); atom_copyAtom (argv + i + 1, t + j + 3);
    }
    
    gpointer_setValues (gp, n, t, 0);
    
    PD_ATOMS_FREEA (t, n);
    //
    }
}

t_error gpointer_getFields (t_gpointer *gp, t_buffer *b)
{
    if (gpointer_isValid (gp)) {
    //
    t_template *tmpl = gpointer_getTemplate (gp);
    
    int i, k = template_getSize (tmpl);
    
    for (i = 0; i < k; i++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
    
    buffer_appendSymbol (b, fieldName);

    if (template_fieldIsFloat (tmpl, fieldName)) {
        buffer_appendFloat (b, gpointer_getFloat (gp, fieldName));
    
    } else if (template_fieldIsSymbol (tmpl, fieldName)) {
        buffer_appendSymbol (b, gpointer_getSymbol (gp, fieldName));
        
    } else if (template_fieldIsArray (tmpl, fieldName)) {
        buffer_appendFloat (b, array_getSize (gpointer_getArray (gp, fieldName)));
    }
    //
    }
    
    return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
