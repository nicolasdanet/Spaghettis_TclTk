
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "g_graphics.h"
#include "x_text.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error textclient_init (t_textclient *x, int *ac, t_atom **av)
{
    int argc = *ac;
    t_atom *argv = *av;
    
    x->tc_name               = NULL;
    x->tc_templateIdentifier = NULL;
    x->tc_fieldName          = NULL;
    
    gpointer_init (&x->tc_gpointer);
    
    while (argc && IS_SYMBOL (argv)) {
    //
    t_symbol *t = GET_SYMBOL (argv);
    
    if (t == sym___dash__s || t == sym___dash__t || t == sym___dash__template) {
        if (argc >= 3 && IS_SYMBOL (argv + 1) && IS_SYMBOL (argv + 2)) {
            x->tc_templateIdentifier = template_makeIdentifierWithWildcard (GET_SYMBOL (argv + 1));
            x->tc_fieldName = GET_SYMBOL (argv + 2);
            argc -= 3; argv += 3;
        } else {
            return PD_ERROR;
        }
    }
    
    break;
    //
    }
    
    if (!x->tc_templateIdentifier && argc) {
        
        /* Dollar expansion is zero in abstraction opened as patch. */
        
        if (IS_FLOAT (argv) && (GET_FLOAT (argv) == 0.0)) { x->tc_name = &s_; argc--; argv++; }
        else {
            if (!IS_SYMBOL (argv)) { return PD_ERROR; }
            else {
                x->tc_name = GET_SYMBOL (argv); argc--; argv++;
            }
        }
    }
        
    *ac = argc;
    *av = argv;
    
    return PD_ERROR_NONE;
}

void textclient_free (t_textclient *x)
{
    gpointer_unset (&x->tc_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer *textclient_fetchBuffer (t_textclient *x)
{
    if (x->tc_name) {

        t_textbuffer *y = (t_textbuffer *)pd_getThingByClass (x->tc_name, textdefine_class);

        if (y) { return textbuffer_getBuffer (y); }
        else {
            error_canNotFind (sym_text, x->tc_name);
        }

    } else if (x->tc_templateIdentifier) {

        if (gpointer_isValidInstanceOf (&x->tc_gpointer, x->tc_templateIdentifier)) {
            if (gpointer_hasField (&x->tc_gpointer, x->tc_fieldName)) {
                if (gpointer_fieldIsText (&x->tc_gpointer, x->tc_fieldName)) {
                    return gpointer_getText (&x->tc_gpointer, x->tc_fieldName);
                    
        } else { error_invalid (sym_text, x->tc_fieldName); }
        } else { error_missingField (sym_text, x->tc_fieldName); }
        } else { error_invalid (sym_text, &s_pointer); }
    }
    
    return NULL;
}

t_glist *textclient_fetchView (t_textclient *x)
{
    if (x->tc_name) {

        t_textbuffer *y = (t_textbuffer *)pd_getThingByClass (x->tc_name, textdefine_class);

        if (y) { return textbuffer_getView (y); }
        else {
            error_canNotFind (sym_text, x->tc_name);
        }

    } else if (x->tc_templateIdentifier) {

        if (gpointer_isValidInstanceOf (&x->tc_gpointer, x->tc_templateIdentifier)) {
            if (gpointer_hasField (&x->tc_gpointer, x->tc_fieldName)) {
                if (gpointer_fieldIsText (&x->tc_gpointer, x->tc_fieldName)) {
                    return gpointer_getView (&x->tc_gpointer);
                    
        } else { error_invalid (sym_text, x->tc_fieldName); }
        } else { error_missingField (sym_text, x->tc_fieldName); }
        } else { error_invalid (sym_text, &s_pointer); }
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textclient_update (t_textclient *x)
{
    if (x->tc_name) {

        t_textbuffer *y = (t_textbuffer *)pd_getThingByClass (x->tc_name, textdefine_class);
        
        if (y) { textbuffer_update (y); }
        else { 
            error_canNotFind (sym_text, x->tc_name);
        }

    } else if (x->tc_templateIdentifier) {

        if (gpointer_isValidInstanceOf (&x->tc_gpointer, x->tc_templateIdentifier)) {
            gpointer_redraw (&x->tc_gpointer);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
