/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

/**
 * \file script/scriptfunc.h
 * \brief CBot script functions
 */

#pragma once

#include "CBot/CBot.h"
#include "CBot/CBotTypResult.h"

#include "common/error.h"

#include <string>
#include <unordered_map>
#include <memory>

class CObject;
class CScript;
class CExchangePost;
namespace CBot
{
class CBotVar;
}


class CScriptFunctions
{
public:
    static void Init();

    static CBot::CBotVar* CreateObjectVar(CObject* obj);
    static void DestroyObjectVar(CBot::CBotVar* botVar, bool permanent);

    static bool CheckOpenFiles();

private:
    static CBot::CBotTypResult cEndMission(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cPlayMusic(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cGetObject(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cDelete(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cSearch(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cSearchAll(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cRadar(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cRadarAll(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cDetect(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cDirection(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cProduce(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cDistance(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cSpace(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cFlatSpace(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cFlatGround(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cGoto(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cGrabDrop(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cReceive(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cSend(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cDeleteInfo(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cTestInfo(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cShield(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cFire(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cAim(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cMotor(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cTopo(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cMessage(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cPenDown(const std::vector<CBot::CBotTypResult> &var, void* user);

    static CBot::CBotTypResult cOnePoint(const std::vector<CBot::CBotTypResult> &var, void* user);
    static CBot::CBotTypResult cOneObject(const std::vector<CBot::CBotTypResult> &var, void* user);

    static bool rEndMission(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rPlayMusic(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rStopMusic(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rGetBuild(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rGetResearchEnable(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rGetResearchDone(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rSetBuild(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rSetResearchEnable(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rSetResearchDone(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rGetObjectById(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rGetObject(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rDelete(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rSearch(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rSearchAll(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rRadar(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rRadarAll(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rDetect(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rDirection(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rCanBuild(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rCanResearch(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rResearched(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rBuildingEnabled(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rBuild(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rProduce(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rDistance(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rDistance2d(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rSpace(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rFlatSpace(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rFlatGround(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rWait(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rMove(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rTurn(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rGoto(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rGrab(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rDrop(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rSniff(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rReceive(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rSend(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rDeleteInfo(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rTestInfo(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rThump(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rRecycle(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rShield(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rFire(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rAim(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rMotor(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rJet(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rTopo(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rMessage(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rCmdline(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rIsMovie(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rErrMode(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rIPF(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rAbsTime(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rPenDown(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rPenUp(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rPenColor(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rPenWidth(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rCameraFocus(CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);


    static CBot::CBotTypResult cBusy(CBot::CBotTypResult thisType, const std::vector<CBot::CBotTypResult> &var);
    static CBot::CBotTypResult cFactory(CBot::CBotTypResult thisType, const std::vector<CBot::CBotTypResult> &var);
    static CBot::CBotTypResult cClassNull(CBot::CBotTypResult thisType, const std::vector<CBot::CBotTypResult> &var);
    static CBot::CBotTypResult cClassOneFloat(CBot::CBotTypResult thisType, const std::vector<CBot::CBotTypResult> &var);

    static bool rBusy(CBot::CBotVar *thisclass, CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rFactory(CBot::CBotVar *thisclass, CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rResearch(CBot::CBotVar *thisclass, CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rTakeOff(CBot::CBotVar *thisclass, CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);
    static bool rDestroy(CBot::CBotVar *thisclass, CBot::CBotVar* var, CBot::CBotVar* result, int& exception, void* user);

    static CBot::CBotTypResult cPointConstructor(CBot::CBotTypResult thisType, const std::vector<CBot::CBotTypResult> &var);
    static bool rPointConstructor(CBot::CBotVar* pThis, CBot::CBotVar* var, CBot::CBotVar* pResult, int& Exception, void* user);

    static void uObject(CBot::CBotVar* botThis, void* user);

private:
    static bool     WaitForForegroundTask(CScript* script, CBot::CBotVar* result, int &exception);
    static bool     WaitForBackgroundTask(CScript* script, CBot::CBotVar* result, int &exception);
    static bool     ShouldTaskStop(Error err, int errMode);
    static CExchangePost* FindExchangePost(CObject* object, float power);
};
