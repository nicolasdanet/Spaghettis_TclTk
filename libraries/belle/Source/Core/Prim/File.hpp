
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2018 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct File {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static bool writeFromString (const char* filename, const std::string& data)
{
    bool b = false;
    
    std::ofstream stream;
    stream.open (filename, std::ios::out | std::ios::trunc | std::ios::binary);
    
    if (stream.is_open()) { 
    //
    stream.write (data.c_str(), data.length());
    b = stream.good();
    stream.close(); 
    //
    }
    
    return b;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < class T > static bool writeFromArray (const char* filename, const Array < T > & data)
{
    bool b = false;
    
    std::ofstream stream;
    stream.open (filename, std::ios::out | std::ios::trunc | std::ios::binary);
    
    if (stream.is_open()) { 
    //
    stream.write (reinterpret_cast < const char* > (&data[0]), data.size() * sizeof (T)); 
    b = stream.good();
    stream.close(); 
    //
    }
    
    return b;
}

template < class T > static bool readToArray (const char* filename, Array < T > & data)
{
    bool b = false;
    
    std::ifstream stream;
    stream.open (filename, std::ios::in | std::ios::binary);

    if (stream.is_open()) {
    //
    stream.seekg (0, std::ios_base::end);
    int size = static_cast < int > (stream.tellg());
    stream.seekg (0, std::ios_base::beg);
    
    data.resize (static_cast < int > (size / sizeof (T)));
    
    stream.read (reinterpret_cast < char* > (&data[0]), size);
    b = stream.good();
    stream.close();
    //
    }

    return b; 
}
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
