
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

extern t_class *textdefine_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void textbuffer_init (t_textbuffer *x)
{
    x->tb_buffer = buffer_new();
    x->tb_owner  = canvas_getCurrent();
}

void textbuffer_free (t_textbuffer *x)
{
    t_pd *t = NULL;
    
    buffer_free (x->tb_buffer);
    
    if (x->tb_guiconnect) {
        sys_vGui ("destroy .x%lx\n", x);
        guiconnect_release (x->tb_guiconnect, 1000.0);
    }

    while (t = pd_findByClass (sym___hash__A, textdefine_class)) { pd_unbind (t, sym___hash__A); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textbuffer_open (t_textbuffer *x)
{
    if (x->tb_guiconnect) {
        sys_vGui ("wm deiconify .x%lx\n", x);
        sys_vGui ("raise .x%lx\n", x);
        sys_vGui ("focus .x%lx.text\n", x);
        
    } else {
        char t[PD_STRING] = { 0 };
        t_error err = string_sprintf (t, PD_STRING, ".x%lx", x);
        PD_ASSERT (!err);
        sys_vGui ("::ui_text::show .x%lx\n", x);
        x->tb_guiconnect = guiconnect_new (cast_pd (x), gensym (t));
        textbuffer_send (x);
    }
}

void textbuffer_close (t_textbuffer *x)
{
    sys_vGui ("::ui_text::release .x%lx\n", x);
    
    if (x->tb_guiconnect) {
        guiconnect_release (x->tb_guiconnect, 1000.0); 
        x->tb_guiconnect = NULL;
    }    
}

void textbuffer_send (t_textbuffer *x)
{
    int i, ntxt;
    char *txt;
    if (!x->tb_guiconnect)
        return;
    buffer_toStringUnzeroed(x->tb_buffer, &txt, &ntxt);
    sys_vGui("::ui_text::clear .x%lx\n", x);
    for (i = 0; i < ntxt; )
    {
        char *j = strchr(txt+i, '\n');
        if (!j) j = txt + ntxt;
        sys_vGui("::ui_text::append .x%lx {%.*s\n}\n",
            x, j-txt-i, txt+i);
        i = (j-txt)+1;
    }
    sys_vGui("::ui_text::dirty .x%lx 0\n", x);
    PD_MEMORY_FREE(txt);
}

void textbuf_addline(t_textbuffer *b, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *z = buffer_new();
    buffer_deserialize(z, argc, argv);
    buffer_append(b->tb_buffer, buffer_size(z), buffer_atoms(z));
    buffer_free(z);
    textbuffer_send(b);
}

void textbuf_read(t_textbuffer *x, t_symbol *s, int argc, t_atom *argv)
{
    int cr = 0;
    t_symbol *filename;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-c"))
            cr = 1;
        else
        {
            post_error ("text read: unknown flag ...");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        filename = argv->a_w.w_symbol;
        argc--; argv++;
    }
    else
    {
        post_error ("text read: no file name given");
        return;
    }
    if (argc)
    {
        post("warning: text define ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (buffer_read(x->tb_buffer, filename->s_name, x->tb_owner))
            post_error ("%s: read failed", filename->s_name);
    textbuffer_send(x);
}

void textbuf_write(t_textbuffer *x, t_symbol *s, int argc, t_atom *argv)
{
    int cr = 0;
    t_symbol *filename;
    char buf[PD_STRING];
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-c"))
            cr = 1;
        else
        {
            post_error ("text write: unknown flag ...");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        filename = argv->a_w.w_symbol;
        argc--; argv++;
    }
    else
    {
        post_error ("text write: no file name given");
        return;
    }
    if (argc)
    {
        post("warning: text define ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    canvas_makeFilePath(x->tb_owner, filename->s_name,
        buf, PD_STRING);
    if (buffer_write(x->tb_buffer, buf, ""))
            post_error ("%s: write failed", filename->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
