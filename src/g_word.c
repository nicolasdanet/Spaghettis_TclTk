
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

void word_init (t_word *w, t_template *tmpl, t_gpointer *gp)
{
    int i, size = template_getSize (tmpl);
    t_dataslot *v = template_getData (tmpl);
    
    for (i = 0; i < size; i++, v++, w++) {
    //
    int type = v->ds_type;
    
    switch (type) {
        case DATA_FLOAT  : w->w_float  = 0.0;                                       break;
        case DATA_SYMBOL : w->w_symbol = &s_symbol;                                 break;
        case DATA_TEXT   : w->w_buffer = buffer_new();                              break;
        case DATA_ARRAY  : w->w_array  = array_new (v->ds_templateIdentifier, gp);  break;
    }
    //
    }
}

void word_restore (t_word *w, t_template *tmpl, int argc, t_atom *argv)
{
    int i, size = template_getSize (tmpl);
    t_dataslot *v = template_getData (tmpl);
    
    for (i = 0; i < size; i++, v++, w++) {
    //
    int type = v->ds_type;
    
    if (type == DATA_FLOAT) {
        t_float f = 0.0;
        if (argc) { f = atom_getFloat (argv); argv++; argc--; }
        w->w_float = f; 
        
    } else if (type == DATA_SYMBOL) {
        t_symbol *s = &s_;
        if (argc) { s = atom_getSymbol (argv); argv++; argc--; }
        w->w_symbol = s;
    }
    //
    }
    
    PD_ASSERT (argc == 0);
}

void word_free (t_word *w, t_template *tmpl)
{
    if (!tmpl) { PD_BUG; }
    else {
    //
    int i;
    t_dataslot *v = template_getData (tmpl);
    
    for (i = 0; i < template_getSize (tmpl); i++) {
        if (v->ds_type == DATA_ARRAY) { array_free (w[i].w_array); }
        else if (v->ds_type == DATA_TEXT) { buffer_free (w[i].w_buffer); }
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

    return 0.0;
}

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

void word_setSymbol (t_word *w, t_template *tmpl, t_symbol *fieldName, t_symbol *s)
{
    int i, type;
    t_symbol *dummy = NULL;
    
    PD_ASSERT (template_fieldIsSymbol (tmpl, fieldName));
    
    if (template_getRaw (tmpl, fieldName, &i, &type, &dummy)) {
        if (type == DATA_SYMBOL) { 
            *(t_symbol **)(w + i) = s;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float word_getFloatByField (t_word *w, t_template *tmpl, t_fielddescriptor *fd)
{
    if (fd->fd_type == DATA_FLOAT) {
    //
    if (fd->fd_isVariable) { return (word_getFloat (w, tmpl, fd->fd_un.fd_variableName)); }
    else {
        return (fd->fd_un.fd_float);
    }
    //
    }

    PD_BUG;
    
    return 0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float word_getFloatByFieldAsPosition (t_fielddescriptor *fd, t_template *tmpl, t_word *w)
{
    if (fd->fd_type == DATA_FLOAT) {
    //
    if (fd->fd_isVariable) {
        return (field_convertValueToPosition (fd, word_getFloat (w, tmpl, fd->fd_un.fd_variableName)));
    } else {
        return (fd->fd_un.fd_float);
    }
    //
    }

    PD_BUG; 
    
    return 0.0;
}

void word_setFloatByFieldAsPosition (t_fielddescriptor *fd, t_template *tmpl, t_word *w, t_float position)
{
    if (fd->fd_type == DATA_FLOAT) {
    //
    if (fd->fd_isVariable) {
        word_setFloat (w, tmpl, fd->fd_un.fd_variableName, field_convertPositionToValue (fd, position));
    } else {
        fd->fd_un.fd_float = position;
    }
    //
    } else {
        PD_BUG;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
