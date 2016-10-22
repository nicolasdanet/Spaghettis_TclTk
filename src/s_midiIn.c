
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void inmidi_noteOn(int portno, int channel, int pitch, int velo)
{
    if (pd_isThingQuiet (sym__notein))
    {
        t_atom at[3];
        SET_FLOAT(at, pitch);
        SET_FLOAT(at+1, velo);
        SET_FLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list (pd_getThing (sym__notein), 3, at);
    }
}

void inmidi_controlChange(int portno, int channel, int ctlnumber, int value)
{
    if (pd_isThingQuiet (sym__ctlin))
    {
        t_atom at[3];
        SET_FLOAT(at, ctlnumber);
        SET_FLOAT(at+1, value);
        SET_FLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list (pd_getThing (sym__ctlin), 3, at);
    }
}

void inmidi_programChange(int portno, int channel, int value)
{
    if (pd_isThingQuiet (sym__pgmin))
    {
        t_atom at[2];
        SET_FLOAT(at, value + 1);
        SET_FLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list (pd_getThing (sym__pgmin), 2, at);
    }
}

void inmidi_pitchBend(int portno, int channel, int value)
{
    if (pd_isThingQuiet (sym__bendin))
    {
        t_atom at[2];
        SET_FLOAT(at, value);
        SET_FLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list (pd_getThing (sym__bendin), 2, at);
    }
}

void inmidi_afterTouch(int portno, int channel, int value)
{
    if (pd_isThingQuiet (sym__touchin))
    {
        t_atom at[2];
        SET_FLOAT(at, value);
        SET_FLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list (pd_getThing (sym__touchin), 2, at);
    }
}

void inmidi_polyPressure(int portno, int channel, int pitch, int value)
{
    if (pd_isThingQuiet (sym__polytouchin))
    {
        t_atom at[3];
        SET_FLOAT(at, pitch);
        SET_FLOAT(at+1, value);
        SET_FLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list (pd_getThing (sym__polytouchin), 3, at);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/*
void inmidi_clk(double timing)
{

    static t_float prev = 0;
    static t_float count = 0;
    t_float cur,diff;

    if (pd_isThingQuiet (sym__midiclkin))
    {
        t_atom at[2];
        diff =timing - prev;
        count++;
   
        if (count == 3)
        { 
             SET_FLOAT(at, 1 );
             count = 0;
        }
        else SET_FLOAT(at, 0);

        SET_FLOAT(at+1, diff);
        pd_list (pd_getThing (sym__midiclkin), 2, at);
        prev = timing;
    }
}
*/

void inmidi_realTimeIn(int portno, int SysMsg)
{
    if (pd_isThingQuiet (sym__midirealtimein))
    {
        t_atom at[2];
        SET_FLOAT(at, portno);
        SET_FLOAT(at+1, SysMsg);
        pd_list (pd_getThing (sym__midirealtimein), 2, at);
    }
}

void inmidi_byte(int portno, int byte)
{
    t_atom at[2];
    if (pd_isThingQuiet (sym__midiin))
    {
        SET_FLOAT(at, byte);
        SET_FLOAT(at+1, portno);
        pd_list (pd_getThing (sym__midiin), 2, at);
    }
}

void inmidi_sysex(int portno, int byte)
{
    t_atom at[2];
    if (pd_isThingQuiet (sym__sysexin))
    {
        SET_FLOAT(at, byte);
        SET_FLOAT(at+1, portno);
        pd_list (pd_getThing (sym__sysexin), 2, at);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
