// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// app.h

#pragma once


#include "common/misc.h"
#include "graphics/common/device.h"
#include "graphics/common/engine.h"

#include <string>


class CInstanceManager;
class CEvent;
class CRobotMain;
class CSound;

struct ApplicationPrivate;


/**
 * \class CApplication
 * \brief Main application
 *
 * This class is responsible for creating and handling main application window,
 * receiving events, etc.
 *
 * ...
 */
class CApplication
{
public:
    //! Constructor (can only be called once!)
    CApplication();
    //! Destructor
    ~CApplication();

public:
    //! Parses commandline arguments
    Error       ParseArguments(int argc, char *argv[]);
    //! Initializes the application
    bool        Create();
    //! Main event loop
    int         Run();

    //! Enters the pause mode
    void        Pause(bool pause);

    //! Updates the simulation state
    void        StepSimulation(float rTime);

    void        SetShowStat(bool show);
    bool        RetShowStat();

    void        SetDebugMode(bool mode);
    bool        RetDebugMode();

    bool        RetSetupMode();

    void        SetJoystickEnabled(bool enable);
    bool        RetJoystickEnabled();

    void        FlushPressKey();
    void        ResetKey();
    void        SetKey(int keyRank, int option, int key);
    int         RetKey(int keyRank, int option);

    void        SetMouseType(Gfx::MouseType type);
    void        SetMousePos(Math::Point pos);

    //? void        SetNiceMouse(bool nice);
    //? bool        RetNiceMouse();
    //? bool        RetNiceMouseCap();

    bool        WriteScreenShot(char *filename, int width, int height);

protected:
    //! Cleans up before exit
    void        Destroy();
    //! Processes an SDL event to Event struct
    void        ParseEvent();
    //! Handles some incoming events
    void        ProcessEvent(Event event);
    //! Renders the image in window
    bool        Render();

    //! Opens the joystick device
    bool OpenJoystick();
    //! Closes the joystick device
    void CloseJoystick();

    //! Converts window coords to interface coords
    Math::Point WindowToInterfaceCoords(int x, int y);

    //HRESULT       ConfirmDevice( DDCAPS* pddDriverCaps, D3DDEVICEDESC7* pd3dDeviceDesc );
    //HRESULT       Initialize3DEnvironment();
    //HRESULT       Change3DEnvironment();
    //HRESULT       CreateZBuffer(GUID* pDeviceGUID);
    //HRESULT       Render3DEnvironment();
    //VOID      Cleanup3DEnvironment();
    //VOID      DeleteDeviceObjects();
    //VOID      DisplayFrameworkError( HRESULT, DWORD );

    void        InitText();
    void        DrawSuppl();
    void        ShowStats();
    void        OutputText(long x, long y, char* str);

protected:
    CInstanceManager*       m_iMan;
    //! Private (SDL-dependent data)
    ApplicationPrivate*     m_private;
    //! Global event queue
    CEventQueue*            m_eventQueue;
    //! Current configuration of display device
    Gfx::DeviceConfig       m_deviceConfig;
    //! Graphics engine
    Gfx::CEngine*           m_engine;
    //! Sound subsystem
    CSound*                 m_sound;
    //! Main class of the proper game engine
    CRobotMain*             m_robotMain;


    //! Code to return at exit
    int             m_exitCode;

    bool            m_active;
    bool            m_activateApp;
    bool            m_ready;

    bool            m_showStats;
    bool            m_debugMode;
    bool            m_setupMode;

    bool            m_joystickEnabled;

    std::string     m_windowTitle;

    //? long            m_vidMemTotal;
    //? bool            m_appUseZBuffer;
    //? bool            m_appUseStereo;
    //? bool            m_audioState;
    //? bool            m_audioTrack;
    //? bool            m_niceMouse;

    int             m_keyState;
    Math::Vector    m_axeKey;
    Math::Vector    m_axeJoy;
    bool            m_joyButton[32];
    Math::Point     m_mousePos;
    long            m_mouseWheel;

    float           m_time;
    long            m_key[50][2];
};
