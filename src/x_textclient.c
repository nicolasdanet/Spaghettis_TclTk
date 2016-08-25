
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *textdefine_class; 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textclient_error (t_textclient *x)
{
    post_error (PD_TRANSLATE ("text: couldn't find %s"), x->tc_name->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void textclient_init (t_textclient *x, int *ac, t_atom **av)
{
    int argc = *ac;
    t_atom *argv = *av;
    
    x->tc_name               = NULL;
    x->tc_templateIdentifier = NULL;
    x->tc_fieldName          = NULL;
    
    gpointer_init (&x->tc_gpointer);
    
    while (argc && IS_SYMBOL (argv)) {
    //
    if (GET_SYMBOL (argv) == sym___dash__s || GET_SYMBOL (argv) == sym___dash__symbol) {
        if (argc >= 3 && IS_SYMBOL (argv + 1) && IS_SYMBOL (argv + 2)) {
            x->tc_templateIdentifier = template_makeIdentifierWithWildcard (GET_SYMBOL (argv + 1));
            x->tc_fieldName = GET_SYMBOL (argv + 2);
            argc -= 3; argv += 3;
        }
    }
    
    break;
    //
    }
    
    if (argc && IS_SYMBOL (argv) && !x->tc_templateIdentifier) {
        x->tc_name = GET_SYMBOL (argv); 
        argc--; argv++;
    }
    
    *ac = argc;
    *av = argv;
}

void textclient_free (t_textclient *x)
{
    gpointer_unset (&x->tc_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_buffer *textclient_fetchBuffer (t_textclient *x)
{
    if (x->tc_name) {
    //
    t_textbuffer *y = (t_textbuffer *)pd_findByClass (x->tc_name, textdefine_class);

    if (y) { return textbuffer_getBuffer (y); }
    else {
        textclient_error (x);
    }
    //
    } else if (x->tc_templateIdentifier) {
    //
    if (gpointer_isValidInstanceOf (&x->tc_gpointer, x->tc_templateIdentifier)) {
        if (gpointer_hasField (&x->tc_gpointer, x->tc_fieldName)) {
            if (gpointer_fieldIsText (&x->tc_gpointer, x->tc_fieldName)) {
                return gpointer_getText (&x->tc_gpointer, x->tc_fieldName);
            }
        }
    }
    
    pointer_error (sym_text);
    //
    }
    
    return NULL;
}

void textclient_update (t_textclient *x)
{
    if (x->tc_name) {
    //
    t_textbuffer *y = (t_textbuffer *)pd_findByClass (x->tc_name, textdefine_class);
    
    if (y) { textbuffer_update (y); }
    else { 
        textclient_error (x);
    }
    //
    } else if (x->tc_templateIdentifier) {
    //
    if (gpointer_isValidInstanceOf (&x->tc_gpointer, x->tc_templateIdentifier)) {
        gpointer_redraw (&x->tc_gpointer);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
