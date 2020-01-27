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

#include "object/subclass/water_pump.h"

#include "common/make_unique.h"
#include "common/regex_utils.h"

#include "graphics/engine/oldmodelmanager.h"

#include "level/parser/parserexceptions.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "object/object_create_params.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

#include <boost/lexical_cast.hpp>

CWaterPump::CWaterPump(int id)
    : CBaseBuilding(id, OBJECT_WATERPUMP)
{}

std::unique_ptr<CWaterPump> CWaterPump::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CWaterPump>(params.id);

    obj->SetTeam(params.team);

    int rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);
    modelManager->AddModelReference("info1.mod", false, rank);
    obj->SetPosition(params.pos);
    obj->SetRotationY(params.angle);
    obj->SetFloorHeight(0.0f);

    /*rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    obj->SetObjectRank(1, rank);
    obj->SetObjectParent(1, 0);
    modelManager->AddModelReference("info1.mod", false, rank);
    obj->SetPartPosition(1, Math::Vector(0.0f, 1.0f, 0.0f));
    obj->SetPartRotationY(1, 0.0f);*/

/*    rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    obj->SetObjectRank(1, rank);
    obj->SetObjectParent(1, 0);
    modelManager->AddModelReference("info2.mod", false, rank);
    obj->SetPartPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));

    for (int i = 0; i < 3; ++i)
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2+i*2, rank);
        obj->SetObjectParent(2+i*2, 1);
        modelManager->AddModelReference("info3.mod", false, rank);
        obj->SetPartPosition(2+i*2, Math::Vector(0.0f, 4.5f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3+i*2, rank);
        obj->SetObjectParent(3+i*2, 2+i*2);
        modelManager->AddModelReference("radar4.mod", false, rank);
        obj->SetPartPosition(3+i*2, Math::Vector(0.0f, 0.0f, -4.0f));

        obj->SetPartRotationY(2+i*2, 2.0f*Math::PI/3.0f*i);
    }*/

    obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
    obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 11.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
    obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 5.0f, 0.0f), 6.0f));

    obj->CreateShadowCircle(8.0f, 1.0f);

    Math::Vector pos = obj->GetPosition();
    pos.y += params.height;
    obj->SetPosition(pos);  // to display the shadows immediately

    /*auto objAuto = MakeUnique<CAutoInfo>(obj.get());
    objAuto->Init();
    obj->SetAuto(std::move(objAuto));*/

    engine->LoadAllTextures();

    return obj;
}

void CWaterPump::Write(CLevelParserLine* line)
{
    CBaseBuilding::Write(line);
}

void CWaterPump::Read(CLevelParserLine* line)
{
    CBaseBuilding::Read(line);
}

