
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

typedef struct _qlist {
    t_textbuffer    ql_textbuffer;                  /* Must be the first. */
    int             ql_indexOfMessage;
    int             ql_waitCount;
    int             ql_flagRewound;
    int             ql_flagReentrant;
    t_symbol        *ql_target;
    t_outlet        *ql_outletLeft;
    t_outlet        *ql_outletRight;
    t_clock         *ql_clock;
    } t_qlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        textbuffer_init             (t_textbuffer *x);
void        textbuffer_free             (t_textbuffer *x);
void        textbuffer_click            (t_textbuffer *x,
                                            t_float a,
                                            t_float b,
                                            t_float shift,
                                            t_float ctrl,
                                            t_float alt);

void        textbuffer_close            (t_textbuffer *x);
void        textbuffer_update           (t_textbuffer *x);
void        textbuffer_read             (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);
void        textbuffer_write            (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);
void        textbuffer_addLine          (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);

t_glist     *textbuffer_getView         (t_textbuffer *x);
t_buffer    *textbuffer_getBuffer       (t_textbuffer *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     textclient_init             (t_textclient *x, int *argc, t_atom **argv);
void        textclient_free             (t_textclient *x);
void        textclient_update           (t_textclient *x);

t_glist     *textclient_fetchView       (t_textclient *x);
t_buffer    *textclient_fetchBuffer     (t_textclient *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        *textget_new                (t_symbol *s, int argc, t_atom *argv);
void        *textset_new                (t_symbol *s, int argc, t_atom *argv);
void        *textsize_new               (t_symbol *s, int argc, t_atom *argv);
void        *textfromlist_new           (t_symbol *s, int argc, t_atom *argv);
void        *texttolist_new             (t_symbol *s, int argc, t_atom *argv);
void        *textsearch_new             (t_symbol *s, int argc, t_atom *argv);
void        *textsequence_new           (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        qlist_rewind                (t_qlist *x);
void        qlist_clear                 (t_qlist *x);
void        qlist_set                   (t_qlist *x, t_symbol *s, int argc, t_atom *argv);
void        qlist_add                   (t_qlist *x, t_symbol *s, int argc, t_atom *argv);
void        qlist_append                (t_qlist *x, t_symbol *s, int argc, t_atom *argv);
void        qlist_read                  (t_qlist *x, t_symbol *name);
void        qlist_write                 (t_qlist *x, t_symbol *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define TEXTCLIENT_ASPOINTER(x)         ((x)->tc_templateIdentifier)
#define TEXTCLIENT_GETPOINTER(x)        &((x)->tc_gpointer)
#define TEXTCLIENT_GETNAME(x)           &((x)->tc_name)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _array_client {
    t_object        tc_obj;
    t_symbol        *tc_sym;
    t_gpointer      tc_gp;
    t_symbol        *tc_struct;
    t_symbol        *tc_field;
    t_glist         *tc_canvas;
    } t_array_client;

typedef struct _array_rangeop   /* any operation meaningful on a subrange */
{
    t_array_client x_tc;
    t_float x_onset;
    t_float x_n;
    t_symbol *x_elemfield;
    t_symbol *x_elemtemplate;   /* unused - perhaps should at least check it */
} t_array_rangeop;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_array     *array_client_getbuf        (t_array_client *x, t_glist **glist);
void        array_client_senditup       (t_array_client *x);
void        array_client_free           (t_array_client *x);

void *array_rangeop_new(t_class *class,
    t_symbol *s, int *argcp, t_atom **argvp,
    int onsetin, int nin, int warnextra);
    
int array_rangeop_getrange(t_array_rangeop *x,
    char **firstitemp, int *nitemp, int *stridep, int *arrayonsetp);

void *array_min_new(t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_control_h_
