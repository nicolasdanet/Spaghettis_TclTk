
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
#include "m_alloca.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *textget_class;                     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textget {
    t_textclient    x_textclient;           /* Must be the first. */
    t_float         x_startField;
    t_float         x_numberOfFields;
    t_outlet        *x_outletLeft;
    t_outlet        *x_outletRight;
    } t_textget;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textget_float(t_textget *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_textclient);
    int start, end, n, startfield, nfield;
    t_atom *vec;
    if (!b)
       return;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    startfield = x->x_startField;
    nfield = x->x_numberOfFields;
    if (buffer_getMessageAt(b, f, &start, &end))
    {
        int outc = end - start, k;
        t_atom *outv;
        if (x->x_startField < 0)    /* negative start field for whole line */
        {
                /* tell us what terminated the line (semi or comma) */
            outlet_float(x->x_outletRight, (end < n && vec[end].a_type == A_COMMA));
            ATOMS_ALLOCA(outv, outc);
            for (k = 0; k < outc; k++)
                outv[k] = vec[start+k];
            outlet_list(x->x_outletLeft, 0, outc, outv);
            ATOMS_FREEA(outv, outc);
        }
        else if (startfield + nfield > outc)
            post_error ("text get: field request (%d %d) out of range",
                startfield, nfield); 
        else
        {
            ATOMS_ALLOCA(outv, nfield);
            for (k = 0; k < nfield; k++)
                outv[k] = vec[(start+startfield)+k];
            outlet_list(x->x_outletLeft, 0, nfield, outv);
            ATOMS_FREEA(outv, nfield);
        }
    }
    else if (x->x_startField < 0)   /* whole line but out of range: empty list and 2 */
    {
        outlet_float(x->x_outletRight, 2);         /* 2 for out of range */
        outlet_list(x->x_outletLeft, 0, 0, 0);    /* ... and empty list */
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textget_new(t_symbol *s, int argc, t_atom *argv)
{
    t_textget *x = (t_textget *)pd_new(textget_class);
    x->x_outletLeft = outlet_new (cast_object (x), &s_list);
    x->x_outletRight = outlet_new(cast_object (x), &s_float);
    inlet_newFloat(cast_object (x), &x->x_startField);
    inlet_newFloat(cast_object (x), &x->x_numberOfFields);
    x->x_startField = -1;
    x->x_numberOfFields = 1;
    textclient_init(&x->x_textclient, &argc, &argv);
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_startField = argv->a_w.w_float;
        else
        {
            post("text get: can't understand field number");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_numberOfFields = argv->a_w.w_float;
        else
        {
            post("text get: can't understand field count");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text get ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (TEXTCLIENT_ASPOINTER (x->x_textclient))
        inlet_newPointer(cast_object (x), TEXTCLIENT_GETPOINTER (x->x_textclient));
    else inlet_newSymbol(cast_object (x), TEXTCLIENT_GETNAME (x->x_textclient));
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textget_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__get,
            (t_newmethod)textget_new,
            (t_method)textclient_free,
            sizeof (t_textget),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, textget_float);
    class_setHelpName (c, sym_text);
    
    textget_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
