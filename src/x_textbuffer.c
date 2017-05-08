
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void textbuffer_init (t_textbuffer *x)
{
    x->tb_buffer = buffer_new();
    x->tb_owner  = instance_contextGetCurrent();
}

void textbuffer_free (t_textbuffer *x)
{
    buffer_free (x->tb_buffer);
    
    if (x->tb_proxy) {
        sys_vGui ("destroy %s\n", proxy_getTagAsString (x->tb_proxy));
        proxy_release (x->tb_proxy);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *textbuffer_getView (t_textbuffer *x)
{
    return x->tb_owner;
}

t_buffer *textbuffer_getBuffer (t_textbuffer *x)
{
    return x->tb_buffer;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textbuffer_click (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->tb_proxy) {
    
        sys_vGui ("wm deiconify %s\n", proxy_getTagAsString (x->tb_proxy));
        sys_vGui ("raise %s\n", proxy_getTagAsString (x->tb_proxy));
        sys_vGui ("focus %s.text\n", proxy_getTagAsString (x->tb_proxy));
        
    } else {
    
        x->tb_proxy = proxy_new (cast_pd (x));
        sys_vGui ("::ui_text::show %s\n", proxy_getTagAsString (x->tb_proxy));
        textbuffer_update (x);
    }
}

void textbuffer_close (t_textbuffer *x)
{
    if (x->tb_proxy) {
        sys_vGui ("::ui_text::release %s\n", proxy_getTagAsString (x->tb_proxy));
        proxy_release (x->tb_proxy); 
        x->tb_proxy = NULL;
    }    
}

void textbuffer_update (t_textbuffer *x)
{
    if (x->tb_proxy) {
    //
    int size;
    char *text = NULL;
    char *tag  = proxy_getTagAsString (x->tb_proxy);
    int i = 0;
    
    buffer_toStringUnzeroed (x->tb_buffer, &text, &size);
    
    sys_vGui ("::ui_text::clear %s\n", tag);
    
    while (i < size) {                              /* Send it line by line. */

        char *start   = text + i;
        char *newline = strchr (start, '\n');
        
        if (!newline) { newline = text + size; }
        
        /* < http://stackoverflow.com/a/13289324 > */
        
        sys_vGui ("::ui_text::append %s {%.*s\n}\n", tag, (int)(newline - start), start);  // --
        
        i = (int)(newline - text) + 1;
    }
    
    sys_vGui ("::ui_text::dirty %s 0\n", tag);
    
    PD_MEMORY_FREE (text);
    //
    }
}

void textbuffer_read (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && IS_SYMBOL (argv)) {
    //
    t_symbol *name = GET_SYMBOL (argv);
    if (buffer_read (x->tb_buffer, name, x->tb_owner)) { error_failsToRead (name); }
    textbuffer_update (x);
    //
    }
}

void textbuffer_write (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && IS_SYMBOL (argv)) {
    //
    char t[PD_STRING] = { 0 };
    t_symbol *name = GET_SYMBOL (argv);
    char *directory = environment_getDirectoryAsString (glist_getEnvironment (x->tb_owner));
    t_error err = path_withDirectoryAndName (t, PD_STRING, directory, name->s_name);
    if (err || buffer_write (x->tb_buffer, gensym (t), &s_)) { error_failsToWrite (name); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Used only by the GUI to set contents of the buffer. */

void textbuffer_addLine (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *t = buffer_new();
    buffer_deserialize (t, argc, argv);
    buffer_appendBuffer (x->tb_buffer, t);
    buffer_free (t);
    textbuffer_update (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
