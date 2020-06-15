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

#include "object/object_interface_type.h"
#include <assert.h>

enum class LiquidType {
    EMPTY,
    WATER
};

/**
 * \class CLiquidContainerObject
 * \brief Interface for buckets
 */
class CLiquidContainerObject
{
protected:
    LiquidType m_liquidType = LiquidType::EMPTY;
    float m_liquidAmount = 0.0f;
public:
    explicit CLiquidContainerObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::LiquidContainer)] = true;
    }
    virtual ~CLiquidContainerObject()
    {}

    void SetLiquid(LiquidType type, float level) {
        assert(level >= 0.0f && level <= 1.0f);
        assert(type != LiquidType::EMPTY || level == 0.0f);
        m_liquidType = (level == 0.0f ? LiquidType::EMPTY : type);
        m_liquidAmount = level;
    }

    LiquidType GetLiquidType() const {return m_liquidType;}
    float GetLiquidAmount() const {return m_liquidAmount;}
};
