
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_gpointer_h_
#define __g_gpointer_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GPOINTER_INIT(x)    { \
                                (x)->gp_un.gp_scalar = NULL; \
                                (x)->gp_refer = NULL; \
                                (x)->gp_uniqueIdentifier = 0; \
                            }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        gpointer_init                       (t_gpointer *gp);
void        gpointer_setAsScalar                (t_gpointer *gp, t_glist *owner, t_scalar *scalar);
void        gpointer_setAsWord                  (t_gpointer *gp, t_array *owner, t_word *w);

t_scalar    *gpointer_getScalar                 (t_gpointer *gp);
t_word      *gpointer_getWord                   (t_gpointer *gp);
t_glist     *gpointer_getParentGlist            (t_gpointer *gp);
t_array     *gpointer_getParentArray            (t_gpointer *gp);

void        gpointer_setByCopy                  (t_gpointer *gp, t_gpointer *toSet);
void        gpointer_unset                      (t_gpointer *gp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_gpointer *gpointer_getEmpty                   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Functions below callable on both types. */

int         gpointer_isSet                      (t_gpointer *gp);
int         gpointer_isNull                     (t_gpointer *gp);
int         gpointer_isValid                    (t_gpointer *gp);
int         gpointer_isValidNullAllowed         (t_gpointer *gp);
int         gpointer_isScalar                   (t_gpointer *gp);
int         gpointer_isWord                     (t_gpointer *gp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist     *gpointer_getView                   (t_gpointer *gp);
t_word      *gpointer_getElement                (t_gpointer *gp);
t_symbol    *gpointer_getTemplateIdentifier     (t_gpointer *gp);
t_template  *gpointer_getTemplate               (t_gpointer *gp);

int         gpointer_isInstanceOf               (t_gpointer *gp, t_symbol *templateIdentifier);
int         gpointer_isValidInstanceOf          (t_gpointer *gp, t_symbol *templateIdentifier);
void        gpointer_redraw                     (t_gpointer *gp);
void        gpointer_setVisibility              (t_gpointer *gp, int isVisible);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     gpointer_fieldToString              (t_gpointer *gp, t_symbol *field, char *dest, int size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         gpointer_hasField                   (t_gpointer *gp, t_symbol *field);
int         gpointer_fieldIsFloat               (t_gpointer *gp, t_symbol *field);
int         gpointer_fieldIsSymbol              (t_gpointer *gp, t_symbol *field);
int         gpointer_fieldIsText                (t_gpointer *gp, t_symbol *field);
int         gpointer_fieldIsArray               (t_gpointer *gp, t_symbol *field);
int         gpointer_fieldIsArrayAndValid       (t_gpointer *gp, t_symbol *field);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float     gpointer_getFloat                   (t_gpointer *gp, t_symbol *field);
t_symbol    *gpointer_getSymbol                 (t_gpointer *gp, t_symbol *field);
t_buffer    *gpointer_getText                   (t_gpointer *gp, t_symbol *field);
t_array     *gpointer_getArray                  (t_gpointer *gp, t_symbol *field);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        gpointer_setFloat                   (t_gpointer *gp, t_symbol *field, t_float f);
void        gpointer_setSymbol                  (t_gpointer *gp, t_symbol *field, t_symbol *s);

t_float     gpointer_getFloatByDescriptor       (t_gpointer *gp, t_fielddescriptor *fd);
void        gpointer_setFloatByDescriptor       (t_gpointer *gp, t_fielddescriptor *fd, t_float f);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_gpointer_h_
