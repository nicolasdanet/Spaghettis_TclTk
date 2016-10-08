
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

void textbuffer_click (t_textbuffer *x, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    if (x->tb_guiconnect) {
        sys_vGui ("wm deiconify .x%lx\n", x);
        sys_vGui ("raise .x%lx\n", x);
        sys_vGui ("focus .x%lx.text\n", x);
        
    } else {
        sys_vGui ("::ui_text::show .x%lx\n", x);
        x->tb_guiconnect = guiconnect_new (cast_pd (x));
        textbuffer_update (x);
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

void textbuffer_update (t_textbuffer *x)
{
    if (x->tb_guiconnect) {
    //
    int size;
    char *text = NULL;
    int i = 0;
        
    buffer_toStringUnzeroed (x->tb_buffer, &text, &size);
    
    sys_vGui ("::ui_text::clear .x%lx\n", x);
    
    while (i < size) {  /* Send it line by line. */

        char *start   = text + i;
        char *newline = strchr (start, '\n');
        
        if (!newline) { newline = text + size; }
        
        /* < http://stackoverflow.com/a/13289324 > */
        
        sys_vGui ("::ui_text::append .x%lx {%.*s\n}\n", x, (int)(newline - start), start);  
        
        i = (newline - text) + 1;
    }
    
    sys_vGui ("::ui_text::dirty .x%lx 0\n", x);
    
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
    t_error err = canvas_makeFilePath (x->tb_owner, name->s_name, t, PD_STRING);
    if (err || buffer_write (x->tb_buffer, t, "")) { error_failsToWrite (name); }
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
