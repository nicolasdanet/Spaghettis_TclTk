
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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

extern t_pdinstance     *pd_this;

/* ------------------------- DSP chain handling ------------------------- */

    /* schedule one canvas for DSP.  This is called below for all "root"
    canvases, but is also called from the "dsp" method for sub-
    canvases, which are treated almost like any other tilde object.  */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        ugen_start                                  (void);
void        ugen_stop                                   (void);

t_dspcontext *ugen_start_graph (int toplevel, t_signal **sp, int ninlets, int noutlets);

void        ugen_add(t_dspcontext *dc, t_object *x);
void        ugen_connect(t_dspcontext *dc, t_object *x1, int outno, t_object *x2, int inno);
void        ugen_done_graph(t_dspcontext *dc);

static void canvas_dodsp(t_glist *x, int toplevel, t_signal **sp)
{
    t_linetraverser t;
    t_outconnect *oc;
    t_gobj *y;
    t_object *ob;
    t_symbol *dspsym = gensym("dsp");
    t_dspcontext *dc;    

        /* create a new "DSP graph" object to use in sorting this canvas.
        If we aren't toplevel, there are already other dspcontexts around. */

    dc = ugen_start_graph(toplevel, sp,
        object_numberOfSignalInlets(&x->gl_obj),
        object_numberOfSignalOutlets(&x->gl_obj));

        /* find all the "dsp" boxes and add them to the graph */
    
    for (y = x->gl_list; y; y = y->g_next)
        if ((ob = canvas_castToObjectIfBox(&y->g_pd)) && class_hasMethod (pd_class (&y->g_pd), dspsym))
            ugen_add(dc, ob);

        /* ... and all dsp interconnections */
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
        if (object_isSignalOutlet(t.tr_srcObject, t.tr_srcIndexOfOutlet))
            ugen_connect(dc, t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);

        /* finally, sort them and add them to the DSP chain */
    ugen_done_graph(dc);
}

void canvas_dsp(t_glist *x, t_signal **sp)
{
    canvas_dodsp(x, 0, sp);
}

    /* this routine starts DSP for all root canvases. */
static void canvas_start_dsp(void)
{
    t_glist *x;
    if (pd_this->pd_dspState) ugen_stop();
    else sys_gui("set ::var(isDsp) 1\n");
    ugen_start();
    
    for (x = pd_this->pd_roots; x; x = x->gl_next)
        canvas_dodsp(x, 1, 0);
    
    pd_this->pd_dspState = 1;
}

static void canvas_stop_dsp(void)
{
    if (pd_this->pd_dspState)
    {
        ugen_stop();
        sys_gui("set ::var(isDsp) 0\n");
        pd_this->pd_dspState = 0;
    }
}

    /* DSP can be suspended before, and resumed after, operations which
    might affect the DSP chain.  For example, we suspend before loading and
    resume afterward, so that DSP doesn't get resorted for every DSP object
    int the patch. */

int canvas_suspend_dsp(void)
{
    int rval = pd_this->pd_dspState;
    if (rval) canvas_stop_dsp();
    return (rval);
}

void canvas_resume_dsp(int oldstate)
{
    if (oldstate) canvas_start_dsp();
}

    /* this is equivalent to suspending and resuming in one step. */
void canvas_update_dsp(void)
{
    if (pd_this->pd_dspState) canvas_start_dsp();
}

void global_dsp(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int newstate;
    
    if (argc)
    {
        newstate = (t_int)atom_getFloatAtIndex (0, argc, argv);
        
        if (newstate && !pd_this->pd_dspState)
        {
            if (audio_startDSP() == PD_ERROR_NONE) { canvas_start_dsp(); }
        }
        else if (!newstate && pd_this->pd_dspState)
        {
            canvas_stop_dsp();
            audio_stopDSP();
        }
    }
}

void *canvas_getblock(t_class *blockclass, t_glist **canvasp)
{
    t_glist *canvas = *canvasp;
    t_gobj *g;
    void *ret = 0;
    for (g = canvas->gl_list; g; g = g->g_next)
    {
        if (g->g_pd == blockclass)
            ret = g;
    }
    *canvasp = canvas->gl_owner;
    return(ret);
}
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
