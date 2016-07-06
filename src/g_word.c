
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
#include "s_system.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void word_init(t_word *wp, t_template *template, t_gpointer *gp)
{
    int i, nitems = template->tp_size;
    t_dataslot *datatypes = template->tp_vector;
    for (i = 0; i < nitems; i++, datatypes++, wp++)
    {
        int type = datatypes->ds_type;
        if (type == DATA_FLOAT)
            wp->w_float = 0; 
        else if (type == DATA_SYMBOL)
            wp->w_symbol = &s_symbol;
        else if (type == DATA_ARRAY)
            wp->w_array = array_new(datatypes->ds_template, gp);
        else if (type == DATA_TEXT)
            wp->w_buffer = buffer_new();
    }
}

void word_restore(t_word *wp, t_template *template,
    int argc, t_atom *argv)
{
    int i, nitems = template->tp_size;
    t_dataslot *datatypes = template->tp_vector;
    for (i = 0; i < nitems; i++, datatypes++, wp++)
    {
        int type = datatypes->ds_type;
        if (type == DATA_FLOAT)
        {
            t_float f;
            if (argc)
            {
                f =  atom_getFloat(argv);
                argv++, argc--;
            }
            else f = 0;
            wp->w_float = f; 
        }
        else if (type == DATA_SYMBOL)
        {
            t_symbol *s;
            if (argc)
            {
                s = atom_getSymbol(argv);
                argv++, argc--;
            }
            else s = &s_;
            wp->w_symbol = s;
        }
    }
    if (argc)
        post("warning: word_restore: extra arguments");
}

void word_free(t_word *wp, t_template *template)
{
    int i;
    t_dataslot *dt;
    for (dt = template->tp_vector, i = 0; i < template->tp_size; i++, dt++)
    {
        if (dt->ds_type == DATA_ARRAY)
            array_free(wp[i].w_array);
        else if (dt->ds_type == DATA_TEXT)
            buffer_free(wp[i].w_buffer);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
