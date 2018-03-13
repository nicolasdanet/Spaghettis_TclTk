
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2018 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Turn binary bellefont to .hpp file to be embedded inside the executable. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../Source/Belle.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

using namespace prim;
using namespace belle;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* LLVM analyzer (clang-802.0.42) seems to report false positives. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifndef __clang_analyzer__

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Bellefont : public Paintable {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Bellefont (const std::string& name, int first = 0, int last = -1)
    {
        std::string filepath;
    
        filepath += "../../../Resources/Bellefont/";
        filepath += name;
        filepath += ".bellefont";
    
        Array < byte > input, output;
    
        if (File::readToArray (filepath.c_str(), input)) {
        //
        typeface_.importBinary (&input[0]); 
        typeface_.crop (first, last);
        typeface_.exportBinary (output);
        
        std::string txt;
        
        for (int i = 0; i < typeface_.size(); ++i) {
            std::string u = String::asHex (typeface_.getGlyphAtIndex (i)->getCharacter());
            txt += String::paddedLeft (u, 4, '0');
            txt += " ";
            if ((i + 1) % 10 == 0) { txt += newLine; }
        }
        
        std::string constant = Binary::make (name + "Bellefont", output);
        
        File::writeFromString ((name + ".txt").c_str(), txt);
        File::writeFromString ((name + ".hpp").c_str(), constant);
        //
        }
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIM_CPP11

public:
    Bellefont (const Bellefont&) = delete;
    Bellefont& operator = (const Bellefont&) = delete;
    
#else

private:
    Bellefont (const Bellefont&);
    Bellefont& operator = (const Bellefont&);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    bool start (Painter& painter) override
    {
        /* PDF only. */
        
        painter_ = dynamic_cast < Pageable* > (&painter);
        
        if ((painter_ == nullptr) || painter_->originIsTopLeft()) { return false; }   
        
        /* Arrangement is computed here in inches (whereas painting uses points). */
        
        scale_ = Affine::scale (Vector (72.0, 72.0));
        
        if (Paper::isSmallerThan (painter_->getPageSize(), Paper::portrait (Paper::A5()))) { return false; }
        else {
        //
        Inches inches = painter_->getPageSize();
        double margin = 0.5;
        double height = typeface_.getHeight();      /* Typefaces are normalized to 1. */
        double x = margin;
        double y = inches.getY() - height;
    
        Array < Pointer < Path > > scoped;
        Box bounds;
        
        Pointer < Path > t (new Path());
        
        for (int i = 0; i < typeface_.size(); ++i) {
        //
        const Glyph* glyph = typeface_.getGlyphAtIndex (i);
        const Glyph* next  = typeface_.getGlyphAtIndex (i + 1);
        
        t.get()->addPath (glyph->getPath(), Affine::translation (Vector (x, y)));
        
        if (next) {
        //
        x += glyph->getAdvance();
        
        if ((i + 1) % 10 == 0) { 
            x = margin; 
            y -= height; 
        }
        
        if (y < height) { 
            bounds += t.get()->getBounds();
            scoped.add (t);
            t = new Path(); 
            x = margin; 
            y = inches.getY() - height;
        }
        //
        }
        //
        }
        
        if (!t.get()->isEmpty()) { bounds += t.get()->getBounds(); scoped.add (t);  }
        
        bounds += Point (0.0, 0.0);
        
        scoped.swapWith (pages_); bounds_ = scale_.appliedTo (bounds);
        //
        }
        
        return true;
    }
    
    void end (Painter&) override
    {
    }
    
    void paint (Painter&) override
    {
        for (int i = 0; i < pages_.size(); ++i) {
            painter_->setActivePage (i);
            painter_->pushAffine (scale_);
            painter_->setState (Raster().setFillColor (House::kColorDefault));
            painter_->draw (*(pages_[i]));
            painter_->popAffine (1);
        }
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    int getNumberOfPages (Painter&) override
    {
        return pages_.size();
    }
    
    Points getRequiredSize (Painter&) override
    {
        return Points (bounds_.getWidth(), bounds_.getHeight());
    }
    
private:
    Typeface typeface_;
    Box bounds_;
    Affine scale_;
    Array < Pointer < Path > > pages_;
    Pageable* painter_;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int main()
{
    Bellefont petrucci ("Petrucci");
    Bellefont joie ("Joie", 10);
    Bellefont gentiumBasicRegular ("GentiumBasicRegular", 1, 90);
    Bellefont gentiumBasicBold ("GentiumBasicBold", 1, 90);
    Bellefont gentiumBasicItalic ("GentiumBasicItalic", 1, 90);
    Bellefont gentiumBasicBoldItalic ("GentiumBasicBoldItalic", 1, 90);
    
    Pdf ("Petrucci.pdf").paint (petrucci);
    Pdf ("Joie.pdf").paint (joie);
    Pdf ("GentiumBasicRegular.pdf").paint (gentiumBasicRegular);
    Pdf ("GentiumBasicBold.pdf").paint (gentiumBasicBold);
    Pdf ("GentiumBasicItalic.pdf").paint (gentiumBasicItalic);
    Pdf ("GentiumBasicBoldItalic.pdf").paint (gentiumBasicBoldItalic);
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __clang_analyzer__

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
