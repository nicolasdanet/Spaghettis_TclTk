
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
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     *pd_newest;

extern t_pd     pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class         *textdefine_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* text_define object - text buffer, accessible by other accessor objects */

typedef struct _text_define
{
    t_textbuffer x_textbuf;     /* Must be the first. */
    t_outlet *x_out;
    t_symbol *x_bindsym;
    t_scalar *x_scalar;     /* faux scalar (struct text-scalar) to point to */
    t_gpointer x_gp;        /* pointer to it */
    t_glist *x_canvas;     /* owning canvas whose stub we use for x_gp */
    unsigned char x_keep;   /* whether to embed contents in patch on save */
} t_text_define;

#define x_ob x_textbuf.tb_obj
#define x_binbuf x_textbuf.tb_buffer
#define x_canvas x_textbuf.tb_owner

static void *text_define_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_define *x = (t_text_define *)pd_new(textdefine_class);
    t_symbol *asym = sym___hash__A;
    x->x_keep = 0;
    x->x_bindsym = &s_;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-k"))
            x->x_keep = 1;
        else
        {
            post_error ("text define: unknown flag ...");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        pd_bind(&x->x_ob.te_g.g_pd, argv->a_w.w_symbol);
        x->x_bindsym = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text define ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    textbuffer_init (&x->x_textbuf);
        /* set up a scalar and a pointer to it that we can output */
    x->x_scalar = scalar_new(canvas_getCurrent(), sym___TEMPLATE__text);
    buffer_free(x->x_scalar->sc_vector[2].w_buffer);                        /* Encaspulate ASAP. */
    x->x_scalar->sc_vector[2].w_buffer = x->x_binbuf;                       /* Encaspulate ASAP. */
    x->x_out = outlet_new(&x->x_ob, &s_pointer);
    gpointer_init(&x->x_gp);
    x->x_canvas = canvas_getCurrent();
           /* bashily unbind #A -- this would create garbage if #A were
           multiply bound but we believe in this context it's at most
           bound to whichever text_define or array was created most recently */
    asym->s_thing = 0;
        /* and now bind #A to us to receive following messages in the
        saved file or copy buffer */
    pd_bind(&x->x_ob.te_g.g_pd, asym); 
    return (x);
}

static void text_define_clear(t_text_define *x)
{
    buffer_reset(x->x_binbuf);
    textbuffer_update(&x->x_textbuf);
}

/*********  random utility function to find a binbuf in a datum */

t_buffer *pointertobinbuf(t_pd *x, t_gpointer *gp, t_symbol *s,
    const char *fname)
{
    t_symbol *templatesym = gpointer_getTemplateIdentifier(gp), *arraytype;
    t_template *template;
    int onset, type;
    t_buffer *b;
    t_word *vec;
    if (!templatesym)
    {
        post_error ("%s: bad pointer", fname);
        return (0);
    }
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("%s: couldn't find template %s", fname,
            templatesym->s_name);
        return (0);
    }
    /* Remove template_findField ASAP !!! */
    if (!template_findField(template, s, &onset, &type, &arraytype))    
    {
        post_error ("%s: %s.%s: no such field", fname,
            templatesym->s_name, s->s_name);
        return (0);
    }
    if (type != DATA_TEXT)
    {
        post_error ("%s: %s.%s: not a list", fname,
            templatesym->s_name, s->s_name);
        return (0);
    }
    vec = gpointer_getData (gp);
    return (vec[onset].w_buffer);
}


    /* these are unused; they copy text from this object to and from a text
        field in a scalar. */
static void text_define_frompointer(t_text_define *x, t_gpointer *gp,
    t_symbol *s)
{
    t_buffer *b = pointertobinbuf(&x->x_ob.te_g.g_pd,
        gp, s, "text_frompointer");
    if (b)
    {
        buffer_reset(x->x_binbuf);
        buffer_appendBuffer(x->x_binbuf, b);
    } 
}

static void text_define_topointer(t_text_define *x, t_gpointer *gp, t_symbol *s)
{
    t_buffer *b = pointertobinbuf(&x->x_ob.te_g.g_pd, gp, s, "text_topointer");
    if (b)
    {
        buffer_reset(b);
        buffer_appendBuffer (b, x->x_binbuf);
        gpointer_redraw (gp);
    } 
}

    /* bang: output a pointer to a struct containing this text */
