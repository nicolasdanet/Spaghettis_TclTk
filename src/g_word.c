
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void word_init (t_word *w, t_template *tmpl, t_gpointer *gp)
{
    t_dataslot *v = template_getSlots (tmpl);
    int size = template_getSize (tmpl);
    int i;
    
    for (i = 0; i < size; i++, v++, w++) {
    //
    int type = v->ds_type;
    
    switch (type) {
        case DATA_FLOAT  : WORD_FLOAT (w)  = (t_float)0.0;                              break;
        case DATA_SYMBOL : WORD_SYMBOL (w) = &s_symbol;                                 break;
        case DATA_TEXT   : WORD_BUFFER (w) = buffer_new();                              break;
        case DATA_ARRAY  : WORD_ARRAY (w)  = array_new (v->ds_templateIdentifier, gp);  break;
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
        if (v->ds_type == DATA_ARRAY)     { array_free (WORD_ARRAY (w + i)); }
        else if (v->ds_type == DATA_TEXT) { buffer_free (WORD_BUFFER (w + i)); }
        v++;
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float word_getFloat (t_word *w, t_template *tmpl, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_FLOAT) {
            return *(t_float *)(w + i);
        }
    }

    return (t_float)0.0;
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

t_buffer *word_getText (t_word *w, t_template *tmpl, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_TEXT) {
            return *(t_buffer **)(w + i);
        }
    }

    return NULL;
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
#pragma mark -

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

void word_setText (t_word *w, t_template *tmpl, t_symbol *fieldName, t_buffer *b)
{
    int i, type; t_symbol *dummy = NULL;
    
    PD_ASSERT (b);
    PD_ASSERT (template_fieldIsText (tmpl, fieldName));
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_TEXT) {
            t_buffer *x = *(t_buffer **)(w + i); buffer_reset (x); buffer_appendBuffer (x, b);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float word_getFloatByDescriptor (t_word *w, t_template *tmpl, t_fielddescriptor *fd)
{
    if (fd->fd_type == DATA_FLOAT) {
    //
    if (fd->fd_isVariable) { return word_getFloat (w, tmpl, fd->fd_un.fd_variableName); }
    else {
        return (fd->fd_un.fd_float);
    }
    //
    }

    PD_BUG;
    
    return (t_float)0.0;
}

void word_setFloatByDescriptor (t_word *w, t_template *tmpl, t_fielddescriptor *fd, t_float f)
{
    if (fd->fd_type == DATA_FLOAT) {
    //
    if (fd->fd_isVariable) {
        word_setFloat (w, tmpl, fd->fd_un.fd_variableName, f);
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
#pragma mark -

/* Edge case in which the buffer is replaced by an outside one. */
/* Requires extra care. */

t_error word_setInternalBuffer (t_word *w, t_template *tmpl, t_symbol *fieldName, t_buffer *b)
{
    t_error err = PD_ERROR;
    
    if (b && template_fieldIsText (tmpl, fieldName)) {
    //
    int i, type;
    t_symbol *dummy = NULL;
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_TEXT) {
            t_buffer *x = *(t_buffer **)(w + i); buffer_free (x); *(t_buffer **)(w + i) = b;
            err = PD_ERROR_NONE;
        }
    }
    //
    }
    
    return err;
}

t_error word_unsetInternalBuffer (t_word *w, t_template *tmpl, t_symbol *fieldName)
{
    t_error err = PD_ERROR;
    
    if (template_fieldIsText (tmpl, fieldName)) {
    //
    int i, type;
    t_symbol *dummy = NULL;
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_TEXT) {
            *(t_buffer **)(w + i) = buffer_new();
            err = PD_ERROR_NONE;
        }
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
