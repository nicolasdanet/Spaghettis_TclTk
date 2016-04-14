/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  a thing to forward messages from the GUI, dealing with race conditions
in which the "target" gets deleted while the GUI is sending it something.

See also the gfxstub object that doesn't oblige the owner to keep a pointer
around (so is better suited to one-off dialogs)
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

struct _guiconnect
{
    t_object x_obj;
    t_pd *x_who;
    t_symbol *x_sym;
    t_clock *x_clock;
};

static t_class *guiconnect_class;

t_guiconnect *guiconnect_new(t_pd *who, t_symbol *sym)
{
    t_guiconnect *x = (t_guiconnect *)pd_new(guiconnect_class);
    x->x_who = who;
    x->x_sym = sym;
    pd_bind(&x->x_obj.te_g.g_pd, sym);
    return (x);
}

    /* cleanup routine; delete any resources we have */
static void guiconnect_free(t_guiconnect *x)
{
    if (x->x_sym)
        pd_unbind(&x->x_obj.te_g.g_pd, x->x_sym);
    if (x->x_clock)
        clock_free(x->x_clock); 
}

    /* this is called when the clock times out to indicate the GUI should
    be gone by now. */
static void guiconnect_tick(t_guiconnect *x)
{
    pd_free(&x->x_obj.te_g.g_pd);
}

    /* the target calls this to disconnect.  If the gui has "signed off"
    we're ready to delete the object; otherwise we wait either for signoff
    or for a timeout. */
void guiconnect_notarget(t_guiconnect *x, double timedelay)
{
    if (!x->x_sym)
        pd_free(&x->x_obj.te_g.g_pd);
    else
    {
        x->x_who = 0;
        if (timedelay > 0)
        {
            x->x_clock = clock_new(x, (t_method)guiconnect_tick);
            clock_delay(x->x_clock, timedelay);
        }
    }    
}

    /* the GUI calls this to send messages to the target. */
static void guiconnect_anything(t_guiconnect *x,
    t_symbol *s, int ac, t_atom *av)
{
    if (x->x_who)
        pd_message(x->x_who, s, ac, av);
}

    /* the GUI calls this when it disappears.  (If there's any chance the
    GUI will fail to do this, the "target", when it signs off, should specify
    a timeout after which the guiconnect will disappear.) */
static void guiconnect_signoff(t_guiconnect *x)
{
    if (!x->x_who)
        pd_free(&x->x_obj.te_g.g_pd);
    else
    {
        pd_unbind(&x->x_obj.te_g.g_pd, x->x_sym);
        x->x_sym = 0;
    }
}

void g_guiconnect_setup(void)
{
    guiconnect_class = class_new(gensym("guiconnect"), 0,
        (t_method)guiconnect_free, sizeof(t_guiconnect), CLASS_PURE, 0);
    class_addAnything(guiconnect_class, guiconnect_anything);
    class_addMethod(guiconnect_class, (t_method)guiconnect_signoff,
        gensym("signoff"), 0);
}

/* --------------------- graphics responder  ---------------- */

/* make one of these if you want to put up a dialog window but want to be
protected from getting deleted and then having the dialog call you back.  In
this design the calling object doesn't have to keep the address of the dialog
window around; instead we keep a list of all open dialogs.  Any object that
might have dialogs, when it is deleted, simply checks down the dialog window
list and breaks off any dialogs that might later have sent messages to it. 
Only when the dialog window itself closes do we delete the gfxstub object. */

static t_class *gfxstub_class;

typedef struct _gfxstub
{
    t_pd x_pd;
    t_pd *x_owner;
    void *x_key;
    t_symbol *x_sym;
    struct _gfxstub *x_next;
} t_gfxstub;

static t_gfxstub *gfxstub_list;

    /* create a new one.  the "key" is an address by which the owner
    will identify it later; if the owner only wants one dialog, this
    could just be a pointer to the owner itself.  The string "cmd"
    is a TK command to create the dialog, with "%s" embedded in
    it so we can provide a name by which the GUI can send us back
    messages; e.g., "pdtk_canvas_dofont %s 10". */

