
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Handy to store and convert the dimensions of surfaces. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef int Unit;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Units {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://en.wikipedia.org/wiki/Point_%28typography%29 > */

public:
    static const Unit kMeter = 0;
    static const Unit kMillimeter = 1;
    static const Unit kCentimeter = 2;
    static const Unit kInch  = 3;
    static const Unit kPoint = 4;
    static const Unit kPixel = 5;

private:
    static double kPPI;
    
public:
    static void setPixelsPerInch (double ppi)
    {
        Units::kPPI = ppi;
    }
    
    static Affine pointToPixel()
    {
        return Affine::scale (Vector (Units::kPPI / 72.0, Units::kPPI / 72.0));
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

public:
    static double getConversionFactor (Unit unit)            /* Conversion ratio to the SI unit. */
    {
        switch (unit) {
            case kMeter      : return 1.0;
            case kMillimeter : return 1.0 / 1000.0;
            case kCentimeter : return 1.0 / 100.0;
            case kInch       : return 25.4 / 1000.0;
            case kPoint      : return 25.4 / 1000.0 / 72.0;
            case kPixel      : return 25.4 / 1000.0 / kPPI;
        }
        
        return 1.0;
    }
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < Unit T > class Measurement : public Vector {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Measurement() 
    {
    }
    
    Measurement (double x, double y) : Vector (x, y)
    {
        PRIM_ASSERT (getX() >= 0.0);
        PRIM_ASSERT (getY() >= 0.0);
    }
    
    template < Unit F > Measurement (const Measurement < F > & m)
    {
        double multiplier = Units::getConversionFactor (F) / Units::getConversionFactor (T);

        getX() = m.getX() * multiplier;
        getY() = m.getY() * multiplier;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_CPP11

public:
    Measurement (const Measurement < T > &) = default;
    Measurement (Measurement < T > &&) = default;
    Measurement < T > & operator = (const Measurement < T > &) = default;
    Measurement < T > & operator = (Measurement < T > &&) = default;

#endif

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef Measurement < Units::kMeter > Meters;
typedef Measurement < Units::kMillimeter > Millimeters;
typedef Measurement < Units::kCentimeter > Centimeters;
typedef Measurement < Units::kInch > Inches;
typedef Measurement < Units::kPoint > Points;
typedef Measurement < Units::kPixel > Pixels;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

double Units::kPPI = 72.0;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
