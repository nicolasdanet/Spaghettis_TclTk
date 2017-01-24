
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class      *guistub_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _guistub {
    t_pd                x_pd;                   /* Must be the first. */
    t_pd                *x_owner;
    void                *x_key;
    t_symbol            *x_bound;
    struct _guistub     *x_next;
    } t_guistub;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_guistub    *guistub_list;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guistub_removeFromList (t_guistub *x)
{
    t_guistub *yA = NULL;
    t_guistub *yB = NULL;
    
    if (guistub_list == x) { guistub_list = x->x_next; }
    else {
        for ((yA = guistub_list); (yB = yA->x_next); (yA = yB)) {
            if (yB == x) { 
                yA->x_next = yB->x_next; break; 
            }
        }
    }
}

void guistub_destroyWithKey (void *key)
{
    t_guistub *y = NULL;
    
    for (y = guistub_list; y; y = y->x_next) {
        if (y->x_key == key) {
            sys_vGui ("destroy " PD_GUISTUB "%lx\n", y);
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

static void guistub_signoff (t_guistub *x)
{
    guistub_removeFromList (x); pd_free (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* < http://stackoverflow.com/questions/1860159/how-to-escape-the-sign-in-cs-printf > */

t_error guistub_new (t_pd *owner, void *key, const char *cmd)
{
    t_symbol *s  = NULL;
    t_guistub *x = NULL;
    char name[PD_STRING] = { 0 };
    
    PD_ASSERT (key != NULL);
    
    guistub_destroyWithKey (key);                   /* Destroy already allocated stub with an equal key. */
    
    x = (t_guistub *)pd_new (guistub_class);
    string_addSprintf (name, PD_STRING, PD_GUISTUB "%lx", x);
    s = gensym (name);
    
    x->x_owner  = owner;
    x->x_bound  = s;
    x->x_key    = key;
    x->x_next   = guistub_list;

    guistub_list = x;
    
    pd_bind (cast_pd (x), s);
        
    {
    //
    t_error err = PD_ERROR_NONE;
    
    char *afterFirstSubstitution = strchr (cmd, '%') + 2;

    if (afterFirstSubstitution == NULL) { PD_BUG; err = PD_ERROR; }
    else {
        char t[PD_STRING] = { 0 };
        t_heapstring *m = heapstring_new (0);
            
        err |= string_append (t, PD_STRING, cmd, (int)(afterFirstSubstitution - cmd));
        err |= heapstring_addSprintf (m, t, s->s_name);
        err |= heapstring_add (m, afterFirstSubstitution);

        PD_ASSERT (!err);
        
        sys_gui (heapstring_getRaw (m));
        
        heapstring_free (m);
    }
    
    return err;
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
            CLASS_NOBOX, 
            A_NULL);
        
    class_addAnything (c, (t_method)guistub_anything);

    class_addMethod (c, (t_method)guistub_signoff, sym__signoff, A_NULL);
    
    guistub_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
