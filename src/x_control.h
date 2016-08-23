
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

typedef struct _text_client
{
    t_object tc_obj;
    t_symbol *tc_sym;
    t_gpointer tc_gp;
    t_symbol *tc_struct;
    t_symbol *tc_field;
} t_text_client;

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

void text_client_argparse (t_text_client *x, int *argcp, t_atom **argvp, char *name);
t_buffer *text_client_getbuf (t_text_client *x);
void text_client_senditup (t_text_client *x);
void text_client_free (t_text_client *x);

int text_nthline(int n, t_atom *vec, int line, int *startp, int *endp);
void *text_sequence_new(t_symbol *s, int argc, t_atom *argv);
void *text_search_new(t_symbol *s, int argc, t_atom *argv);
void *text_tolist_new(t_symbol *s, int argc, t_atom *argv);
void *text_fromlist_new(t_symbol *s, int argc, t_atom *argv);
void *text_size_new(t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_control_h_
