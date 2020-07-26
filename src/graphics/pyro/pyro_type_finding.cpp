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

#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "graphics/pyro/pyro.h"
#include "graphics/pyro/pyro_manager.h"

#include "object/object.h"

#include "sound/sound.h"

using namespace Gfx;

void CFindingPyro::AfterCreate()
{
    float limit = (m_size-1.0f)/4.0f;
    if (limit > 8.0f) limit = 8.0f;
    if (m_object->GetType() == OBJECT_APOLLO2) limit = 2.0f;
    m_speed = 1.0f/limit;

    LightOperAdd(0.00f, 1.0f,  4.0f,  4.0f,  2.0f);  // yellow
    LightOperAdd(1.00f, 0.0f,  4.0f,  4.0f,  2.0f);  // yellow
    CreateLight(m_pos, 40.0f);
}

void CFindingPyro::UpdateEffect()
{
    if ( m_object != nullptr &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        float factor = m_size*0.3f;
        if (m_object->GetType() == OBJECT_SAFE) factor *= 1.3f;
        if (factor > 40.0f) factor = 40.0f;
        Math::Vector pos = m_pos;
        m_terrain->AdjustToFloor(pos);
        pos.x += (Math::Rand()-0.5f)*factor;
        pos.z += (Math::Rand()-0.5f)*factor;
        Math::Vector speed;
        speed.x = (Math::Rand()-0.5f)*2.0f;
        speed.z = (Math::Rand()-0.5f)*2.0f;
        speed.y = 4.0f+Math::Rand()*4.0f;
        Math::Point dim;
        dim.x = (Math::Rand()*3.0f+3.0f)*(1.0f-m_progress*0.9f);
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f, 0.5f);
    }
}
