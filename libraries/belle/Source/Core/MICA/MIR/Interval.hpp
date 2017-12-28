
/* Copyright (c) 2017 Nicolas Danet. */

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef MICA_MIR_INTERVAL_HPP
#define MICA_MIR_INTERVAL_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace MIR {     /* Must be in his own scope to avoid name collision with the MICA concepts. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Compound intervals are always reduced to a simple interval. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Interval {

friend class Spell;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Interval()                      /* An invalid interval. */
    {
    }
    
private:
    Interval (Concept a, Concept b) : a_ (a), b_ (b)
    {
        PRIM_ASSERT (a_.isUndefined() || !index (mica::ChromaticPitches, a_).isUndefined());
        PRIM_ASSERT (b_.isUndefined() || !index (mica::ChromaticPitches, b_).isUndefined());
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_CPP11

public:
    Interval (const Interval&) = default;
    Interval (Interval&&) = default;
    Interval& operator = (const Interval&) = default;
    Interval& operator = (Interval&&) = default;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static Interval withNotes (Concept a, Concept b)
    {
        return Interval (a, b);
    }
    
    static Interval withName (Concept name, Concept direction = mica::Above, int64 n = 0)
    {
        Concept note = n < 0 ? mica::Undefined : Interval::transpose (mica::C4, name, direction, n);
        
        return Interval::withNotes (mica::C4, note);
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Concept getName() const         /* Always reduced to a simple interval. */
    {
        return map (getDistance(), getQuality());
    }
    
    Concept getOctaves() const      /* Octaves for compound intervals. */
    {
        return octaves();
    }
    
    Concept getDistance() const
    {
        return distance();
    }

    Concept getQuality() const
    {
        return quality();
    }

    Concept getDirection() const
    {
        return direction();
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    bool isSimple() const
    {
        return (getOctaves().getNumerator() == 0);
    }

    bool isCompound() const
    {
        return (getOctaves().getNumerator() != 0);
    }

    bool isEnharmonic() const
    {
        return (getDirection() == mica::Unison);
    }

    bool isValid() const            /* Rather inefficient in case of valid interval. */
    {
        return (!a_.isUndefined() && !b_.isUndefined() && !getName().isUndefined());
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Transpose a pitch according to this interval. */

public:
    Concept appliedTo (Concept p) const
    {   
        Concept d = order().getNumerator() < 0 ? mica::Below : mica::Above;
        
        return Interval::transpose (p, getName(), d, getOctaves());
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Make the Interval class sortable. */
/* Notice that the order obtained is NOT based on musical rules. */

public:
    bool operator < (const Interval& o) const
    {
        return ((a_ < o.a_) || (a_ == o.a_ && b_ < o.b_));
    }
  
    bool operator > (const Interval& o) const
    {
        return ((a_ > o.a_) || (a_ == o.a_ && b_ > o.b_));
    }
    
    bool operator == (const Interval& o) const
    {
        return (a_ == o.a_ && b_ == o.b_);
    }
  
    bool operator != (const Interval& o) const
    {
        return (a_ != o.a_ || b_ != o.b_);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Make the Interval class hashable. */

public:
    std::size_t hash() const
    {
        Concept hashed = UUID::merge (a_, b_);
        
        return hashed.hash();
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    Concept quality() const
    {
        Concept cmp = order();
        
        if (!cmp.isUndefined()) {
        //
        Concept letterA = firstLetter();
        Concept letterB = lastLetter();
        Concept accidentalsA = map (mica::Accidental, a_);
        Concept accidentalsB = map (mica::Accidental, b_);
        
        if (cmp.getNumerator() < 0) { 
            using std::swap; swap (accidentalsA, accidentalsB); swap (letterA, letterB); 
        }
        
        int d = index (letterA, letterB).getNumerator();
        
        Concept qualities = item (mica::DistanceQualities, d);
        Concept diatonic  = item (map (mica::LetterQualities, letterA), d);
        int alterations = index (mica::Accidentals, accidentalsA, accidentalsB).getNumerator();
        
        return item (qualities, diatonic, alterations);
        //
        }
        
        return Concept();
    }

    Concept distance() const
    {
        Concept cmp = order();
        
        if (!cmp.isUndefined()) {
        //
        Concept letterA = firstLetter();
        Concept letterB = lastLetter();
        
        if (cmp.getNumerator() < 0) { using std::swap; swap (letterA, letterB); } 

        if ((letterA == letterB) && (map (a_, mica::Octave) != map (b_, mica::Octave))) { 
            return mica::Octave; 
            
        } else {
            return item (mica::Distances, index (letterA, letterB).getNumerator());
        }
        //
        }
        
        return Concept();
    }

    Concept octaves() const
    {
        Concept keyA = firstKey();
        Concept keyB = lastKey();
        
        if (!keyA.isUndefined() && !keyB.isUndefined()) {
        //
        int n1 = keyA.getNumerator(); 
        int n2 = keyB.getNumerator();
        prim::Math::ascending (n1, n2);
        int d = n2 - n1;
        int k = static_cast < int > (d / 12.0);
        if (d && (d % 12 == 0)) { return Concept (k - 1); }
        else {
            return Concept (k);
        }
        //
        }
        
        return Concept();
    }

    Concept direction() const
    {
        Concept keyA = firstKey();
        Concept keyB = lastKey();
        
        if (!keyA.isUndefined() && !keyB.isUndefined()) {
        //
        if (keyA == keyB) { return mica::Unison; }      /* Enharmonic. */
        else { 
            return (order().getNumerator() < 0 ? mica::Below : mica::Above); 
        }
        //
        }
        
        return Concept();
    }

    Concept order() const
    {
        return index (mica::ChromaticPitches, a_, b_);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    Concept firstKey() const
    {
        return map (a_, mica::MIDIKeyNumber);
    }

    Concept firstLetter() const
    {
        return map (a_, mica::Letter);
    }

    Concept lastKey() const
    {
        return map (b_, mica::MIDIKeyNumber);
    }

    Concept lastLetter() const
    {
        return map (b_, mica::Letter);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    static Concept transpose (Concept pitch, Concept interval, Concept direction, Concept octaves)
    {
        /* Get the base letter values. */
        
        Concept letter = map (pitch, mica::Letter);
        Concept accidental = map (pitch, mica::Accidental);
        Concept octave = map (pitch, mica::Octave);
                
        if (!letter.isUndefined() && !accidental.isUndefined() && !octave.isUndefined()) { 
        //
        /* Get the direction. */
        
        int64 sign = (direction == mica::Above ? 1 : -1);
        
        /* Get the interval values. */
        
        Concept intervalQuality = map (interval, mica::Quality);
        Concept intervalDistance = map (interval, mica::Distance);
        
        /* Get the qualities for that kind of interval. */
        
        int64 intervalDelta = index (mica::Distances, intervalDistance).getNumerator();
        Concept intervalQualities = item (mica::DistanceQualities, intervalDelta);
        
        if (!intervalQuality.isUndefined() && !intervalDistance.isUndefined()) {
        //
        /* Get name of the base and top letters. */
        
        Concept letterResult = item (letter, sign * intervalDelta);
        Concept baseLetter = (sign == 1 ? letter : letterResult);
        Concept topLetter = (sign == 1 ? letterResult : letter);
        
        /* Compute the required accidentals. */
        
        Concept baseQuality = item (map (baseLetter, mica::LetterQualities), intervalDelta);
        int64 accidentalDelta = index (intervalQualities, baseQuality, intervalQuality).getNumerator();
        Concept accidentalResult = item (mica::Accidentals, accidental, sign * accidentalDelta);
        
        /* Compute the octave number. */
        
        int64 octaveDelta = sign * (octaves.getNumerator() + (intervalDistance == mica::Octave ? 1 : 0));
        
        if (baseLetter != mica::C) { 
            octaveDelta += sign * (index (baseLetter, mica::C, topLetter).getNumerator() >= 0 ? 1 : 0);
        }
        
        /* Return the transposed note. */
        
        return map (letterResult, accidentalResult, Concept (octave.getNumerator() + octaveDelta));
        //
        }
        //
        }
        
        return Concept();           /* Undefined. */
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    Concept first() const
    {
        return a_;
    }
    
    Concept last() const
    {
        return b_;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

private:
    Concept a_;
    Concept b_;

private:
    PRIM_LEAK_DETECTOR (Interval)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace MIR

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // MICA_MIR_INTERVAL_HPP