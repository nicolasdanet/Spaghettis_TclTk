
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *message_class;
static t_class *messageresponder_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void messageresponder_bang (t_messageresponder *x)
{
    outlet_bang (x->mr_outlet);
}

static void messageresponder_float (t_messageresponder *x, t_float f)
{
    outlet_float (x->mr_outlet, f);
}

static void messageresponder_symbol (t_messageresponder *x, t_symbol *s)
{
    outlet_symbol (x->mr_outlet, s);
}

static void messageresponder_list (t_messageresponder *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (x->mr_outlet, s, argc, argv);
}

static void messageresponder_anything (t_messageresponder *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->mr_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void message_bang(t_message *x)
{
    buffer_eval(x->m_obj.te_buffer, &x->m_responder.mr_pd, 0, 0);
}

static void message_float(t_message *x, t_float f)
{
    t_atom at;
    SET_FLOAT(&at, f);
    buffer_eval(x->m_obj.te_buffer, &x->m_responder.mr_pd, 1, &at);
}

static void message_symbol(t_message *x, t_symbol *s)
{
    t_atom at;
    SET_SYMBOL(&at, s);
    buffer_eval(x->m_obj.te_buffer, &x->m_responder.mr_pd, 1, &at);
}

static void message_list(t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_eval(x->m_obj.te_buffer, &x->m_responder.mr_pd, argc, argv);
}

static void message_set(t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_reset(x->m_obj.te_buffer);
    buffer_append(x->m_obj.te_buffer, argc, argv);
    glist_retext(x->m_owner, &x->m_obj);
}

static void message_add2(t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append(x->m_obj.te_buffer, argc, argv);
    glist_retext(x->m_owner, &x->m_obj);
}

static void message_add(t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append(x->m_obj.te_buffer, argc, argv);
    buffer_appendSemicolon(x->m_obj.te_buffer);
    glist_retext(x->m_owner, &x->m_obj);
}

static void message_addcomma(t_message *x)
{
    t_atom a;
    SET_COMMA(&a);
    buffer_append(x->m_obj.te_buffer, 1, &a);
    glist_retext(x->m_owner, &x->m_obj);
}

static void message_addsemi(t_message *x)
{
    message_add(x, 0, 0, 0);
}

static void message_adddollar(t_message *x, t_float f)
{
    t_atom a;
    int n = f;
    if (n < 0)
        n = 0;
    SET_DOLLAR(&a, n);
    buffer_append(x->m_obj.te_buffer, 1, &a);
    glist_retext(x->m_owner, &x->m_obj);
}

static void message_adddollsym(t_message *x, t_symbol *s)
{
    t_atom a;
    char buf[PD_STRING];
    buf[0] = '$';
    strncpy(buf+1, s->s_name, PD_STRING-2);
    buf[PD_STRING-1] = 0;
    SET_DOLLARSYMBOL(&a, gensym (buf));
    buffer_append(x->m_obj.te_buffer, 1, &a);
    glist_retext(x->m_owner, &x->m_obj);
}

void message_click(t_message *x, t_float xpos, t_float ypos, t_float shift, t_float ctrl, t_float alt)
{
    message_float(x, 0);
    if (canvas_isMapped(x->m_owner))
    {
        t_boxtext *y = boxtext_fetch(x->m_owner, &x->m_obj);
        sys_vGui(".x%lx.c itemconfigure %sR -width 5\n", 
            canvas_getView(x->m_owner), boxtext_getTag(y));
        clock_delay(x->m_clock, 120);
    }
}

static void message_tick(t_message *x)
{
    if (canvas_isMapped(x->m_owner))
    {
        t_boxtext *y = boxtext_fetch(x->m_owner, &x->m_obj);
        sys_vGui(".x%lx.c itemconfigure %sR -width 1\n",
            canvas_getView(x->m_owner), boxtext_getTag(y));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_makeObject (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_message *x = (t_message *)pd_new (message_class);
    
    x->m_obj.te_buffer          = buffer_new();
    x->m_obj.te_width           = 0;
    x->m_obj.te_type            = TYPE_MESSAGE;
    x->m_responder.mr_pd        = messageresponder_class;
    x->m_responder.mr_outlet    = outlet_new (cast_object (x), &s_anything);
    x->m_owner                  = glist;
    x->m_clock                  = clock_new (x, (t_method)message_tick);
    
    if (argc > 1) {                                                             /* File creation. */
    
        x->m_obj.te_xCoordinate = atom_getFloatAtIndex (0, argc, argv);
        x->m_obj.te_yCoordinate = atom_getFloatAtIndex (1, argc, argv);
        
        if (argc > 2) {
            buffer_deserialize (x->m_obj.te_buffer, argc - 2, argv + 2);
        }
        
        glist_add (glist, cast_gobj (x));
        
    } else if (canvas_isMapped (glist)) {                                       /* Interactive creation. */

        int positionX = 0;
        int positionY = 0;
        
        canvas_getLastMotionCoordinates (glist, &positionX, &positionY);
        canvas_deselectAll (glist);
    
        x->m_obj.te_xCoordinate = positionX;
        x->m_obj.te_yCoordinate = positionY;
        
        glist_add (glist, cast_gobj (x));
        canvas_selectObject (glist, cast_gobj (x));
        gobj_activate (cast_gobj (x), glist, 1);
    }
}

static void message_free (t_message *x)
{
    clock_free (x->m_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_message,
            NULL,
            (t_method)message_free,
            sizeof (t_message),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, message_bang);
    class_addFloat (c, message_float);
    class_addSymbol (c, message_symbol);
    class_addList (c, message_list);
    class_addAnything (c, message_list);

    class_addClick (c, message_click);
        
    class_addMethod (c, (t_method)message_set,          sym_set,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_add,          sym_add,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_add2,         sym_addword,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_addcomma,     sym_addcomma,           A_NULL);
    class_addMethod (c, (t_method)message_addsemi,      sym_addsemicolon,       A_NULL);
    class_addMethod (c, (t_method)message_adddollar,    sym_adddollar,          A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)message_adddollsym,   sym_adddollarsymbol,    A_SYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)message_add2,         sym_add2,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_addsemi,      sym_addsemi,            A_NULL);
    class_addMethod (c, (t_method)message_adddollsym,   sym_adddollsym,         A_SYMBOL, A_NULL);
        
    #endif
    
    message_class = c;
    
    c = class_new (sym_messageresponder,
            NULL,
            NULL,
            0,
            CLASS_PURE,
            A_NULL);
            
    class_addBang (c, messageresponder_bang);
    class_addFloat (c, messageresponder_float);
    class_addSymbol (c, messageresponder_symbol);
    class_addList (c, messageresponder_list);
    class_addAnything (c, messageresponder_anything);
    
    messageresponder_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
