
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
#include "s_midi.h"
#include "x_control.h"

/*----------midirealtimein (midi FA,FB,FC,FF message )-----------------*/

static t_class *midirealtimein_class;

typedef struct _midirealtimein
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_midirealtimein;

static void *midirealtimein_new( void)
{
    t_midirealtimein *x = (t_midirealtimein *)pd_new(midirealtimein_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__midirealtimein);
    return (x);
}

static void midirealtimein_list(t_midirealtimein *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_float portno = atom_getFloatAtIndex(0, argc, argv);
    t_float byte = atom_getFloatAtIndex(1, argc, argv);

    outlet_float(x->x_outlet2, portno);
    outlet_float(x->x_outlet1, byte);
}

static void midirealtimein_free(t_midirealtimein *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__midirealtimein);
}

void midirealtimein_setup(void)
{
    midirealtimein_class = class_new(sym_midirealtimein, 
        (t_newmethod)midirealtimein_new, (t_method)midirealtimein_free, 
            sizeof(t_midirealtimein), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(midirealtimein_class, midirealtimein_list);
        class_setHelpName(midirealtimein_class, sym_midiout);
}
