
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define EXPR_VARIABLES                  9
#define EXPR_FUNCTIONS                  10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define EXPR_TE_VARIABLE(i, s)          { \
                                            x->x_variables[i].name      = (s)->s_name;  \
                                            x->x_variables[i].address   = x->x_v + (i); \
                                            x->x_variables[i].type      = TE_VARIABLE;  \
                                            x->x_variables[i].context   = NULL; \
                                        }

#define EXPR_TE_FUNCTION(i, s, f, t)    { \
                                            x->x_variables[i].name      = s;    \
                                            x->x_variables[i].address   = f;    \
                                            x->x_variables[i].type      = t;    \
                                            x->x_variables[i].context   = NULL; \
                                        }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

double expr_functionRandom              (void);
double expr_functionRandomMT            (void);
double expr_functionMinimum             (double, double);
double expr_functionMaximum             (double, double);
double expr_functionEqual               (double, double);
double expr_functionUnequal             (double, double);
double expr_functionLessThan            (double, double);
double expr_functionLessEqual           (double, double);
double expr_functionGreaterThan         (double, double);
double expr_functionGreaterEqual        (double, double);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
