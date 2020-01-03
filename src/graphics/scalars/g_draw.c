
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void *drawtext_new      (t_symbol *, int, t_atom *);
void *drawpolygon_new   (t_symbol *, int, t_atom *);
void *plot_new          (t_symbol *, int, t_atom *);
void *drawcircle_new    (t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void buffer_appendDescriptor (t_buffer *, t_fielddescriptor *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class  *draw_class;    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* A dummy object. */

typedef struct _draw {
    t_object    x_obj;          /* Must be the first. */
    } t_draw;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void draw_makeObjectParseText (t_symbol *s, t_buffer *b, int argc, t_atom *argv)
{
    t_fielddescriptor x;
    t_fielddescriptor y;
    t_fielddescriptor color;
    t_symbol *label = &s_;
    t_symbol *field = &s_;
    
    field_setAsFloatConstant (&x,     0.0);
    field_setAsFloatConstant (&y,     0.0);
    field_setAsFloatConstant (&color, 0.0);
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        if (argc > 1 && t == sym___dash__visible) {
            buffer_appendAtom (b, argv + 0);
            buffer_appendAtom (b, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__x) {
            field_setAsFloat (&x, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__y) {
            field_setAsFloat (&y, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__color) {
            field_setAsFloat (&color, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__label) {
            label = atom_getSymbolAtIndex (1, argc, argv);
            argc -= 2; argv += 2;
        
        } else if (t == sym___dash____dash__) {
            argc--; argv++;
            break;
        
        } else {
            break;
        }
    }

    error__options (s, argc, argv);
    
    field = atom_getSymbolAtIndex (0, argc, argv);
    
    if (argc) { argc--; argv++; }
    if (argc) { warning_unusedArguments (s, argc, argv); }

    buffer_appendSymbol (b, field);
    buffer_appendDescriptor (b, &x);
    buffer_appendDescriptor (b, &y);
    buffer_appendDescriptor (b, &color);
    
    if (label != &s_) { buffer_appendSymbol (b, label); }
}

static t_symbol *draw_makeObjectParsePolygon (t_symbol *s, t_buffer *b, int argc, t_atom *argv)
{
    t_fielddescriptor color;
    t_fielddescriptor fillcolor;
    t_fielddescriptor width;
    int isFilled = 0;
    int isCurved = 0;
    
    field_setAsFloatConstant (&color,     0.0);
    field_setAsFloatConstant (&fillcolor, 0.0);
    field_setAsFloatConstant (&width,     1.0);
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        if (argc > 1 && t == sym___dash__visible) {
            buffer_appendAtom (b, argv + 0);
            buffer_appendAtom (b, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__x) {
            buffer_appendAtom (b, argv + 0);
            buffer_appendAtom (b, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__y) {
            buffer_appendAtom (b, argv + 0);
            buffer_appendAtom (b, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__color) {
            field_setAsFloat (&color, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__width) {
            field_setAsFloat (&width, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__fillcolor) {
            field_setAsFloat (&fillcolor, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (t == sym___dash__curve) {
            isCurved = 1;
            argc--; argv++;
        
        } else if (t == sym___dash__fill) {
            isFilled = 1;
            argc--; argv++;
        
        } else if (t == sym___dash____dash__) {
            argc--; argv++;
            break;
            
        } else {
            break;
        }
    }

    error__options (s, argc, argv);

    if (isFilled) { buffer_appendDescriptor (b, &fillcolor); }
    
    buffer_appendDescriptor (b, &color);
    buffer_appendDescriptor (b, &width);
    buffer_append (b, argc, argv);

    if (isFilled) {
        if (isCurved) { return sym_filledcurve; } else { return sym_filledpolygon; }
    } else {
        if (isCurved) { return sym_drawcurve; } else { return sym_drawpolygon; }
    }
}

static void draw_makeObjectParsePlot (t_symbol *s, t_buffer *b, int argc, t_atom *argv)
{
    t_fielddescriptor x;
    t_fielddescriptor y;
    t_fielddescriptor color;
    t_fielddescriptor width;
    t_fielddescriptor increment;
    t_symbol *field = &s_;
    
    field_setAsFloatConstant (&x,         0.0);
    field_setAsFloatConstant (&y,         0.0);
    field_setAsFloatConstant (&color,     0.0);
    field_setAsFloatConstant (&width,     1.0);
    field_setAsFloatConstant (&increment, 1.0);
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        if (argc > 1 && t == sym___dash__visible) {
            buffer_appendAtom (b, argv + 0);
            buffer_appendAtom (b, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__x) {
            field_setAsFloat (&x, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__y) {
            field_setAsFloat (&y, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__color) {
            field_setAsFloat (&color, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__width) {
            field_setAsFloat (&width, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__increment) {
            field_setAsFloat (&increment, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__fieldx) {
            buffer_appendSymbol (b, sym___dash__x);
            buffer_appendAtom (b, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__fieldy) {
            buffer_appendSymbol (b, sym___dash__y);
            buffer_appendAtom (b, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__fieldw) {
            buffer_appendSymbol (b, sym___dash__width);
            buffer_appendAtom (b, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (t == sym___dash__curve) {
            buffer_appendAtom (b, argv);
            argc--; argv++;
        
        } else if (t == sym___dash____dash__) {
            argc--; argv++;
            break;
            
        } else {
            break;
        }
    }

    error__options (s, argc, argv);
    
    field = atom_getSymbolAtIndex (0, argc, argv);
    
    if (argc) { argc--; argv++; }
    if (argc) { warning_unusedArguments (s, argc, argv); }

    buffer_appendSymbol (b, field);
    buffer_appendDescriptor (b, &color);
    buffer_appendDescriptor (b, &width);
    buffer_appendDescriptor (b, &x);
    buffer_appendDescriptor (b, &y);
    buffer_appendDescriptor (b, &increment);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *draw_makeObject (t_symbol *s, int argc, t_atom *argv)
{
    t_pd *newest = NULL;
    
    if (!argc || !IS_SYMBOL (argv)) { newest = pd_new (draw_class); }
    else {
    //
    t_symbol *o = NULL;
    t_symbol *t = GET_SYMBOL (argv);
    
    instance_setNewestObject (NULL);
    
    if (t == sym_text)          { o = sym_drawtext; }
    else if (t == sym_polygon)  { o = sym_drawpolygon; }
    else if (t == sym_plot)     { o = sym_plot; }
    else if (t == sym_circle)   { o = sym_drawcircle; }
    else {
        error_unexpected (sym_draw, t);
    }
    
    argc--; argv++;
    
    if (o) {
    //
    t_buffer *args = buffer_new();
    
    if (o == sym_drawtext) {
        draw_makeObjectParseText (s, args, argc, argv);
        newest = (t_pd *)drawtext_new (o, buffer_getSize (args), buffer_getAtoms (args));
        
    } else if (o == sym_drawpolygon) {
        o = draw_makeObjectParsePolygon (s, args, argc, argv);
        newest = (t_pd *)drawpolygon_new (o, buffer_getSize (args), buffer_getAtoms (args));
        
    } else if (o == sym_plot) {
        draw_makeObjectParsePlot (s, args, argc, argv);
        newest = (t_pd *)plot_new (o, buffer_getSize (args), buffer_getAtoms (args));
        
    } else if (o == sym_drawcircle) {
        newest = (t_pd *)drawcircle_new (o, argc, argv);
    }
    
    buffer_free (args);
    
    instance_setNewestObject (newest);
    //
    }
    //
    }
    
    return newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void draw_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_draw,
            NULL,
            NULL,
            sizeof (t_draw),
            CLASS_DEFAULT,
            A_NULL);
    
    draw_class = c;
    
    class_addCreator ((t_newmethod)draw_makeObject, sym_draw, A_GIMME, A_NULL);
}

void draw_destroy (void)
{
    class_free (draw_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
