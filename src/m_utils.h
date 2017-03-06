
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

#define COLOR_IEM_BACKGROUND_DARK       0xcccccc        /* Grey. */
#define COLOR_IEM_BACKGROUND_LIGHT      0xffffff        /* White. */
#define COLOR_IEM_FOREGROUND            0x000000        /* Black. */
#define COLOR_IEM_LABEL                 0x000000        /* Black. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define COLOR_MASKED                    0xdddddd        /* Grey. */
#define COLOR_NORMAL                    0x000000        /* Black. */
#define COLOR_SELECTED                  0x0000ff        /* Blue. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_color     color_checked                               (t_color color);
t_color     color_withRGB                               (int argc, t_atom *argv);
t_color     color_withDigits                            (int c);
t_color     color_withEncodedSymbol                     (t_symbol *s);
t_error     color_toEncodedString                       (char *dest, size_t size, t_color color);

t_symbol    *color_toEncodedSymbol                      (t_color color);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     string_copy                                 (char *dest, size_t size, const char *src);
t_error     string_add                                  (char *dest, size_t size, const char *src);
t_error     string_append                               (char *dest, size_t size, const char *src, int n);
t_error     string_sprintf                              (char *dest, size_t size, const char *format, ...);
t_error     string_addSprintf                           (char *dest, size_t size, const char *format, ...);
t_error     string_addAtom                              (char *dest, size_t size, t_atom *a);
t_error     string_clear                                (char *dest, size_t size);

int         string_startWith                            (const char *s, const char *isStartWith);
int         string_endWith                              (const char *s, const char *isEndWith);
int         string_containsCharacterAtStart             (const char *s, const char *isContained);
int         string_contains                             (const char *s, const char *isContained);
        
int         string_indexOfFirstOccurrenceUntil          (char *s, const char *c, int n);
int         string_indexOfFirstOccurrenceFrom           (char *s, const char *c, int n);
void        string_replaceCharacter                     (char *s, char toBeReplaced, char c);

void        string_getNumberOfColumnsAndLines           (char *s, int *numberOfColumns, int *numberOfLines);

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

int         math_ilog2                                  (int n);

int         math_compareFloat                           (t_float a, t_float b);
t_float     math_euclideanDistance                      (t_float a, t_float b, t_float c, t_float d);

t_float     math_midiToFrequency                        (t_float f);
t_float     math_frequencyToMidi                        (t_float f);
t_float     math_rootMeanSquareToDecibel                (t_float f);
t_float     math_decibelToRootMeanSquare                (t_float f);
t_float     math_powerToDecibel                         (t_float f);
t_float     math_decibelToPower                         (t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_utils_h_