void gfxstub_new(t_pd *owner, void *key, const char *cmd)
{
    char buf[4*PD_STRING];
    char namebuf[80];
    char sprintfbuf[PD_STRING];
    char *afterpercent;
    t_int afterpercentlen;
    t_gfxstub *x;
    t_symbol *s;
        /* if any exists with matching key, burn it. */
    for (x = gfxstub_list; x; x = x->x_next)
        if (x->x_key == key)
            gfxstub_deleteforkey(key);
    if (strlen(cmd) + 50 > 4*PD_STRING)
    {
        PD_BUG;
        return;
    }
    x = (t_gfxstub *)pd_new(gfxstub_class);
    sprintf(namebuf, ".gfxstub%lx", (t_int)x);

    s = gensym(namebuf);
    pd_bind(&x->x_pd, s);
    x->x_owner = owner;
    x->x_sym = s;
    x->x_key = key;
    x->x_next = gfxstub_list;
    gfxstub_list = x;
    /* only replace first %s so sprintf() doesn't crash */
    afterpercent = strchr(cmd, '%') + 2;
    afterpercentlen = afterpercent - cmd;
    strncpy(sprintfbuf, cmd, afterpercentlen);
    sprintfbuf[afterpercentlen] = '\0';
    sprintf(buf, sprintfbuf, s->s_name);
    strncat(buf, afterpercent, (4*PD_STRING) - afterpercentlen);
    sys_gui(buf);
}

static void gfxstub_offlist(t_gfxstub *x)
{
    t_gfxstub *y1, *y2;
    if (gfxstub_list == x)
        gfxstub_list = x->x_next;
    else for (y1 = gfxstub_list; y2 = y1->x_next; y1 = y2)
        if (y2 == x) 
    {
        y1->x_next = y2->x_next;
        break;
    }
}

    /* if the owner disappears, we still may have to stay around until our
    dialog window signs off.  Anyway we can now tell the GUI to destroy the
    window.  */
void gfxstub_deleteforkey(void *key)
{
    t_gfxstub *y;
    int didit = 1;
    while (didit)
    {
        didit = 0;
        for (y = gfxstub_list; y; y = y->x_next)
        {
            if (y->x_key == key)
            {
                sys_vGui("destroy .gfxstub%lx\n", y);
                y->x_owner = 0;
                gfxstub_offlist(y);
                didit = 1;
                break;
            }
        }
    }
}

/* --------- pd messages for gfxstub (these come from the GUI) ---------- */

    /* "cancel" to request that we close the dialog window. */
static void gfxstub_cancel(t_gfxstub *x)
{
    gfxstub_deleteforkey(x->x_key);
}

    /* "signoff" comes from the GUI to say the dialog window closed. */
static void gfxstub_signoff(t_gfxstub *x)
{
    gfxstub_offlist(x);
    pd_free(&x->x_pd);
}

static t_buffer *gfxstub_binbuf;

    /* a series of "data" messages rebuilds a scalar */
static void gfxstub_data(t_gfxstub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!gfxstub_binbuf)
        gfxstub_binbuf = buffer_new();
    buffer_append(gfxstub_binbuf, argc, argv);
    buffer_appendSemicolon(gfxstub_binbuf);
}
    /* the "end" message terminates rebuilding the scalar */
static void gfxstub_end(t_gfxstub *x)
{
    canvas_dataproperties((t_glist *)x->x_owner,
        (t_scalar *)x->x_key, gfxstub_binbuf);
    buffer_free(gfxstub_binbuf);
    gfxstub_binbuf = 0;
}

    /* anything else is a message from the dialog window to the owner;
    just forward it. */
static void gfxstub_anything(t_gfxstub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner)
        pd_message(x->x_owner, s, argc, argv);
}

static void gfxstub_free(t_gfxstub *x)
{
    pd_unbind(&x->x_pd, x->x_sym);
}

void gfxstub_setup(void)
{
    gfxstub_class = class_new(gensym("gfxstub"), 0, (t_method)gfxstub_free,
        sizeof(t_gfxstub), CLASS_PURE, 0);
    class_addAnything(gfxstub_class, gfxstub_anything);
    class_addMethod(gfxstub_class, (t_method)gfxstub_signoff,
        gensym("signoff"), 0);
    class_addMethod(gfxstub_class, (t_method)gfxstub_data,
        gensym("data"), A_GIMME, 0);
    class_addMethod(gfxstub_class, (t_method)gfxstub_end,
        gensym("end"), 0);
    class_addMethod(gfxstub_class, (t_method)gfxstub_cancel,
        gensym("cancel"), 0);
}
