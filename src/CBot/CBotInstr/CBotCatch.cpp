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

#include "CBot/CBotInstr/CBotCatch.h"
#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"
#include "common/make_unique.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotCatch::CBotCatch()
{
    m_cond = nullptr;
    m_block = nullptr;
    m_next = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotCatch::~CBotCatch()
{
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotCatch> CBotCatch::Compile(CBotToken* &p, CBotCStack* pStack)
{
    std::unique_ptr<CBotCatch> inst = MakeUnique<CBotCatch>(); // creates the object
    pStack->SetStartError(p->GetStart());

    inst->SetToken(p);
    if (!IsOfType(p, ID_CATCH)) return nullptr;    // should never happen

    if (IsOfType(p, ID_OPENPAR))
    {
        inst->m_cond = CBotExpression::Compile(p, pStack);
        if (( pStack->GetVarType().GetType() < CBotTypLong ||
              pStack->GetVarType().Eq(CBotTypBoolean) )&& pStack->IsOk()
            && pStack->GetVarType().GetType() != CBotTypVoid)
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                inst->m_block = CBotBlock::CompileBlkOrInst(p, pStack );
                if ( pStack->IsOk() )
                    return inst;                // return an object to the application
            }
            pStack->SetError(CBotErrClosePar, p->GetStart());
            // TODO does this return the right error message?
        }
        pStack->SetError(CBotErrBadType1, p->GetStart());
        // TODO does this return the right error message?
    }
    pStack->SetError(CBotErrOpenPar, p->GetStart());
    return nullptr;                                // no object, the error is on the stack
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCatch :: Execute(CBotStack* &pj)
{
    if (m_block == nullptr ) return true;
    return m_block->Execute(pj);                // executes the associated block
}

////////////////////////////////////////////////////////////////////////////////
void CBotCatch :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( bMain && m_block != nullptr ) m_block->RestoreState(pj, bMain);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCatch :: RestoreCondState(CBotStack* &pj, bool bMain)
{
    m_cond->RestoreState(pj, bMain);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCatch :: TestCatch(CBotStack* &pile, int val)
{
    if ( !m_cond->Execute(pile) ) return false;

    if ( val > 0 || pile->GetVar() == nullptr || pile->GetVar()->GetType() != CBotTypBoolean )
    {
        std::unique_ptr<CBotVar> var = CBotVar::Create(CBotTypBoolean);
        var->SetValInt( pile->GetVal() == val );
        pile->SetVar(std::move(var));                          // calls on the stack
    }

    return true;
}

std::map<std::string, CBotInstr*> CBotCatch::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_block"] = m_block.get();
    links["m_cond"] = m_cond.get();
    links["m_next"] = m_next.get();
    return links;
}

} // namespace CBot
