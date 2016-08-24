
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

typedef struct _textbuffer {
    t_object        tb_obj;                         /* Must be the first. */
    t_buffer        *tb_buffer;
    t_glist         *tb_owner;
    t_guiconnect    *tb_guiconnect;
    } t_textbuffer;

typedef struct _textclient {
    t_object        tc_obj;                         /* Must be the first. */
    t_gpointer      tc_gpointer;
    t_symbol        *tc_name;
    t_symbol        *tc_templateIdentifier;
    t_symbol        *tc_fieldName;
    } t_textclient;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        textbuffer_init             (t_textbuffer *x);
void        textbuffer_free             (t_textbuffer *x);
void        textbuffer_open             (t_textbuffer *x);
void        textbuffer_close            (t_textbuffer *x);
void        textbuffer_send             (t_textbuffer *x);
void        textbuffer_addLine          (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);
void        textbuffer_read             (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);
void        textbuffer_write            (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        textclient_init             (t_textclient *x, int *argc, t_atom **argv, char *type);
void        textclient_free             (t_textclient *x);
void        textclient_send             (t_textclient *x);

t_buffer    *textclient_fetchBuffer     (t_textclient *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         text_nthline                (int n, t_atom *vec, int line, int *startp, int *endp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        *text_get_new               (t_symbol *s, int argc, t_atom *argv);
void        *text_set_new               (t_symbol *s, int argc, t_atom *argv);
void        *text_size_new              (t_symbol *s, int argc, t_atom *argv);
void        *text_fromlist_new          (t_symbol *s, int argc, t_atom *argv);
void        *text_tolist_new            (t_symbol *s, int argc, t_atom *argv);
void        *text_search_new            (t_symbol *s, int argc, t_atom *argv);
void        *text_sequence_new          (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_control_h_
