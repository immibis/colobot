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

#include "CBot/CBotInstr/CBotBoolExpr.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotCStack.h"

#include "CBot/CBotDefines.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CBotBoolExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GetStart());

    std::unique_ptr<CBotInstr> inst = CBotTwoOpExpr::Compile(p, pStack);

    if (nullptr != inst)
    {
        if (pStack->GetVarType().Eq(CBotTypBoolean))
        {
            return inst;
        }
        pStack->SetError(CBotErrNotBoolean, p->GetStart());    // is not a boolean
    }

    return nullptr;
}

} // namespace CBot
