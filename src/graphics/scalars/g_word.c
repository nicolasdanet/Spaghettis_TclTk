
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void word_init (t_word *w, t_template *tmpl, t_gpointer *gp)
{
    t_dataslot *v = template_getSlots (tmpl);
    int isPrivate = template_isPrivate (template_getTemplateIdentifier (tmpl));
    int i;
    
    for (i = 0; i < template_getSize (tmpl); i++, v++, w++) {
    //
    t_constructor *ctor = NULL;
    
    if (!isPrivate) { ctor = template_getInstanceConstructorIfAny (tmpl, v->ds_fieldName); }
    
    switch (v->ds_type) {
        case DATA_FLOAT  :  WORD_FLOAT (w)  = ctor ? constructor_evaluateAsFloat (ctor)  : 0.0;       break;
        case DATA_SYMBOL :  WORD_SYMBOL (w) = ctor ? constructor_evaluateAsSymbol (ctor) : &s_symbol; break;
        case DATA_ARRAY  :  int n = (int)(ctor ? constructor_evaluateAsFloat (ctor) : 0.0);
                            WORD_ARRAY (w) = array_new (v->ds_templateIdentifier, gp);
                            if (n > 1) { array_resize (WORD_ARRAY (w), n); }
                            break;
    }
    //
    }
}

void word_free (t_word *w, t_template *tmpl)
{
    if (!tmpl) { PD_BUG; }
    else {
    //
    int i;
    t_dataslot *v = template_getSlots (tmpl);
    
    for (i = 0; i < template_getSize (tmpl); i++) {
        if (v->ds_type == DATA_ARRAY) { array_free (WORD_ARRAY (w + i)); }
        v++;
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float word_getFloat (t_word *w, t_template *tmpl, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_FLOAT) {
            return *(t_float *)(w + i);
        }
    }

    return 0.0;
}

t_symbol *word_getSymbol (t_word *w, t_template *tmpl, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_SYMBOL) {
            return *(t_symbol **)(w + i);
        }
    }

    return &s_;
}

t_array *word_getArray (t_word *w, t_template *tmpl, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_ARRAY) {
            return *(t_array **)(w + i);
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void word_setFloat (t_word *w, t_template *tmpl, t_symbol *fieldName, t_float f)
{
    int i, type; t_symbol *dummy = NULL;
    
    PD_ASSERT (template_fieldIsFloat (tmpl, fieldName));
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_FLOAT) { 
            *(t_float *)(w + i) = f; 
        }
    }
}

void word_setSymbol (t_word *w, t_template *tmpl, t_symbol *fieldName, t_symbol *s)
{
    int i, type; t_symbol *dummy = NULL;
    
    PD_ASSERT (template_fieldIsSymbol (tmpl, fieldName));
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_SYMBOL) { 
            *(t_symbol **)(w + i) = s;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float word_getFloatByDescriptor (t_word *w, t_template *tmpl, t_fielddescriptor *fd)
{
    t_float f = 0.0;
    
    if (fd->fd_type == DATA_FLOAT) {
    //
    if (fd->fd_isVariable) {
        f = word_getFloat (w, tmpl, fd->fd_un.fd_variableName);
        if (fd->fd_isVariableOpposite) { return (-f + fd->fd_offset); }
        else {
            return (f + fd->fd_offset);
        }
    
    } else {
        return (fd->fd_un.fd_float);
    }
    //
    }

    PD_BUG;
    
    return f;
}

void word_setFloatByDescriptor (t_word *w, t_template *tmpl, t_fielddescriptor *fd, t_float f)
{
    if (fd->fd_type == DATA_FLOAT) {
    //
    if (fd->fd_isVariable) {
        
        f -= fd->fd_offset;
        
        if (fd->fd_isVariableOpposite) {
            word_setFloat (w, tmpl, fd->fd_un.fd_variableName, (-f));
        } else {
            word_setFloat (w, tmpl, fd->fd_un.fd_variableName, (f));
        }
        
    } else {
        fd->fd_un.fd_float = f;
    }
    //
    } else {
        PD_BUG;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
