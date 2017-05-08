
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol     *main_directoryExtras;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int          main_directoryWriteRequirePrivileges;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ! ( PD_WINDOWS )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

static char *properties_loadBuffer;     /* Static. */
static FILE *properties_saveFile;       /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error properties_loadBegin (void)
{
    char filepath[PD_STRING] = { 0 };
    
    t_error err = PD_ERROR_NONE;
    
    if (!main_directoryWriteRequirePrivileges) {
        err = string_sprintf (filepath, PD_STRING, "%s/preferences.txt", main_directoryExtras->s_name);
    } else {
        char *home = getenv ("HOME");
        err = string_sprintf (filepath, PD_STRING, "%s/."PD_NAME_LOWERCASE"rc", (home ? home : "."));
    }

    if (!err) { err |= !path_isFileExist (filepath); }
    if (!err) {
    //
    int f;
    
    err |= ((f = file_openRaw (filepath, O_RDONLY)) < 0);
    
    if (!err) {
    //
    off_t length;
    
    err |= ((length = lseek (f, 0, SEEK_END)) < 0);
    err |= (lseek (f, 0, SEEK_SET) < 0); 
    
    if (!err) {
    //
    properties_loadBuffer = (char *)PD_MEMORY_GET ((size_t)length + 2);
    properties_loadBuffer[0] = '\n';
    err |= (read (f, properties_loadBuffer + 1, (size_t)length) < length);
    //
    }
    
    if (err) { properties_loadBuffer[0] = 0; PD_BUG; }
    
    close (f);
    //
    }
    //
    }
    
    return err;
}

void properties_loadClose (void)
{
    if (properties_loadBuffer) {
    //
    PD_MEMORY_FREE (properties_loadBuffer); properties_loadBuffer = NULL; 
    //
    }
}

t_error properties_saveBegin (void)
{
    char filepath[PD_STRING] = { 0 };
    
    t_error err = PD_ERROR_NONE;
    
    if (!main_directoryWriteRequirePrivileges) {
        err = string_sprintf (filepath, PD_STRING, "%s/preferences.txt", main_directoryExtras->s_name);
    } else {
        char *home = getenv ("HOME");
        err = string_sprintf (filepath, PD_STRING, "%s/."PD_NAME_LOWERCASE"rc", (home ? home : "."));
    }
    
    if (!err) { err = ((properties_saveFile = file_openWrite (filepath)) == NULL); }
    
    return err;
}

void properties_saveClose (void)
{
    if (properties_saveFile) {
    //
    fclose (properties_saveFile); properties_saveFile = NULL;
    //
    }
}

int properties_getKey (const char *key, char *value, int size)
{
    char t[PD_STRING] = { 0 };
    char *p = NULL;
    char *pEnd = NULL;
    t_error err = string_sprintf (t, PD_STRING, "\n%s:", key);

    PD_ASSERT (properties_loadBuffer != NULL);
    PD_ASSERT (!err);
    
    p = strstr (properties_loadBuffer, t);
    
    if (p) {
    //
    *value = 0; p += strlen (t);
    
    while (*p == ' ' || *p == '\t') { p++; }
    
    for (pEnd = p; *pEnd && *pEnd != '\n'; pEnd++) { } 
    
    if (*pEnd == '\n') { 
        pEnd--; 
    }
    
    size_t length = pEnd + 1 - p;
    
    if (length > 0) { 
        if (!string_append (value, size, p, (int)length)) { return 1; }
    }
    //
    }
    
    return 0;
}

void properties_setKey (const char *key, const char *value)
{
    if (properties_saveFile) { fprintf (properties_saveFile, "%s: %s\n", key, value); }   // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#else

t_error properties_loadBegin (void)
{
    return PD_ERROR_NONE;
}

void properties_loadClose (void)
{
}

t_error properties_saveBegin (void)
{
    return PD_ERROR_NONE;
}

void properties_saveClose (void)
{
}

int properties_getKey (const char *key, char *value, int size)
{
    HKEY hkey;
    DWORD n = size;
    LONG err = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\" PD_NAME, 0, KEY_QUERY_VALUE, &hkey);
    
    if (err != ERROR_SUCCESS) { return 0; }
    
    err = RegQueryValueEx (hkey, key, 0, 0, value, &n);
    
    if (err != ERROR_SUCCESS) { RegCloseKey (hkey); return 0; }
    
    RegCloseKey (hkey);
    
    return 1;
}

void properties_setKey (const char *key, const char *value)
{
    HKEY hkey;
    LONG err = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                "Software\\" PD_NAME,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE,
                                NULL,
                                &hkey,
                                NULL);
                                
    if (err != ERROR_SUCCESS) { PD_BUG; return; }
    
    err = RegSetValueEx (hkey, key, 0, REG_EXPAND_SZ, value, strlen (value) + 1);
    
    if (err != ERROR_SUCCESS) { PD_BUG; }
    
    RegCloseKey (hkey);
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
