
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_error_h_
#define __m_error_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_error1   (char *);
void error_error2   (char *, char *);
void error_post     (int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_invalidExpansion                 (void);
void error_stackOverflow                    (void);
void error_ioStuck                          (void);
void error_stubNotFound                     (void);
void error_tooManyCharacters                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_invalidPointer                   (t_symbol *);
void error_canNotSetMultipleFields          (t_symbol *);
void error_mismatchTypeOrUnspecifiedField   (t_symbol *);
void error_unspecifiedArrayField            (t_symbol *);
void error_recursiveInstantiation           (t_symbol *);
void error_sendReceiveLoop                  (t_symbol *);
void error_canNotOpen                       (t_symbol *);
void error_canNotCreate                     (t_symbol *);
void error_failsToRead                      (t_symbol *);
void error_failsToWrite                     (t_symbol *);
void error_ignored                          (t_symbol *);
void error_failed                           (t_symbol *);
void error_alreadyExists                    (t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_unexpected                       (t_symbol *, t_symbol *);
void error_invalid                          (t_symbol *, t_symbol *);
void error_noSuch                           (t_symbol *, t_symbol *);
void error_canNotFind                       (t_symbol *, t_symbol *);
void error_unknownMethod                    (t_symbol *, t_symbol *);
void error_unknownFunction                  (t_symbol *, t_symbol *);
void error_invalidArgumentsFor              (t_symbol *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_canNotMake                       (int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_invalidArguments                 (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_error_h_
