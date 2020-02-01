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


#pragma once

#include "object/subclass/base_building.h"

#include "object/auto/auto.h"

#include <string>
#include <vector>

#include <boost/optional.hpp>

struct ObjectCreateParams;

namespace Gfx
{
class COldModelManager;
class CModelManager;
class CEngine;
}

std::unique_ptr<CObject> CreateObjectWaterPump(
        const ObjectCreateParams& params,
        Gfx::COldModelManager* oldModelManager,
        Gfx::CModelManager* modelManager,
        Gfx::CEngine* engine);

// TODO: integrate this with CExchangePost
/*class CAutoInfo : public CAuto
{
public:
    CAutoInfo(CExchangePost* object);
    ~CAutoInfo();

    void        DeleteObject(bool all=false) override;

    void        Init() override;
    void        Start(int param) override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    bool        CreateInterface(bool select) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    void        UpdateInterface(float rTime);
    void        UpdateList();
    void        UpdateListVirus();

protected:
    CExchangePost*  m_exchangePost;
    enum class Phase : unsigned int;
    Phase           m_phase;
    float           m_progress;
    float           m_speed;
    float           m_timeVirus;
    float           m_lastParticle;
    Math::Vector    m_goal;
    bool            m_lastVirus;
};*/
