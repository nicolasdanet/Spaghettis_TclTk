
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __x_control_h_
#define __x_control_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _textbuf
{
    t_object b_ob;
    t_buffer *b_binbuf;
    t_glist *b_canvas;
    t_guiconnect *b_guiconnect;
} t_textbuf;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void textbuf_init (t_textbuf *x);
void textbuf_senditup (t_textbuf *x);
void textbuf_open (t_textbuf *x);
void textbuf_close (t_textbuf *x);
void textbuf_addline (t_textbuf *b, t_symbol *s, int argc, t_atom *argv);
void textbuf_read (t_textbuf *x, t_symbol *s, int argc, t_atom *argv);
void textbuf_write (t_textbuf *x, t_symbol *s, int argc, t_atom *argv);
void textbuf_free (t_textbuf *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_control_h_
