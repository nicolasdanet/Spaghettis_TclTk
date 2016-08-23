
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

extern t_class *text_define_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void textbuf_init(t_textbuf *x)
{
    x->b_binbuf = buffer_new();
    x->b_canvas = canvas_getCurrent();
}

void textbuf_senditup(t_textbuf *x)
{
    int i, ntxt;
    char *txt;
    if (!x->b_guiconnect)
        return;
    buffer_toStringUnzeroed(x->b_binbuf, &txt, &ntxt);
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

void textbuf_open(t_textbuf *x)
{
    if (x->b_guiconnect)
    {
        sys_vGui("wm deiconify .x%lx\n", x);
        sys_vGui("raise .x%lx\n", x);
        sys_vGui("focus .x%lx.text\n", x);
    }
    else
    {
        char buf[40];
        sys_vGui("::ui_text::show .x%lx\n",
            x /*, 600, 340, "myname", "text",
                 font_getHostFontSize(canvas_getFontSize(x->b_canvas))*/);
        sprintf(buf, ".x%lx", (unsigned long)x);
        x->b_guiconnect = guiconnect_new(&x->b_ob.te_g.g_pd, gensym (buf));
        textbuf_senditup(x);
    }
}

void textbuf_close(t_textbuf *x)
{
    sys_vGui("::ui_text::release .x%lx\n", x);
    if (x->b_guiconnect)
    {
        guiconnect_release(x->b_guiconnect, 1000);
        x->b_guiconnect = 0;
    }    
}

void textbuf_addline(t_textbuf *b, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *z = buffer_new();
    buffer_deserialize(z, argc, argv);
    buffer_append(b->b_binbuf, buffer_size(z), buffer_atoms(z));
    buffer_free(z);
    textbuf_senditup(b);
}

void textbuf_read(t_textbuf *x, t_symbol *s, int argc, t_atom *argv)
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
    if (buffer_read(x->b_binbuf, filename->s_name, x->b_canvas))
            post_error ("%s: read failed", filename->s_name);
    textbuf_senditup(x);
}

void textbuf_write(t_textbuf *x, t_symbol *s, int argc, t_atom *argv)
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
    canvas_makeFilePath(x->b_canvas, filename->s_name,
        buf, PD_STRING);
    if (buffer_write(x->b_binbuf, buf, ""))
            post_error ("%s: write failed", filename->s_name);
}

void textbuf_free(t_textbuf *x)
{
    t_pd *x2;
    buffer_free(x->b_binbuf);
    if (x->b_guiconnect)
    {
        sys_vGui("destroy .x%lx\n", x);
        guiconnect_release(x->b_guiconnect, 1000);
    }
        /* just in case we're still bound to #A from loading... */
    while (x2 = pd_findByClass(sym___hash__A, text_define_class))
        pd_unbind(x2, sym___hash__A);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
