
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *stub_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _stub {
    t_pd            x_pd;                   /* Must be the first. */
    t_pd            *x_owner;
    void            *x_key;
    t_symbol        *x_bound;
    struct _stub    *x_next;
    } t_stub;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_stub *stub_list;                   /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void stub_removeFromList (t_stub *x)
{
    t_stub *t1 = NULL;
    t_stub *t2 = NULL;
    
    if (stub_list == x) { stub_list = x->x_next; }
    else {
        for ((t1 = stub_list); (t2 = t1->x_next); (t1 = t2)) {
            if (t2 == x) { 
                t1->x_next = t2->x_next; break; 
            }
        }
    }
}

void stub_destroyWithKey (void *key)
{
    t_stub *t = NULL;
    
    for (t = stub_list; t; t = t->x_next) {
        if (t->x_key == key) {
            sys_vGui ("destroy " PD_GUISTUB "%lx\n", t);
            t->x_owner = NULL;
            stub_removeFromList (t);
            pd_free (cast_pd (t));
            break;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void stub_anything (t_stub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner) { pd_message (x->x_owner, s, argc, argv); }
}

static void stub_signoff (t_stub *x)
{
    stub_removeFromList (x); pd_free (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* < http://stackoverflow.com/questions/1860159/how-to-escape-the-sign-in-cs-printf > */

t_error stub_new (t_pd *owner, void *key, const char *cmd)
{
    t_symbol *s  = NULL;
    t_stub *x = NULL;
    char name[PD_STRING] = { 0 };
    
    PD_ASSERT (key != NULL);
    
    stub_destroyWithKey (key);                   /* Destroy already allocated stub with an equal key. */
    
    x = (t_stub *)pd_new (stub_class);
    
    string_addSprintf (name, PD_STRING, PD_GUISTUB "%lx", x);
    
    s = gensym (name);
    
    x->x_owner  = owner;
    x->x_bound  = s;
    x->x_key    = key;
    x->x_next   = stub_list;

    stub_list = x;
    
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

static void stub_free (t_stub *x)
{
    pd_unbind (cast_pd (x), x->x_bound);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void stub_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_stub,
            NULL, 
            (t_method)stub_free,
            sizeof (t_stub),
            CLASS_NOBOX, 
            A_NULL);
        
    class_addAnything (c, (t_method)stub_anything);

    class_addMethod (c, (t_method)stub_signoff, sym__signoff, A_NULL);
    
    stub_class = c;
}

void stub_destroy (void)
{
    CLASS_FREE (stub_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
