/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/object.h"
#include "object/old_object.h"
#include "object/interface/liquid_container_object.h"

#include "object/object_factory.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/oldmodelmanager.h"

#include "common/make_unique.h"

#include "level/parser/parserexceptions.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "object/object_create_params.h"

#include <boost/lexical_cast.hpp>

class CBucket
     : public COldObject
     , public CLiquidContainerObject
{
    LiquidType m_displayedLiquidType = LiquidType::EMPTY;
    float m_displayedLiquidAmount = -100; // force initial update
public:
    CBucket(int id)
        : COldObject(id)
        , CLiquidContainerObject(m_implementedInterfaces)
    {
        SetType(OBJECT_BUCKET);
        m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Transportable)] = true;
    }

    bool EventProcess(const Event& event) override {
        if (!COldObject::EventProcess(event))
            return false;

        if (event.type == EVENT_FRAME && (m_displayedLiquidType != m_liquidType || !Math::IsEqual(m_displayedLiquidAmount, m_liquidAmount, 0.003f))) {
            UpdateTextureMapping();
            m_displayedLiquidType = m_liquidType;
            m_displayedLiquidAmount = m_liquidAmount;
        }

        return true;
    }

private:
    void UpdateTextureMapping() {
        Gfx::Material mat;
        mat.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f);  // white
        mat.ambient = Gfx::Color(0.5f, 0.5f, 0.5f);

        int type = static_cast<int>(m_liquidType) - 1;
        if(type > 0) type--; // don't use a texture slot for EMPTY liquid type, just reuse whatever liquid comes first
        float vcoord = (type + 0.5f) / 128.0f;

        // Y coordinate ranges from 0 to 2.
        // If m_liquidAmount is 0, u coordinate ranges from 0 to 0.5.
        // If m_liquidAmount is 1, u coordinate ranges from 0.5 to 1.
        // In any case, v is a constant depending on the liquid type.
        // The highest u coordinate needs to be at the lowest Y coordinate.

        m_engine->ChangeTextureMapping(m_objectPart[0].object,
                                       mat, Gfx::ENG_RSTATE_PART3, "objects/liquid.png", "",
                                       Gfx::ENG_TEX_MAPPING_X,
                                       0.0f, vcoord, -0.25f, 0.5f + 0.5f*m_liquidAmount);
    }
};

std::unique_ptr<CObject> CreateObjectBucket(const ObjectCreateParams &params, Gfx::COldModelManager *modelManager, Gfx::CEngine *graphicsEngine)
{
    std::unique_ptr<CBucket> newObj = MakeUnique<CBucket>(params.id);

    int rootGraphObj = graphicsEngine->CreateObject();
    graphicsEngine->SetObjectType(rootGraphObj, Gfx::ENG_OBJTYPE_FIX); // XXX: why is ENG_OBJTYPE_FIX used for movable objects? (this is the same as for powercells, titanium, etc)
    newObj->SetObjectRank(0, rootGraphObj);
    modelManager->AddModelCopy("bucket.txt", false, rootGraphObj);

    newObj->SetPosition(params.pos);
    newObj->SetRotationY(params.angle);

    // standard settings for transportable objects
    newObj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f));
    newObj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.5f));
    newObj->CreateShadowCircle(1.5f, 1.0f);
    newObj->SetFloorHeight(0.0f);
    newObj->FloorAdjust();

    graphicsEngine->LoadAllTextures();

    return newObj;
}
