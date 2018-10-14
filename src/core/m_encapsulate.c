
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"
#include "../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void            *canvas_newSubpatch                 (t_symbol *);
t_buffer        *clipboard_copyProceed              (t_glist *, int, int);
t_undomanager   *glist_undoReplaceManager           (t_glist *, t_undomanager *);

void            canvas_selectAll                    (t_glist *);
void            glist_objectRemoveSelectedProceed   (t_glist *);
void            glist_objectAddNextProceed          (t_glist *, t_gobj *, t_gobj *);
void            glist_objectAddUndoProceed          (t_glist *, t_gobj *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _inlethelper {
    t_id                    x_srcId;
    int                     x_srcOutlet;
    t_id                    x_destId;
    int                     x_destInlet;
    int                     x_destIndex;
    t_rectangle             x_destRectangle;
    int                     x_isDsp;
    int                     x_assigned;
    int                     x_assignedIndex;
    int                     x_create;
    struct _inlethelper     *x_next;
    } t_inlethelper;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _outlethelper {
    t_id                    x_srcId;
    int                     x_srcOutlet;
    int                     x_srcIndex;
    t_rectangle             x_srcRectangle;
    t_id                    x_destId;
    int                     x_destInlet;
    int                     x_isDsp;
    int                     x_assigned;
    int                     x_assignedIndex;
    int                     x_create;
    struct _outlethelper    *x_next;
    } t_outlethelper;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ENCAPSULATE_MARGIN  (snap_getStep() * 2)
#define ENCAPSULATE_INLETS  (snap_getStep() * 4)
#define ENCAPSULATE_OUTLETS (snap_getStep() * 2)
#define ENCAPSULATE_PAD     (snap_getStep())

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int inlethelper_alreadyAssigned (t_inlethelper *inlets, t_inlethelper *toAssign)
{
    t_inlethelper *t = inlets;
    
    while (t && (t != toAssign)) {
    //
    if (toAssign->x_destId == t->x_destId) {
    if (toAssign->x_destInlet == t->x_destInlet) {
    if (toAssign->x_isDsp == t->x_isDsp) {
        return t->x_assigned;
    }
    }
    }
    t = t->x_next;
    //
    }
    
    return -1;
}

static int inlethelper_compare (t_inlethelper *a, t_inlethelper *b)
{
    int x1 = rectangle_getTopLeftX (&a->x_destRectangle);
    int y1 = rectangle_getTopLeftY (&a->x_destRectangle);
    int i1 = a->x_destInlet;
    int x2 = rectangle_getTopLeftX (&b->x_destRectangle);
    int y2 = rectangle_getTopLeftY (&b->x_destRectangle);
    int i2 = b->x_destInlet;
    
    if (x1 < x2) { return -1; } else if (x1 > x2) { return 1; }
    if (y1 < y2) { return -1; } else if (y1 > y2) { return 1; }
    if (i1 < i2) { return -1; } else if (i1 > i2) { return 1; }
    
    return 0;
}

static int inlethelper_isBefore (t_inlethelper *a, t_inlethelper *b)
{
    return (inlethelper_compare (a, b) <= 0);
}

static t_inlethelper *inlethelper_addSorted (t_inlethelper *inlets, t_glist *glist,
    t_gobj  *src,
    int     srcOutlet,
    t_gobj  *dest,
    int     destInlet,
    int     destIndex,
    int     isSignal)
{
    t_inlethelper *toAdd = (t_inlethelper *)PD_MEMORY_GET (sizeof (t_inlethelper));
    
    toAdd->x_srcId          = gobj_getUnique (src);
    toAdd->x_srcOutlet      = srcOutlet;
    toAdd->x_destId         = gobj_getUnique (dest);
    toAdd->x_destInlet      = destInlet;
    toAdd->x_destIndex      = destIndex;
    toAdd->x_isDsp          = isSignal;
    toAdd->x_assigned       = -1;
    toAdd->x_assignedIndex  = -1;
    
    gobj_getRectangle (dest, glist, &toAdd->x_destRectangle);

    if (inlets) {
    //
    if (inlethelper_isBefore (toAdd, inlets)) { toAdd->x_next = inlets; return toAdd; }
    else {
    //
    t_inlethelper *t = inlets;
        
    while (t->x_next) { if (inlethelper_isBefore (toAdd, t->x_next)) { break; } t = t->x_next; }
    
    toAdd->x_next = t->x_next; t->x_next = toAdd;
    
    return inlets;
    //
    }
    //
    }
    
    return toAdd;
}

static void inlethelper_free (t_inlethelper *inlets)
{
    t_inlethelper *t = inlets;
    
    while (t) { t_inlethelper *next = t->x_next; PD_MEMORY_FREE (t); t = next; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int outlethelper_alreadyAssigned (t_outlethelper *outlets, t_outlethelper *toAssign)
{
    t_outlethelper *t = outlets;
    
    while (t && (t != toAssign)) {
    //
    if (toAssign->x_srcId == t->x_srcId) {
    if (toAssign->x_srcOutlet == t->x_srcOutlet) {
        PD_ASSERT (toAssign->x_isDsp == t->x_isDsp); return t->x_assigned;
    }
    }
    t = t->x_next;
    //
    }
    
    return -1;
}

static int outlethelper_compare (t_outlethelper *a, t_outlethelper *b)
{
    int x1 = rectangle_getTopLeftX (&a->x_srcRectangle);
    int y1 = rectangle_getTopLeftY (&a->x_srcRectangle);
    int i1 = a->x_srcOutlet;
    int x2 = rectangle_getTopLeftX (&b->x_srcRectangle);
    int y2 = rectangle_getTopLeftY (&b->x_srcRectangle);
    int i2 = b->x_srcOutlet;
    
    if (x1 < x2) { return -1; } else if (x1 > x2) { return 1; }
    if (y1 < y2) { return -1; } else if (y1 > y2) { return 1; }
    if (i1 < i2) { return -1; } else if (i1 > i2) { return 1; }
    
    return 0;
}

static int outlethelper_isBefore (t_outlethelper *a, t_outlethelper *b)
{
    return (outlethelper_compare (a, b) <= 0);
}

static t_outlethelper *outlethelper_addSorted (t_outlethelper *outlets, t_glist *glist,
    t_gobj  *src,
    int     srcOutlet,
    int     srcIndex,
    t_gobj  *dest,
    int     destInlet,
    int     isSignal)
{
    t_outlethelper *toAdd = (t_outlethelper *)PD_MEMORY_GET (sizeof (t_outlethelper));
    
    toAdd->x_srcId          = gobj_getUnique (src);
    toAdd->x_srcOutlet      = srcOutlet;
    toAdd->x_srcIndex       = srcIndex;
    toAdd->x_destId         = gobj_getUnique (dest);
    toAdd->x_destInlet      = destInlet;
    toAdd->x_isDsp          = isSignal;
    toAdd->x_assigned       = -1;
    toAdd->x_assignedIndex  = -1;
    
    gobj_getRectangle (src, glist, &toAdd->x_srcRectangle);

    if (outlets) {
    //
    if (outlethelper_isBefore (toAdd, outlets)) { toAdd->x_next = outlets; return toAdd; }
    else {
    //
    t_outlethelper *t = outlets;
        
    while (t->x_next) { if (outlethelper_isBefore (toAdd, t->x_next)) { break; } t = t->x_next; }
    
    toAdd->x_next = t->x_next; t->x_next = toAdd;
    
    return outlets;
    //
    }
    //
    }
    
    return toAdd;
}

static void outlethelper_free (t_outlethelper *outlets)
{
    t_outlethelper *t = outlets;
    
    while (t) { t_outlethelper *next = t->x_next; PD_MEMORY_FREE (t); t = next; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_inlethelper *encapsulate_addInletsToSnippetFetch (t_glist *glist)
{
    t_inlethelper *inlets    = NULL;
    t_outconnect *connection = NULL;
    t_traverser t;
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    t_gobj *o = cast_gobj (traverser_getSource (&t));
    t_gobj *d = cast_gobj (traverser_getDestination (&t));
    int m = glist_objectIsSelected (glist, o);
    int n = glist_objectIsSelected (glist, d);
    
    if (!m && n) {
    //
    inlets = inlethelper_addSorted (inlets, glist,
                o,
                traverser_getIndexOfOutlet (&t),
                d,
                traverser_getIndexOfInlet (&t),
                glist_objectGetIndexAmongSelected (glist, d),
                cord_isSignal (traverser_getCord (&t)));
    //
    }
    //
    }
    
    return inlets;
}

static int encapsulate_addInletsToSnippetAssign (t_glist *glist, t_inlethelper *inlets)
{
    t_inlethelper *t = inlets;
    
    int n = glist_objectGetNumberOfSelected (glist);
    int k = 0;
    
    while (t) {
    //
    int j = inlethelper_alreadyAssigned (inlets, t);
    
    t->x_assigned      = (j != -1) ? j : k++;
    t->x_assignedIndex = t->x_assigned + n;
    t->x_create        = (j != -1) ? 0 : 1;
    
    t = t->x_next;
    //
    }
    
    return (n + k);
}

static void encapsulate_addInletsToSnippetCreate (t_glist *glist,
    t_inlethelper *inlets,
    t_buffer *b,
    t_rectangle *r)
{
    t_inlethelper *t = inlets;
    
    int left = rectangle_getTopLeftX (r);
    int up   = rectangle_getTopLeftY (r);
    int step = font_getWidth (glist_getFontSize (glist)) * 6;
    
    up   -= ENCAPSULATE_INLETS;
    step += ENCAPSULATE_PAD;
    
    while (t) {
    //
    if (t->x_create) {
    //
    int a = rectangle_getTopLeftX (&t->x_destRectangle);
    
    left = PD_MAX (left, a);
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  left);
    buffer_appendFloat (b,  up);
    buffer_appendSymbol (b, t->x_isDsp ? sym_inlet__tilde__ : sym_inlet);
    buffer_appendSemicolon (b);
    
    left += step;
    
    if (snap_hasSnapToGrid()) { left = snap_getSnapped (left); }
    //
    }
    
    t = t->x_next;
    //
    }
}

static void encapsulate_addInletsToSnippetConnect (t_glist *glist, t_inlethelper *inlets, t_buffer *b)
{
    t_inlethelper *t = inlets;
    
    while (t) {
    //
    if (t->x_create) {
    //
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_connect);
    buffer_appendFloat (b,  t->x_assignedIndex);
    buffer_appendFloat (b,  0);
    buffer_appendFloat (b,  t->x_destIndex);
    buffer_appendFloat (b,  t->x_destInlet);
    buffer_appendSemicolon (b);
    //
    }
    
    t = t->x_next;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_outlethelper *encapsulate_addOutletsToSnippetFetch (t_glist *glist)
{
    t_outlethelper *outlets  = NULL;
    t_outconnect *connection = NULL;
    t_traverser t;
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    t_gobj *o = cast_gobj (traverser_getSource (&t));
    t_gobj *d = cast_gobj (traverser_getDestination (&t));
    int m = glist_objectIsSelected (glist, o);
    int n = glist_objectIsSelected (glist, d);
    
    if (m && !n) {
    //
    outlets = outlethelper_addSorted (outlets, glist,
                o,
                traverser_getIndexOfOutlet (&t),
                glist_objectGetIndexAmongSelected (glist, o),
                d,
                traverser_getIndexOfInlet (&t),
                cord_isSignal (traverser_getCord (&t)));
    //
    }
    //
    }
    
    return outlets;
}

static void encapsulate_addOutletsToSnippetAssign (t_glist *glist, t_outlethelper *outlets, int n)
{
    t_outlethelper *t = outlets;
    
    int k = 0;
    
    while (t) {
    //
    int j = outlethelper_alreadyAssigned (outlets, t);
    
    t->x_assigned      = (j != -1) ? j : k++;
    t->x_assignedIndex = t->x_assigned + n;
    t->x_create        = (j != -1) ? 0 : 1;
    
    t = t->x_next;
    //
    }
}

static void encapsulate_addOutletsToSnippetCreate (t_glist *glist,
    t_outlethelper *outlets,
    t_buffer *b,
    t_rectangle *r)
{
    t_outlethelper *t = outlets;
    
    int left = rectangle_getTopLeftX (r);
    int down = rectangle_getBottomRightY (r);
    int step = font_getWidth (glist_getFontSize (glist)) * 6;
    
    down += ENCAPSULATE_OUTLETS;
    step += ENCAPSULATE_PAD;
    
    if (snap_hasSnapToGrid()) { down = snap_getSnapped (down); }
    
    while (t) {
    //
    if (t->x_create) {
    //
    int a = rectangle_getTopLeftX (&t->x_srcRectangle);
    
    left = PD_MAX (left, a);
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  left);
    buffer_appendFloat (b,  down);
    buffer_appendSymbol (b, t->x_isDsp ? sym_outlet__tilde__ : sym_outlet);
    buffer_appendSemicolon (b);
    
    left += step;
    
    if (snap_hasSnapToGrid()) { left = snap_getSnapped (left); }
    //
    }
    
    t = t->x_next;
    //
    }
}

static void encapsulate_addOutletsToSnippetConnect (t_glist *glist, t_outlethelper *outlets, t_buffer *b)
{
    t_outlethelper *t = outlets;
    
    while (t) {
    //
    if (t->x_create) {
    //
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_connect);
    buffer_appendFloat (b,  t->x_srcIndex);
    buffer_appendFloat (b,  t->x_srcOutlet);
    buffer_appendFloat (b,  t->x_assignedIndex);
    buffer_appendFloat (b,  0);
    buffer_appendSemicolon (b);
    //
    }
    
    t = t->x_next;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void encapsulate_encapsulateAddInletsAndOutlets (t_glist *glist,
    t_buffer *b,
    t_rectangle *r,
    t_inlethelper **i,
    t_outlethelper **o)
{
    t_inlethelper *inlets   = encapsulate_addInletsToSnippetFetch (glist);
    t_outlethelper *outlets = encapsulate_addOutletsToSnippetFetch (glist);
    
    int n = encapsulate_addInletsToSnippetAssign (glist, inlets);
    encapsulate_addInletsToSnippetCreate (glist, inlets, b, r);
    encapsulate_addInletsToSnippetConnect (glist, inlets, b);
    
    encapsulate_addOutletsToSnippetAssign (glist, outlets, n);
    encapsulate_addOutletsToSnippetCreate (glist, outlets, b, r);
    encapsulate_addOutletsToSnippetConnect (glist, outlets, b);
    
    *i = inlets;
    *o = outlets;
}

static t_glist *encapsulate_encapsulateNewSubpatch (t_glist *owner, t_point *position)
{
    t_glist *x = NULL;
    int a = point_getX (position);
    int b = point_getY (position);
    
    t_buffer *t = buffer_new(); buffer_appendSymbol (t, sym_pd);
    
    instance_stackPush (owner);
    
    x = cast_glist (canvas_newSubpatch (&s_));
    
    instance_stackPop (owner);
    
    object_setBuffer (cast_object (x), t);
    object_setSnappedX (cast_object (x), a);
    object_setSnappedY (cast_object (x), b);
    object_setWidth (cast_object (x), 0);
    object_setType (cast_object (x), TYPE_OBJECT);
    
    glist_objectAdd (owner, cast_gobj (x));
    
    return x;
}

static void encapsulate_encapsulatePaste (t_glist *owner, t_glist *subpatch, t_buffer *b)
{
    /* Temporary use parent's undomanager. */
    
    t_undomanager *t = glist_undoReplaceManager (subpatch, glist_getUndoManager (owner));
    
        snippet_disposeObjects (b, ENCAPSULATE_MARGIN); instance_loadSnippet (subpatch, b);
    
        glist_loadbang (subpatch);
    
        canvas_selectAll (subpatch);
    
    glist_undoReplaceManager (subpatch, t);
}

static void encapsulate_encapsulateConnectInletsAndOutlets (t_glist *owner,
    t_glist *subpatch,
    t_inlethelper *inlets,
    t_outlethelper *outlets)
{
    t_id subpatchId = gobj_getUnique (cast_gobj (subpatch));
    
    /* Inlets. */
    
    {
        t_inlethelper *t = inlets;
    
        while (t) {
        //
        t_error err = glist_lineConnectByUnique (t->x_srcId, t->x_srcOutlet, subpatchId, t->x_assigned);
        
        PD_ASSERT (!err); PD_UNUSED (err);
        
        t = t->x_next;
        //
        }
    }
    
    /* Outlets. */
    
    {
        t_outlethelper *t = outlets;
    
        while (t) {
        //
        t_error err = glist_lineConnectByUnique (subpatchId, t->x_assigned, t->x_destId, t->x_destInlet);
        
        PD_ASSERT (!err); PD_UNUSED (err);
        
        t = t->x_next;
        //
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void encapsulate_encapsulate (t_glist *glist)
{
    int undoable = glist_undoIsOk (glist);
    
    if (editor_hasSelection (glist_getEditor (glist))) {
    //
    if (undoable) { glist_undoAppend (glist, undoencapsulate_new()); }
    
    {
        t_rectangle r = glist_objectGetBoundingBoxOfSelected (glist);
        t_buffer *b   = clipboard_copyProceed (glist, 0, 1);
        int state     = dsp_suspend();
        t_point pt    = point_make (rectangle_getMiddleX (&r), rectangle_getMiddleY (&r));
    
        t_inlethelper *inlets   = NULL;
        t_outlethelper *outlets = NULL;
        t_glist *subpatch       = NULL;
    
        encapsulate_encapsulateAddInletsAndOutlets (glist, b, &r, &inlets, &outlets);
        glist_objectRemoveSelectedProceed (glist);
        subpatch = encapsulate_encapsulateNewSubpatch (glist, &pt);
        encapsulate_encapsulatePaste (glist, subpatch, b);
        encapsulate_encapsulateConnectInletsAndOutlets (glist, subpatch, inlets, outlets);
        outlethelper_free (outlets);
        inlethelper_free (inlets);
    
        dsp_resume (state);
        buffer_free (b);
        glist_setDirty (glist, 1);
    }
    
    if (undoable) { glist_undoAppendSeparator (glist); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
