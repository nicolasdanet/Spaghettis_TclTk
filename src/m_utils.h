
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_utils_h_
#define __m_utils_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     utils_version                   (char *dest, size_t size);
t_unique    utils_unique                    (void);

void        utils_anythingToList            (t_pd *x, t_listmethod fn, t_symbol *s, int argc, t_atom *argv);

t_symbol    *utils_getUnusedBindName        (t_class *c, t_symbol *prefix);

t_symbol    *utils_getFirstAtomOfObject     (t_object *x);
t_symbol    *utils_getFirstAtomOfBuffer     (t_buffer *x);

int         utils_isNameAllowedForWindow    (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol    *symbol_withAtoms               (int argc, t_atom *argv);

t_symbol    *symbol_nil                     (void);
t_symbol    *symbol_dash                    (void);
t_symbol    *symbol_emptyAsNil              (t_symbol *s);
t_symbol    *symbol_emptyAsDash             (t_symbol *s);

int         symbol_isNil                    (t_symbol *s);
int         symbol_isNilOrDash              (t_symbol *s);

t_symbol    *symbol_dollarToHash            (t_symbol *s);
t_symbol    *symbol_hashToDollar            (t_symbol *s);
t_symbol    *symbol_decode                  (t_symbol *s);
t_symbol    *symbol_removeExtension         (t_symbol *s);

t_symbol    *symbol_makeBind                (t_symbol *s);
t_symbol    *symbol_makeTemplateIdentifier  (t_symbol *s);
t_symbol    *symbol_stripBind               (t_symbol *s);
t_symbol    *symbol_stripTemplateIdentifier (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_color     color_withRGB                   (int argc, t_atom *argv);
t_color     color_withDigits                (int c);
t_color     color_withEncoded               (t_symbol *s);

t_symbol    *color_toEncoded                (t_color color);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         math_compareFloat               (t_float a, t_float b);
t_float     math_euclideanDistance          (t_float a, t_float b, t_float c, t_float d);

t_float     math_midiToFrequency            (t_float f);
t_float     math_frequencyToMidi            (t_float f);
t_float     math_rootMeanSquareToDecibel    (t_float f);
t_float     math_decibelToRootMeanSquare    (t_float f);
t_float     math_powerToDecibel             (t_float f);
t_float     math_decibelToPower             (t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     string_copy                                 (char *dest, size_t size, const char *src);
t_error     string_add                                  (char *dest, size_t size, const char *src);
t_error     string_append                               (char *dest, size_t size, const char *src, int n);
t_error     string_sprintf                              (char *dest, size_t size, const char *format, ...);
t_error     string_addSprintf                           (char *dest, size_t size, const char *format, ...);
t_error     string_addAtom                              (char *dest, size_t size, t_atom *a);
t_error     string_clear                                (char *dest, size_t size);

int         string_startWith                            (const char *s, const char *isStart);
int         string_endWith                              (const char *s, const char *isEnd);
int         string_contains                             (const char *s, const char *isContained);
int         string_containsCharacterAtStart             (const char *s, const char *chars);

int         string_indexOfFirstOccurrenceUntil          (char *s, const char *chars, int n);
int         string_indexOfFirstOccurrenceFrom           (char *s, const char *chars, int n);
int         string_indexOfFirstOccurrenceFromEnd        (char *s, const char *chars);
void        string_replaceCharacter                     (char *s, char toBeReplaced, char c);

void        string_getNumberOfColumnsAndLines           (char *s, int *numberOfColumns, int *numberOfLines);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* True if the string start with a dollar following by zero or more numbers. */

static inline int string_containsOneDollarFollowingByNumbers (const char *s)
{
    if (*s != '$') { return 0; } while (*(++s)) { if (*s < '0' || *s > '9') { return 0; } }
    
    return 1;
}

/* True if the string start with a dollar following by one number. */

static inline int string_startWithOneDollarAndOneNumber (const char *s)
{
    PD_ASSERT (s[0] != 0);
    
    if (s[0] != '$' || s[1] < '0' || s[1] > '9') { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define COLOR_IEM_BACKGROUND        0xffffff    // White.
#define COLOR_IEM_FOREGROUND        0x000000    // Black.
#define COLOR_IEM_LABEL             0x000000    // Black.
#define COLOR_IEM_PANEL             0xcccccc    // Grey.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define COLOR_SCALAR_WRONG          0xdddddd    // Grey.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define COLOR_OPENED                0xdddddd    // Grey.
#define COLOR_NORMAL                0x000000    // Black.
#define COLOR_SELECTED              0x0000ff    // Blue.
#define COLOR_GOP                   0xff8080    // Red.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define COLOR_MASK                  0xffffff

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_color color_checked (t_color color)
{
    return (color & COLOR_MASK);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int char_isEnd (char c)
{
    return (c == ',' || c == ';');
}

static inline int char_isEscape (char c)
{
    return (c == '\\');
}

static inline int char_isWhitespace (char c)
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

static inline int char_isAlphanumeric (char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_'));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_utils_h_
