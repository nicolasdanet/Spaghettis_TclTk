
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_utils_h_
#define __m_utils_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        utils_anythingToList                        (t_pd *x, 
                                                            t_listmethod fn,
                                                            t_symbol *s,
                                                            int argc,
                                                            t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol    *utils_gensymWithAtoms                      (int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     string_copy                                 (char *dest, size_t size, const char *src);
t_error     string_add                                  (char *dest, size_t size, const char *src);
t_error     string_append                               (char *dest, size_t size, const char *src, int n);
t_error     string_sprintf                              (char *dest, size_t size, const char *format, ...);
t_error     string_addSprintf                           (char *dest, size_t size, const char *format, ...);
t_error     string_addAtom                              (char *dest, size_t size, t_atom *a);

int         string_startWith                            (const char *s, const char *isStartWith);
int         string_containsCharacterAtStart             (const char *s, const char *isContained);

int         string_indexOfFirstOccurrenceUntil          (char *s, const char *c, size_t n);
int         string_indexOfFirstOccurrenceFrom           (char *s, const char *c, size_t n);

void        string_getNumberOfColumnsAndLines           (char *s, int *numberOfColumns, int *numberOfLines);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol    *utils_getFirstAtomOfObjectAsSymbol         (t_object *x);
t_symbol    *utils_getFirstAtomOfBufferAsSymbol         (t_buffer *x);

t_symbol    *utils_decode                               (t_symbol *s);
t_symbol    *utils_dash                                 (void);
t_symbol    *utils_empty                                (void); 
t_symbol    *utils_substituteIfEmpty                    (t_symbol *s, int asDash);

t_symbol    *utils_makeBindSymbol                       (t_symbol *s);
t_symbol    *utils_makeTemplateIdentifier               (t_symbol *s);
t_symbol    *utils_stripBindSymbol                      (t_symbol *s);
t_symbol    *utils_stripTemplateIdentifier              (t_symbol *s);

t_symbol    *utils_getDefaultBindName                   (t_class *c, t_symbol *prefix);

int         utils_isNameAllowedForWindow                (t_symbol *s);

int         utils_isTokenEnd                            (char c);
int         utils_isTokenEscape                         (char c);
int         utils_isTokenWhitespace                     (char c);
int         utils_isAlphanumericOrUnderscore            (char c);
t_unique    utils_unique                                (void);
t_error     utils_version                               (char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         math_compareFloat                           (t_float a, t_float b);
t_float     math_euclideanDistance                      (t_float x1, t_float y1, t_float x2, t_float y2);

t_float     math_midiToFrequency                        (t_float f);
t_float     math_frequencyToMidi                        (t_float f);
t_float     math_rootMeanSquareToDecibel                (t_float f);
t_float     math_decibelToRootMeanSquare                (t_float f);
t_float     math_powerToDecibel                         (t_float f);
t_float     math_decibelToPower                         (t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        rectangle_setEverything                     (int *a, int *b, int *c, int *d);
void        rectangle_setNothing                        (int *a, int *b, int *c, int *d);
void        rectangle_setNowhere                        (int *a, int *b, int *c, int *d);

int         rectangle_isEverything                      (int a, int b, int c, int d);
int         rectangle_isNothing                         (int a, int b, int c, int d);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_utils_h_