void text_define_bang(t_text_define *x)
{
    gpointer_setAsScalar(&x->x_gp, x->x_canvas, x->x_scalar);
    outlet_pointer(x->x_out, &x->x_gp);
}

    /* set from a list */
void text_define_set(t_text_define *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_deserialize(x->x_binbuf, argc, argv);
    textbuffer_update(&x->x_textbuf);
}


static void text_define_save(t_gobj *z, t_buffer *bb)
{
    t_text_define *x = (t_text_define *)z;
    buffer_vAppend(bb, "ssff", sym___hash__X, sym_obj,
        (float)x->x_ob.te_xCoordinate, (float)x->x_ob.te_yCoordinate);
    buffer_serialize(bb, x->x_ob.te_buffer);
    buffer_appendSemicolon(bb);
    if (x->x_keep)
    {
        buffer_vAppend(bb, "ss", sym___hash__A, sym_set);
        buffer_serialize(bb, x->x_binbuf);
        buffer_appendSemicolon(bb);
    }
    object_saveWidth(&x->x_ob, bb);
}

static void text_define_free(t_text_define *x)
{
    textbuffer_free (&x->x_textbuf);
    if (x->x_bindsym != &s_)
        pd_unbind(&x->x_ob.te_g.g_pd, x->x_bindsym);
    gpointer_unset(&x->x_gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* --- overall creator for "text" objects: dispatch to "text define" etc --- */
static void *text_new(t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || argv[0].a_type != A_SYMBOL)
        pd_newest = text_define_new(s, argc, argv);
    else
    {
        char *str = argv[0].a_w.w_symbol->s_name;
        if (!strcmp(str, "d") || !strcmp(str, "define"))
            pd_newest = text_define_new(s, argc-1, argv+1);
        else if (!strcmp(str, "get"))
            pd_newest = text_get_new(s, argc-1, argv+1);
        else if (!strcmp(str, "set"))
            pd_newest = text_set_new(s, argc-1, argv+1);
        else if (!strcmp(str, "size"))
            pd_newest = text_size_new(s, argc-1, argv+1);
        else if (!strcmp(str, "tolist"))
            pd_newest = text_tolist_new(s, argc-1, argv+1);
        else if (!strcmp(str, "fromlist"))
            pd_newest = text_fromlist_new(s, argc-1, argv+1);
        else if (!strcmp(str, "search"))
            pd_newest = text_search_new(s, argc-1, argv+1);
        else if (!strcmp(str, "sequence"))
            pd_newest = text_sequence_new(s, argc-1, argv+1);
        else 
        {
            post_error ("list %s: unknown function", str);
            pd_newest = 0;
        }
    }
    return (pd_newest);
}

static t_pd *text_templatecanvas;
static char text_templatefile[] = "\
canvas 0 0 458 153 10;\n\
#X obj 43 31 struct text float x float y text t;\n\
";

/* create invisible, built-in canvas to supply template containing one text
field named 't'.  I don't know how to make this not break
pre-0.45 patches using templates named 'text'... perhaps this is a minor
enough incompatibility that I'll just get away with it. */

static void text_template_init( void)
{
    t_buffer *b;
    if (text_templatecanvas)
        return;
    b = buffer_new();
    
    canvas_setActiveFileNameAndDirectory (sym__texttemplate, sym___dot__);
    buffer_withStringUnzeroed(b, text_templatefile, strlen (text_templatefile));
    buffer_eval(b, &pd_canvasMaker, 0, 0);
    pd_vMessage(s__X.s_thing, sym__pop, "i", 0);
    
    canvas_setActiveFileNameAndDirectory (&s_, &s_);
    buffer_free(b);  
}

void textdefine_setup (void)
{
    text_template_init();
    textdefine_class = class_new(sym_text__space__define,
        (t_newmethod)text_define_new,
        (t_method)text_define_free, sizeof(t_text_define), 0, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_open,
        sym_click, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_close,
        sym_close, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_add, 
        sym__addline, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)text_define_set,
        sym_set, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)text_define_clear,
        sym_clear, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_write,
        sym_write, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_read,
        sym_read, A_GIMME, 0);
    class_setSaveFunction(textdefine_class, text_define_save);
    class_addBang(textdefine_class, text_define_bang);
    class_setHelpName(textdefine_class, sym_text);

    class_addCreator((t_newmethod)text_new, sym_text, A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
