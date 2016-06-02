
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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

#define GUISTUB_STRING     4096

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

static t_class      *guistub_class;             /* Shared. */
static t_class      *guiconnect_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_buffer     *guistub_buffer;            /* Shared. */
static t_guistub    *guistub_list;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guiconnect_task (t_guiconnect *x)
{
    pd_free (cast_pd (x));
}

void guiconnect_release (t_guiconnect *x, double timeOut)
{
    PD_ASSERT (timeOut >= 0.0);
    
    if (!x->x_bound) { pd_free (cast_pd (x)); }
    else {
        x->x_owner = NULL;
        x->x_clock = clock_new (x, (t_method)guiconnect_task);
        clock_delay (x->x_clock, PD_MAX (0.0, timeOut));
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
    
    c = class_new (sym_guiconnect,
            NULL,
            (t_method)guiconnect_free,
            sizeof (t_guiconnect), 
            CLASS_ABSTRACT,
            A_NULL);
        
    class_addAnything (c, guiconnect_anything);
    
    class_addMethod (c, (t_method)guiconnect_signoff, sym__signoff, A_NULL);
        
    guiconnect_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guistub_removeFromList (t_guistub *x)
{
    t_guistub *y1 = NULL;
    t_guistub *y2 = NULL;
    
    if (guistub_list == x) { guistub_list = x->x_next; }
    else {
        for (y1 = guistub_list; y2 = y1->x_next; y1 = y2) {
            if (y2 == x) { 
                y1->x_next = y2->x_next; break; 
            }
        }
    }
}

void guistub_destroyWithKey (void *key)
{
    t_guistub *y = NULL;
    
    for (y = guistub_list; y; y = y->x_next) {
        if (y->x_key == key) {
            sys_vGui ("destroy .guistub%lx\n", (t_int)y);
            y->x_owner = NULL;
            guistub_removeFromList (y);
            pd_free (cast_pd (y));
            break;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guistub_anything (t_guistub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner) { pd_message (x->x_owner, s, argc, argv); }
}

static void guistub_data (t_guistub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!guistub_buffer) { guistub_buffer = buffer_new(); }
    buffer_append (guistub_buffer, argc, argv);
    buffer_appendSemicolon (guistub_buffer);
}

static void guistub_end (t_guistub *x)
{
    canvas_dataproperties (cast_glist (x->x_owner), cast_scalar (x->x_key), guistub_buffer);
    buffer_free (guistub_buffer);
    guistub_buffer = NULL;
}

static void guistub_signoff (t_guistub *x)
{
    guistub_removeFromList (x);
    pd_free (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://stackoverflow.com/questions/1860159/how-to-escape-the-sign-in-cs-printf > */

void guistub_new (t_pd *owner, void *key, const char *cmd)
{
    t_symbol *s  = NULL;
    t_guistub *x = NULL;
    char name[PD_STRING] = { 0 };
    
    PD_ASSERT (key != NULL);
    
    guistub_destroyWithKey (key);                   /* Destroy already allocated stub with an equal key. */
    
    x = (t_guistub *)pd_new (guistub_class);
    string_sprintf (name, PD_STRING, ".guistub%lx", (t_int)x);
    s = gensym (name);
    
    x->x_owner  = owner;
    x->x_bound  = s;
    x->x_key    = key;
    x->x_next   = guistub_list;

    guistub_list = x;
    
    pd_bind (cast_pd (x), s);
        
    {
    //
    char *afterFirstSubstitution = strchr (cmd, '%') + 2;

    if (afterFirstSubstitution == NULL) { PD_BUG; }
    else {
        t_error err = PD_ERROR_NONE;
        char t[PD_STRING] = { 0 };
        char m[GUISTUB_STRING] = { 0 };
            
        err |= string_append (t, PD_STRING, cmd, (int)(afterFirstSubstitution - cmd));
        err |= string_sprintf (m, GUISTUB_STRING, t, s->s_name);
        err |= string_add (m, GUISTUB_STRING, afterFirstSubstitution);

        PD_ASSERT (!err);
        
        sys_gui (m); 
    }
    //
    }
}

static void guistub_free (t_guistub *x)
{
    pd_unbind (cast_pd (x), x->x_bound);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void guistub_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_guistub,
            NULL, 
            (t_method)guistub_free,
            sizeof (t_guistub),
            CLASS_ABSTRACT, 
            A_NULL);
        
    class_addAnything (c, guistub_anything);

    class_addMethod (c, (t_method)guistub_data,     sym__data,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)guistub_end,      sym__end,       A_NULL);
    class_addMethod (c, (t_method)guistub_signoff,  sym__signoff,   A_NULL);
    
    guistub_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
