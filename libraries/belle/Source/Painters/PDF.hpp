
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    < http://opensource.org/licenses/BSD-2-Clause >

    File modified by Nicolas Danet.
    
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// ====================================

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Basic implementation of the PDF standard. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.adobe.com/content/dam/Adobe/en/devnet/acrobat/pdfs/pdf_reference_1-7.pdf > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle { 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Pdf : public Pageable {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Object {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

public:
    Object (int label) : label_ (label), offset_ (0) 
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIM_CPP11

public:
    Object (const Object&) = delete;
    Object& operator = (const Object&) = delete;
    
#else

private:
    Object (const Object&);
    Object& operator = (const Object&);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void setOffset (int offset)
    {
        offset_ = offset;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    String getOffset() const
    {
        String s = String (String::paddedLeft (offset_, 10, '0').c_str()); s << " 00000 n ";
        return s;
    }
    
    int getLabel() const
    {
        return label_;
    }
    
    const String& getContent() const
    {
        return content_;
    }

    const std::string& getDictionary() const
    {
        return dictionary_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    std::string asReference() const
    {
        std::ostringstream s; s << label_ << " 0 R";
        return s.str();
    } 
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void addToContent (const String& s)
    {
        content_ << s << newLine;
    }
    
    void addToDictionary (const std::string& s)
    {
        dictionary_ += s;
        dictionary_ += newLine;
    }

private:
    int label_;
    int offset_;
    String content_;
    std::string dictionary_;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Pdf (const std::string& filename) : filename_ (filename), size_ (Paper::portrait (Paper::A4()))
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIM_CPP11

public:
    Pdf (const Pdf&) = delete;
    Pdf& operator = (const Pdf&) = delete;
    
#else

private:
    Pdf (const Pdf&);
    Pdf& operator = (const Pdf&);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    bool originIsTopLeft() const override
    {
        return false;
    }
    
    Points getPageSize() override
    {
        return size_;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void setState (const Raster& state) override
    {
        state_ = state;
        
        if (stream_) { 
            stream_->addToContent (String (state_.asPDFString().c_str()));
        }
    }
    
    void pushAffine (const Affine& affine) override
    {
        String s;
        
        s << "    " "q" << newLine << String (affine.asPDFString().c_str());
        
        if (stream_) { 
            stream_->addToContent (s); 
        }
    }
    
    void popAffine (int n) override
    {
        String s;
        
        for (int i = 0; i < n; ++i) { s << "    " "Q" << newLine; }
        
        if (stream_) { 
            stream_->addToContent (s); 
        }
    }
    
    void draw (const Path& path) override
    {
        String s;

        s << String (path.asPDFString().c_str());
        
        bool fill   = (state_.getFillColor().getAlpha() > 0.0);
        bool stroke = (state_.getStrokeColor().getAlpha() > 0.0) && (state_.getWidth() > 0.0);
        
        if (fill && stroke) { s << "    " "B" << newLine; }
        else if (fill)      { s << "    " "f" << newLine; }
        else if (stroke)    { s << "    " "S" << newLine; }
        else { 
            s << "    " "n" << newLine;
        }
      
        if (stream_) { stream_->addToContent (s); }
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void setActivePage (int n) override
    {
        if (n < 0 || n >= contents_.size()) { stream_ = nullptr; }
        else {
        //
        stream_ = contents_[n];
            
        String s;
            
        s << newLine;
        s << "    " "/DeviceRGB cs" << newLine;
        s << "    " "/DeviceRGB CS" << newLine;
            
        stream_->addToContent (s);
        //
        }
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void performPaint (Paintable& toPaint) override
    {
        PRIM_ASSERT (Paper::isSmallerOrEqualThan (toPaint.getRequiredSize (*this), size_));
        
        writeBegin (toPaint);
        writePaint (toPaint);
        writeClose (toPaint);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    void writeBegin (Paintable& toPaint)
    {
        const int catalog = 0;
        const int info = 1;
        const int tree = 2;
        
        int count = 1;
        int n = Math::max (toPaint.getNumberOfPages (*this), 1);
        
        headers_.add (Pointer < Object > (new Object (count++)));
        headers_.add (Pointer < Object > (new Object (count++)));
        headers_.add (Pointer < Object > (new Object (count++)));
        
        for (int i = 0; i < n; ++i) { pages_.add (Pointer < Object > (new Object (count++))); }
        for (int i = 0; i < n; ++i) { contents_.add (Pointer < Object > (new Object (count++))); }
            
        headers_[catalog]->addToDictionary ("/Type /Catalog");
        headers_[catalog]->addToDictionary ("    " "/Pages " + headers_[tree].get()->asReference());
        
        headers_[info]->addToDictionary ("/Producer (Belle, Bonne, Sage)");
        
        headers_[tree]->addToDictionary ("/Type /Pages");
        headers_[tree]->addToDictionary ("    " "/Kids [ " + pages_[0].get()->asReference());
        for (int i = 1; i < n; ++i) { 
        headers_[tree]->addToDictionary ("    " "        " + pages_[i].get()->asReference());
        }
        headers_[tree]->addToDictionary ("    " "      ]");
        headers_[tree]->addToDictionary ("    " "/Count " + String::paddedLeft (pages_.size()));
        
        for (int i = 0; i < n; ++i) {
        //
        std::string sizeX = String::paddedLeft (size_.getX());
        std::string sizeY = String::paddedLeft (size_.getY());
        
        pages_[i]->addToDictionary ("/Type /Page");
        pages_[i]->addToDictionary ("    " "/Parent " + headers_[tree].get()->asReference());
        pages_[i]->addToDictionary ("    " "/Contents " + contents_[i].get()->asReference());
        pages_[i]->addToDictionary ("    " "/MediaBox [ 0 0 " + sizeX + " " + sizeY + " ]");
        pages_[i]->addToDictionary ("    " "/Resources " "<<  >>");
        //
        }
    }
    
    void writePaint (Paintable& toPaint)
    {
        setActivePage (0);
        toPaint.paint (*this);
        stream_ = nullptr;
    }
    
    void writeClose (Paintable&)
    {
        String output;
        
        output << "%PDF-1.3" << newLine;
        output << "%" "\xc2\xa5\xc2\xb1\xc3\xab" << newLine;
        output << newLine;
        
        int size = headers_.size() + pages_.size() + contents_.size() + 1;
        
        for (int i = 0; i < headers_.size(); ++i)  { writeObject (headers_[i], output);  }
        for (int i = 0; i < pages_.size(); ++i)    { writeObject (pages_[i], output);    }
        for (int i = 0; i < contents_.size(); ++i) { writeObject (contents_[i], output); }
    
        int XRefLocation = output.length();
        
        output << "xref" << newLine;
        output << "0 " << size << newLine;
        output << "0000000000 65535 f " << newLine;

        for (int i = 0; i < headers_.size(); ++i)  { output << headers_[i]->getOffset() << newLine;  }
        for (int i = 0; i < pages_.size(); ++i)    { output << pages_[i]->getOffset() << newLine;    }
        for (int i = 0; i < contents_.size(); ++i) { output << contents_[i]->getOffset() << newLine; }
      
        Random rand (filename_);
        std::string unique = rand.nextID();
        
        output << newLine;
        output << "trailer" << newLine;
        output << " << " "/Root " << headers_[0]->asReference() << newLine;
        output << "    " "/Info " << headers_[1]->asReference() << newLine;
        output << "    " "/Size " << size << newLine;
        output << "    " "/ID [ " << "<" << unique << ">" << newLine;
        output << "    " "      " << "<" << unique << ">" << newLine;
        output << "    " "    ]" << newLine;
        output << " >>" << newLine;
        output << "startxref" << newLine;
        output << XRefLocation << newLine;
        output << "%%EOF" << newLine;
        
        headers_.clear();
        pages_.clear();
        contents_.clear();
        
        if (filename_.length()) { File::writeFromString (filename_.c_str(), std::string (output.toCString())); }
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    static void writeObject (Pointer < Object > object, String& output)
    {
        object->setOffset (output.length());

        output << object->getLabel() << " 0 obj" << newLine;
        output << " << " << object->getDictionary().c_str();
        
        if (object->getContent().length() == 0) { output << " >>" << newLine; }
        else {
            if (object->getDictionary().length()) { output << "    "; }
            output << "/Length " << object->getContent().length() << newLine;
            output << " >>" << newLine;
            output << "stream" << newLine;
            output << object->getContent() << newLine;
            output << "endstream" << newLine;
        }
        
        output << "endobj" << newLine;
        output << newLine;
    }
    
private:
    std::string filename_;
    Points size_;
    Raster state_;
    Pointer < Object > stream_;
    Array < Pointer < Object > > headers_;
    Array < Pointer < Object > > pages_;
    Array < Pointer < Object > > contents_;

private:
    PRIM_LEAK_DETECTOR (Pdf)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
