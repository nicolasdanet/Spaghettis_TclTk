
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

static int file_openRawNative (const char *filepath, int oflag)
{
    char t[PD_STRING]           = { 0 };
    wchar_t ucs2path[PD_STRING] = { 0 };
    
    if (string_copy (t, PD_STRING, filepath)) { PD_BUG; }
    path_slashToBackslashIfNecessary (t);
    u8_utf8toucs2 (ucs2path, PD_STRING, t, -1);

    if (oflag & O_CREAT) { return _wopen (ucs2path, oflag | O_BINARY, _S_IREAD | _S_IWRITE); }
    else {
        return _wopen (ucs2path, oflag | O_BINARY);
    }
}

static FILE *file_openModeNative (const char *filepath, const char *mode)
{
    char t[PD_STRING]           = { 0 };
    wchar_t ucs2path[PD_STRING] = { 0 };
    wchar_t ucs2mode[PD_STRING] = { 0 };
    
    if (string_copy (t, PD_STRING, filepath)) { PD_BUG; }
    path_slashToBackslashIfNecessary (t);
    u8_utf8toucs2 (ucs2path, PD_STRING, t, -1);
    
    mbstowcs (ucs2mode, mode, PD_STRING);
    
    return _wfopen (ucs2path, ucs2mode);
}

#else

static int file_openRawNative (const char *filepath, int oflag)
{
    if (oflag & O_CREAT) { return open (filepath, oflag, 0666); }
    else {
        return open (filepath, oflag);
    } 
}

static FILE *file_openModeNative (const char *filepath, const char *mode)
{
    return fopen (filepath, mode);
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int file_openRaw (const char *filepath, int oflag)
{
    if (!(oflag & O_CREAT)) { if (!(path_isFileExistAsRegularFile (filepath))) { return -1; } }
    
    return file_openRawNative (filepath, oflag);
}

FILE *file_openWrite (const char *filepath)
{
    return file_openModeNative (filepath, "w");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
