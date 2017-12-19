
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    File modified by Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef BELLE_CORE_INSTRUCTION_HPP
#define BELLE_CORE_INSTRUCTION_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Instruction {

friend class Path;
template < class T, class GM > friend class prim::Array;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    Instruction() : type_ (ClosePath) 
    {
    }
    
    Instruction (Point pt, bool start) : end_ (pt)
    {
        type_ = (start ? MoveTo : LineTo);
    }

    Instruction (Point cp1, Point cp2, Point end, int type) : cp1_ (cp1), cp2_ (cp2), end_ (end), type_ (type)
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_CPP11

public:
    Instruction (const Instruction&) = default;
    Instruction (Instruction&&) = default;
    Instruction& operator = (const Instruction&) = default;
    Instruction& operator = (Instruction&&) = default;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    String asPDFString() const
    {
        String s;
        
        String cp1X = String::paddedLeft (getControl1().getX(), 12);
        String cp1Y = String::paddedLeft (getControl1().getY(), 12);
        String cp2X = String::paddedLeft (getControl2().getX(), 12);
        String cp2Y = String::paddedLeft (getControl2().getY(), 12);
        String endX = String::paddedLeft (getEnd().getX(), 12);
        String endY = String::paddedLeft (getEnd().getY(), 12);
        
        if (isMoveTo())         { s << Tab << endX << " " << endY << " m" << newLine; } 
        else if (isLineTo())    { s << Tab << endX << " " << endY << " l" << newLine; } 
        else if (isClosePath()) { s << Tab << "h" << newLine; }
        else if (isCubicTo())   {
        //
        s << Tab;
        s << cp1X;
        s << " " << cp1Y;
        s << " " << cp2X;
        s << " " << cp2Y;
        s << " " << endX;
        s << " " << endY;
        s << " c" << newLine;
        //
        }
        
        return s;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Point getControl1() const
    {
        return cp1_;
    }
    
    Point getControl2() const
    {
        return cp2_;
    }
    
    Point getEnd() const
    {
        return end_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    bool hasEnd() const 
    {
        return type_ != ClosePath;
    }
    
    bool hasControls() const 
    {
        return type_ == CubicTo;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    int getType() const
    {
        return type_;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    bool isMoveTo() const 
    {
        return type_ == MoveTo;
    }
    
    bool isLineTo() const 
    {
        return type_ == LineTo;
    }
    
    bool isCubicTo() const 
    {
        return type_ == CubicTo;
    }
    
    bool isClosePath() const 
    {
        return type_ == ClosePath;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static Instruction moveTo (Point pt)
    {
        return Instruction (pt, true);
    } 
    
    static Instruction lineTo (Point pt)
    {
        return Instruction (pt, false);
    } 
    
    static Instruction cubicTo (Point cp1, Point cp2, Point end)
    {
        return Instruction (cp1, cp2, end, CubicTo);
    } 
    
    static Instruction closePath()
    {
        return Instruction();
    } 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static Instruction withBezier (const Bezier& bezier) 
    {
        Point start, cp1, cp2, end;
        bezier.getControlPoints (start, cp1, cp2, end);
        return Instruction (cp1, cp2, end, CubicTo);
    }
    
    static Instruction withAffine (const Instruction& instruction, const Affine& affine)
    {
        Point cp1 = affine.appliedTo (instruction.cp1_);
        Point cp2 = affine.appliedTo (instruction.cp2_);
        Point end = affine.appliedTo (instruction.end_);
        return Instruction (cp1, cp2, end, instruction.type_);
    }
    
public:
    enum Type { MoveTo = 1, LineTo, CubicTo, ClosePath };
    
private:
    Point cp1_;
    Point cp2_;
    Point end_;
    int type_;

private:
    PRIM_LEAK_DETECTOR (Instruction)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // BELLE_CORE_INSTRUCTION_HPP
