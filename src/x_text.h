
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __x_text_h_
#define __x_text_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _textbuffer {
    t_object        tb_obj;                         /* Must be the first. */
    t_buffer        *tb_buffer;
    t_glist         *tb_owner;
    t_proxy         *tb_proxy;
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
    t_pd            *ql_target;
    t_outlet        *ql_outletLeft;
    t_outlet        *ql_outletMiddle;
    t_outlet        *ql_outletRight;
    t_clock         *ql_clock;
    } t_qlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TEXTCLIENT_ASPOINTER(x)             ((x)->tc_templateIdentifier)
#define TEXTCLIENT_GETPOINTER(x)            &((x)->tc_gpointer)
#define TEXTCLIENT_GETNAME(x)               &((x)->tc_name)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        textbuffer_init                 (t_textbuffer *x);
void        textbuffer_free                 (t_textbuffer *x);

void        textbuffer_close                (t_textbuffer *x);
void        textbuffer_update               (t_textbuffer *x);
void        textbuffer_click                (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);
void        textbuffer_read                 (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);
void        textbuffer_write                (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);
void        textbuffer_addLine              (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv);

t_glist     *textbuffer_getView             (t_textbuffer *x);
t_buffer    *textbuffer_getBuffer           (t_textbuffer *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     textclient_init                 (t_textclient *x, int *argc, t_atom **argv);
void        textclient_free                 (t_textclient *x);
void        textclient_update               (t_textclient *x);

t_glist     *textclient_fetchView           (t_textclient *x);
t_buffer    *textclient_fetchBuffer         (t_textclient *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        *textget_new                    (t_symbol *s, int argc, t_atom *argv);
void        *textset_new                    (t_symbol *s, int argc, t_atom *argv);
void        *textinsert_new                 (t_symbol *s, int argc, t_atom *argv);
void        *textdelete_new                 (t_symbol *s, int argc, t_atom *argv);
void        *textsize_new                   (t_symbol *s, int argc, t_atom *argv);
void        *textfromlist_new               (t_symbol *s, int argc, t_atom *argv);
void        *texttolist_new                 (t_symbol *s, int argc, t_atom *argv);
void        *textsearch_new                 (t_symbol *s, int argc, t_atom *argv);
void        *textsequence_new               (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        qlist_rewind                    (t_qlist *x);
void        qlist_clear                     (t_qlist *x);
void        qlist_set                       (t_qlist *x, t_symbol *s, int argc, t_atom *argv);
void        qlist_add                       (t_qlist *x, t_symbol *s, int argc, t_atom *argv);
void        qlist_append                    (t_qlist *x, t_symbol *s, int argc, t_atom *argv);
void        qlist_read                      (t_qlist *x, t_symbol *name);
void        qlist_write                     (t_qlist *x, t_symbol *name);
void        qlist_modified                  (t_qlist *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_text_h_
