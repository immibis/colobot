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
#include "object/interface/liquid_container_object.h"

#include "common/make_unique.h"
#include "common/regex_utils.h"

#include "graphics/engine/oldmodelmanager.h" // XXX delete
#include "graphics/model/model_manager.h"

#include "level/parser/parserexceptions.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "object/object_create_params.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

#include <boost/lexical_cast.hpp>

class CWaterPump : public CBaseBuilding
{
public:
    CWaterPump(int id)
        : CBaseBuilding(id, OBJECT_WATERPUMP)
    {
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Powered)] = true;
    }

    void Write(CLevelParserLine* line) override;
    void Read(CLevelParserLine* line) override;

    using COldObject::SetAuto;
};

struct CAutoWaterPump : public CAuto
{
    CAutoWaterPump(CWaterPump* object) : CAuto(object) {}
    ~CAutoWaterPump() {}

    float cycle = 0.0f;

    bool EventProcess(const Event &event) override {
        CAuto::EventProcess(event);
        if (event.type == EVENT_FRAME && !m_engine->GetPause()) {
            CObject *powerCell = m_object->GetPower();
            if (powerCell != nullptr) {

                if (powerCell->Implements(ObjectInterfaceType::LiquidContainer)) {
                    // test code
                    CLiquidContainerObject *asPC = dynamic_cast<CLiquidContainerObject*>(powerCell);
                    if (asPC->GetLiquidType() == LiquidType::WATER || asPC->GetLiquidType() == LiquidType::EMPTY) {
                        float water = asPC->GetLiquidAmount();

                        if (water < 1.0f)
                        {
                            // animation
                            cycle = fmodf(cycle + event.rTime, 1.0f);
                            if (cycle < 0.5f)
                                m_object->SetPartPosition(1, Math::Vector(0.0f, 4.0f * cycle + 1.5f, 0.0f));
                            else
                                m_object->SetPartPosition(1, Math::Vector(0.0f, 4.0f - (4.0f * cycle) + 1.5f, 0.0f));
                        }

                        water = water + (0.2f * event.rTime);
                        if(water > 1.0f) water = 1.0f;
                        asPC->SetLiquid(LiquidType::WATER, water);
                    }
                }
            }
        }
        return true; // XXX what does this mean?
    }
};

std::unique_ptr<CObject> CreateObjectWaterPump(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* oldModelManager, // XXX remove
    Gfx::CModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CWaterPump>(params.id);

    obj->SetTeam(params.team);

    /*static int info1_base_rank = -1;
    if (info1_base_rank == -1) {
        Gfx::CModel& model_info1 = modelManager->GetModel("info1");
        info1_base_rank = engine->CreateBaseObject();
        engine->AddBaseObjTriangles(info1_base_rank, model_info1.GetMesh("main")->GetTriangles());
    }*/

    int rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);
    //engine->SetObjectBaseRank(rank, info1_base_rank);
    oldModelManager->AddModelReference("waterpump.txt", false, rank);
    obj->SetPosition(params.pos);
    obj->SetRotationY(params.angle);
    obj->SetFloorHeight(0.0f);

    rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    obj->SetObjectRank(1, rank);
    obj->SetObjectParent(1, 0);
    //engine->SetObjectBaseRank(rank, info1_base_rank);
    oldModelManager->AddModelReference("info2.mod", false, rank);
    obj->SetPartPosition(1, Math::Vector(0.0f, 1.0f, 0.0f));
    obj->SetPartRotationY(1, 0.0f);

    obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
    obj->AddCrashSphere(CrashSphere(Math::Vector(7.0f,  1.0f, 0.0f), 1.5f, SOUND_BOUMm, 0.45f));
    obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 5.0f, 0.0f), 6.0f));
    obj->SetPowerPosition(Math::Vector(7.0, 2.5, 0.0));

    obj->CreateShadowCircle(8.0f, 1.0f);

    Math::Vector pos = obj->GetPosition();
    pos.y += params.height;
    obj->SetPosition(pos);  // to display the shadows immediately

    auto objAuto = MakeUnique<CAutoWaterPump>(obj.get());
    objAuto->Init();
    obj->SetAuto(std::move(objAuto));

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

