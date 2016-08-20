
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Weak pointer machinery. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define GPOINTER_NONE   0
#define GPOINTER_GLIST  1
#define GPOINTER_ARRAY  2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

t_gmaster *gpointer_masterCreateWithGlist (t_glist *glist)
{
    t_gmaster *master = PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (glist);
    
    master->gm_type         = GPOINTER_GLIST;
    master->gm_un.gm_glist  = glist;
    master->gm_count        = 0;
    
    return master;
}

t_gmaster *gpointer_masterCreateWithArray (t_array *array)
{
    t_gmaster *master = PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (array);
    
    master->gm_type         = GPOINTER_ARRAY;
    master->gm_un.gm_array  = array;
    master->gm_count        = 0;
    
    return master;
}

void gpointer_masterRelease (t_gmaster *master)
{
    PD_ASSERT (master->gm_count >= 0);
    
    master->gm_type = GPOINTER_NONE; if (master->gm_count == 0) { PD_MEMORY_FREE (master); }
}

static void gpointer_masterIncrement (t_gmaster *master)
{
    master->gm_count++;
}

static void gpointer_masterDecrement (t_gmaster *master)
{
    int count = --master->gm_count;
    
    PD_ASSERT (count >= 0);
    
    if (count == 0 && master->gm_type == GPOINTER_NONE) { PD_MEMORY_FREE (master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int gpointer_isValidRaw (t_gpointer *gp, int nullPointerIsValid)
{
    if (gpointer_isSet (gp)) {
    //
    if (!nullPointerIsValid && gpointer_isNull (gp)) { return 0; }
    else {
    //
    t_gmaster *master = gp->gp_master;
        
    if (master->gm_type == GPOINTER_ARRAY) {
        if (master->gm_un.gm_array->a_uniqueIdentifier == gp->gp_uniqueIdentifier)  { return 1; }
        
    } else if (master->gm_type == GPOINTER_GLIST) {
        if (master->gm_un.gm_glist->gl_uniqueIdentifier == gp->gp_uniqueIdentifier) { return 1; }
        
    } else {
        PD_ASSERT (master->gm_type == GPOINTER_NONE);
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_init (t_gpointer *gp)
{
    gp->gp_un.gp_scalar     = NULL;
    gp->gp_master           = NULL;
    gp->gp_uniqueIdentifier = 0;
}

/* Point to a scalar. */

void gpointer_setAsScalar (t_gpointer *gp, t_glist *glist, t_scalar *scalar)
{
    gpointer_unset (gp);
    
    gp->gp_un.gp_scalar     = scalar;
    gp->gp_master           = glist->gl_master;
    gp->gp_uniqueIdentifier = glist->gl_uniqueIdentifier;

    gpointer_masterIncrement (gp->gp_master);
}

/* Point to an element (i.e. a chunk of t_word) from an array. */

void gpointer_setAsWord (t_gpointer *gp, t_array *array, t_word *w)
{
    gpointer_unset (gp);
    
    gp->gp_un.gp_w          = w;
    gp->gp_master           = array->a_master;
    gp->gp_uniqueIdentifier = array->a_uniqueIdentifier;

    gpointer_masterIncrement (gp->gp_master);
}

void gpointer_setByCopy (t_gpointer *gp, t_gpointer *toSet)
{
    gpointer_unset (toSet);
    
    *toSet = *gp;
    
    if (toSet->gp_master) { gpointer_masterIncrement (toSet->gp_master); }
}

void gpointer_unset (t_gpointer *gp)
{
    if (gp->gp_master) { gpointer_masterDecrement (gp->gp_master); }
    
    gpointer_init (gp);
}

int gpointer_isSet (t_gpointer *gp)
{
    return (gp->gp_master != NULL);
}

int gpointer_isNull (t_gpointer *gp)
{
    return (gp->gp_un.gp_scalar == NULL);
}

int gpointer_isValid (t_gpointer *gp)
{
    return gpointer_isValidRaw (gp, 0); 
}

int gpointer_isValidNullAllowed (t_gpointer *gp)
{
    return gpointer_isValidRaw (gp, 1); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_retain (t_gpointer *gp)
{
    if (gp->gp_master) { gpointer_masterIncrement (gp->gp_master); }
    else {
        PD_BUG;
    }
}

void gpointer_rawCopy (t_gpointer *src, t_gpointer *dest)
{
    *dest = *src;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gpointer_isScalar (t_gpointer *gp)
{
    return (gp->gp_master->gm_type == GPOINTER_GLIST);
}

int gpointer_isWord (t_gpointer *gp)
{
    return (gp->gp_master->gm_type == GPOINTER_ARRAY);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_scalar *gpointer_getScalar (t_gpointer *gp)
{
    PD_ASSERT (gpointer_isScalar (gp)); return (gp->gp_un.gp_scalar);
}

t_word *gpointer_getWord (t_gpointer *gp)
{
    PD_ASSERT (gpointer_isWord (gp)); return (gp->gp_un.gp_w);
}

t_glist *gpointer_getParentGlist (t_gpointer *gp)
{
    PD_ASSERT (gp->gp_master->gm_type == GPOINTER_GLIST);
    
    return (gp->gp_master->gm_un.gm_glist);
}

t_array *gpointer_getParentArray (t_gpointer *gp)
{
    PD_ASSERT (gp->gp_master->gm_type == GPOINTER_ARRAY);
    
    return (gp->gp_master->gm_un.gm_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_word *gpointer_getData (t_gpointer *gp)
{
    if (gpointer_isWord (gp))       { return gpointer_getWord (gp); } 
    else if (!gpointer_isNull (gp)) { return (scalar_getData (gpointer_getScalar (gp))); }
    
    return NULL;
}

t_glist *gpointer_getView (t_gpointer *gp)
{
    if (gpointer_isScalar (gp)) { return gpointer_getParentGlist (gp); }
    else {
        return (gpointer_getParentGlist (array_getTopParent (gpointer_getParentArray (gp))));
    }
}

t_symbol *gpointer_getTemplateIdentifier (t_gpointer *gp)
{
    t_gmaster *master = gp->gp_master;
    
    PD_ASSERT (gpointer_isValidNullAllowed (gp));
    
    if (master->gm_type == GPOINTER_GLIST) {
        if (!gpointer_isNull (gp)) { return (scalar_getTemplateIdentifier (gpointer_getScalar (gp))); }
        
    } else {
        return (array_getTemplateIdentifier (master->gm_un.gm_array));
    }
    
    return &s_;
}

t_template *gpointer_getTemplate (t_gpointer *gp)
{
    t_template *template = template_findByIdentifier (gpointer_getTemplateIdentifier (gp));
    
    PD_ASSERT (template);
    
    return (template);
}

static t_scalar *gpointer_getBase (t_gpointer *gp)
{
    t_scalar *scalar = NULL;
    
    if (gpointer_isScalar (gp)) { scalar = gpointer_getScalar (gp); }
    else {
        scalar = gpointer_getScalar (array_getTopParent (gpointer_getParentArray (gp)));
    }
    
    return scalar;
}

int gpointer_isInstanceOf (t_gpointer *gp, t_symbol *templateIdentifier)
{
    if (templateIdentifier == template_getWildcard())               { return 1; }
    if (templateIdentifier == gpointer_getTemplateIdentifier (gp))  { return 1; }
    
    return 0;
}

int gpointer_isValidInstanceOf (t_gpointer *gp, t_symbol *templateIdentifier)
{
    if (!gpointer_isValid (gp))                             { return 0; }
    if (!gpointer_isInstanceOf (gp, templateIdentifier))    { return 0; }
    if (!gpointer_getTemplate (gp))                         { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_redraw (t_gpointer *gp)
{
    scalar_redraw (gpointer_getBase (gp), gpointer_getView (gp));
}

void gpointer_setVisibility (t_gpointer *gp, int isVisible)
{
    gobj_visibilityChanged (cast_gobj (gpointer_getBase (gp)), gpointer_getView (gp), isVisible); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gpointer_hasField (t_gpointer *gp, t_symbol *fieldName)
{
    return (template_hasField (gpointer_getTemplate (gp), fieldName));
}

int gpointer_fieldIsFloat (t_gpointer *gp, t_symbol *fieldName)
{
    return (template_fieldIsFloat (gpointer_getTemplate (gp), fieldName));
}

int gpointer_fieldIsSymbol (t_gpointer *gp, t_symbol *fieldName)
{
    return (template_fieldIsSymbol (gpointer_getTemplate (gp), fieldName));
}

int gpointer_fieldIsText (t_gpointer *gp, t_symbol *fieldName)
{
    return (template_fieldIsText (gpointer_getTemplate (gp), fieldName));
}

int gpointer_fieldIsArray (t_gpointer *gp, t_symbol *fieldName)
{
    return (template_fieldIsArray (gpointer_getTemplate (gp), fieldName));
}

int gpointer_fieldIsArrayAndValid (t_gpointer *gp, t_symbol *fieldName)
{
    return (template_fieldIsArrayAndValid (gpointer_getTemplate (gp), fieldName));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float gpointer_getFloat (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getFloat (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName);
}

t_symbol *gpointer_getSymbol (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getSymbol (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName);
}

t_buffer *gpointer_getText (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getText (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName);
}

t_array *gpointer_getArray (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getArray (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName);
}

void gpointer_setFloat (t_gpointer *gp, t_symbol *fieldName, t_float f)
{
    word_setFloat (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName, f);
}

void gpointer_setSymbol (t_gpointer *gp, t_symbol *fieldName, t_symbol *s)
{
    word_setSymbol (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float gpointer_getFloatByDescriptor (t_gpointer *gp, t_fielddescriptor *fd)
{
    word_getFloatByDescriptor (gpointer_getData (gp), gpointer_getTemplate (gp), fd);
}

t_float gpointer_getFloatByDescriptorAsPosition (t_gpointer *gp, t_fielddescriptor *fd)
{
    word_getFloatByDescriptorAsPosition (gpointer_getData (gp), gpointer_getTemplate (gp), fd);
}

void gpointer_setFloatByDescriptorAsPosition (t_gpointer *gp, t_fielddescriptor *fd, t_float position)
{
    word_setFloatByDescriptorAsPosition (gpointer_getData (gp), gpointer_getTemplate (gp), fd, position);
}
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
