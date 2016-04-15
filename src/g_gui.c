
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _guiconnect
{
    t_object        x_obj;
    t_pd            *x_owner;
    t_symbol        *x_bound;
    t_clock         *x_clock;
};

typedef struct _guistub
{
    t_pd            x_pd;
    t_pd            *x_owner;
    void            *x_key;
    t_symbol        *x_bound;
    struct _guistub *x_next;
} t_guistub;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class      *guistub_class;
static t_class      *guiconnect_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_guistub    *guistub_list;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guiconnect_task (t_guiconnect *x)
{
    pd_free (cast_pd (x));
}

void guiconnect_release (t_guiconnect *x, double timeOut)
{
    if (!x->x_bound) { pd_free (cast_pd (x)); }
    else {
        x->x_owner = NULL;
        
        if (timeOut > 0) {
            x->x_clock = clock_new (x, (t_method)guiconnect_task);
            clock_delay (x->x_clock, timeOut);
        }
    }    
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guiconnect_anything (t_guiconnect *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner) { pd_message (x->x_owner, s, argc, argv); }
}

static void guiconnect_signoff (t_guiconnect *x)
{
    if (!x->x_owner) { pd_free (cast_pd (x)); }
    else {
        pd_unbind (cast_pd (x), x->x_bound); x->x_bound = NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_guiconnect *guiconnect_new (t_pd *owner, t_symbol *bindTo)
{
    t_guiconnect *x = (t_guiconnect *)pd_new (guiconnect_class);
    
    PD_ASSERT (owner);
    PD_ASSERT (bindTo);
    
    x->x_owner = owner;
    x->x_bound = bindTo;
    
    pd_bind (cast_pd (x), x->x_bound);
    
    return x;
}

static void guiconnect_free (t_guiconnect *x)
{
    if (x->x_bound) { pd_unbind (cast_pd (x), x->x_bound); }
    if (x->x_clock) { clock_free (x->x_clock); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void guiconnect_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("guiconnect"),
        NULL,
        (t_method)guiconnect_free,
        sizeof (t_guiconnect), 
        CLASS_PURE,
        A_NULL);
        
    class_addAnything (c, guiconnect_anything);
    
    class_addMethod (c, (t_method)guiconnect_signoff, gensym ("signoff"), A_NULL);
        
    guiconnect_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guistub_offlist(t_guistub *x)
{
    t_guistub *y1, *y2;
    if (guistub_list == x)
        guistub_list = x->x_next;
    else for (y1 = guistub_list; y2 = y1->x_next; y1 = y2)
        if (y2 == x) 
    {
        y1->x_next = y2->x_next;
        break;
    }
}

    /* if the owner disappears, we still may have to stay around until our
    dialog window signs off.  Anyway we can now tell the GUI to destroy the
    window.  */
void guistub_deleteforkey(void *key)
{
    t_guistub *y;
    int didit = 1;
    while (didit)
    {
        didit = 0;
        for (y = guistub_list; y; y = y->x_next)
        {
            if (y->x_key == key)
            {
                sys_vGui("destroy .guistub%lx\n", y);
                y->x_owner = 0;
                guistub_offlist(y);
                didit = 1;
                break;
            }
        }
    }
}

/* --------- pd messages for guistub (these come from the GUI) ---------- */

    /* "cancel" to request that we close the dialog window. */
static void guistub_cancel(t_guistub *x)
{
    guistub_deleteforkey(x->x_key);
}

    /* "signoff" comes from the GUI to say the dialog window closed. */
static void guistub_signoff(t_guistub *x)
{
    guistub_offlist(x);
    pd_free(&x->x_pd);
}

static t_buffer *guistub_binbuf;

    /* a series of "data" messages rebuilds a scalar */
static void guistub_data(t_guistub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!guistub_binbuf)
        guistub_binbuf = buffer_new();
    buffer_append(guistub_binbuf, argc, argv);
    buffer_appendSemicolon(guistub_binbuf);
}
    /* the "end" message terminates rebuilding the scalar */
static void guistub_end(t_guistub *x)
{
    canvas_dataproperties((t_glist *)x->x_owner,
        (t_scalar *)x->x_key, guistub_binbuf);
    buffer_free(guistub_binbuf);
    guistub_binbuf = 0;
}

    /* anything else is a message from the dialog window to the owner;
    just forward it. */
static void guistub_anything(t_guistub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner)
        pd_message(x->x_owner, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void guistub_new (t_pd *owner, void *key, const char *cmd)
{
    char buf[4*PD_STRING];
    char namebuf[80];
    char sprintfbuf[PD_STRING];
    char *afterpercent;
    t_int afterpercentlen;
    t_guistub *x;
    t_symbol *s;
        /* if any exists with matching key, burn it. */
    for (x = guistub_list; x; x = x->x_next)
        if (x->x_key == key)
            guistub_deleteforkey(key);
    if (strlen(cmd) + 50 > 4*PD_STRING)
    {
        PD_BUG;
        return;
    }
    x = (t_guistub *)pd_new(guistub_class);
    sprintf(namebuf, ".guistub%lx", (t_int)x);

    s = gensym(namebuf);
    pd_bind(&x->x_pd, s);
    x->x_owner = owner;
    x->x_bound = s;
    x->x_key = key;
    x->x_next = guistub_list;
    guistub_list = x;
    /* only replace first %s so sprintf() doesn't crash */
    afterpercent = strchr(cmd, '%') + 2;
    afterpercentlen = afterpercent - cmd;
    strncpy(sprintfbuf, cmd, afterpercentlen);
    sprintfbuf[afterpercentlen] = '\0';
    sprintf(buf, sprintfbuf, s->s_name);
    strncat(buf, afterpercent, (4*PD_STRING) - afterpercentlen);
    sys_gui(buf);
}

static void guistub_free(t_guistub *x)
{
    pd_unbind(&x->x_pd, x->x_bound);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void guistub_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("guistub"),
        NULL, 
        (t_method)guistub_free,
        sizeof (t_guistub),
        CLASS_PURE, 
        A_NULL);
        
    class_addAnything (c, guistub_anything);
    
    class_addMethod (c, (t_method)guistub_signoff,  gensym ("signoff"), A_NULL);
    class_addMethod (c, (t_method)guistub_data,     gensym ("data"),    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)guistub_end,      gensym ("end"),     A_NULL);
    class_addMethod (c, (t_method)guistub_cancel,   gensym ("cancel"),  A_NULL);
        
    guistub_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
