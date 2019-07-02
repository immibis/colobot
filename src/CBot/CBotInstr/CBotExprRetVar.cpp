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

#include <sstream>
#include "CBot/CBotInstr/CBotExprRetVar.h"

#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotInstrMethode.h"
#include "CBot/CBotInstr/CBotIndexExpr.h"
#include "CBot/CBotInstr/CBotFieldExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotClass.h"

#include "common/make_unique.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprRetVar::CBotExprRetVar()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotExprRetVar::~CBotExprRetVar()
{
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CBotExprRetVar::Compile(CBotToken*& p, CBotCStack* pStack, bool bMethodsOnly, bool bPreviousIsSuper)
{
    if (p->GetType() == ID_DOT)
    {
        CBotTypResult val = pStack->GetVarType();

        if (val.GetType() == CBotTypVoid) return nullptr;

        CBotCStack* pStk = pStack->TokenStack();
        std::unique_ptr<CBotInstr> inst = MakeUnique<CBotExprRetVar>();

        while (true)
        {
            pStk->SetStartError(p->GetStart());
            if (val.GetType() == CBotTypArrayPointer)
            {
                if (bMethodsOnly) goto err;

                if (IsOfType( p, ID_OPBRK ))
                {
                    std::unique_ptr<CBotIndexExpr> i = MakeUnique<CBotIndexExpr>();
                    i->m_expr = CBotExpression::Compile(p, pStk);

                    val = val.GetTypElem();

                    if (i->m_expr == nullptr || pStk->GetVarType().GetType() != CBotTypInt)
                    {
                        pStk->SetError(CBotErrBadIndex, p->GetStart());
                        goto err;
                    }
                    if (!pStk->IsOk() || !IsOfType( p, ID_CLBRK ))
                    {
                        pStk->SetError(CBotErrCloseIndex, p->GetStart());
                        goto err;
                    }
                    inst->AddNext3(move(i));
                    continue;
                }
            }
            if (val.GetType() == CBotTypClass || val.GetType() == CBotTypPointer)
            {
                if (IsOfType(p, ID_DOT))
                {
                    CBotToken* pp = p;

                    if (p->GetType() == TokenTypVar)
                    {
                        if (p->GetNext()->GetType() == ID_OPENPAR)
                        {
                            std::unique_ptr<CBotInstr> i = CBotInstrMethode::Compile(p, pStk, val, bMethodsOnly, false);
                            if (!pStk->IsOk()) goto err;
                            inst->AddNext3(move(i));
                            return pStack->Return(move(inst), pStk);
                        }
                        else if (bMethodsOnly)
                        {
                            p = p->GetPrev();
                            goto err;
                        }
                        else
                        {
                            CBotTypResult preVal = val;
                            CBotVariable *var = val.GetClass()->GetItem(p->GetString());

                            if (var != nullptr)
                            {
                                val = var->m_value->GetTypResult();
                                std::unique_ptr<CBotFieldExpr> i = MakeUnique<CBotFieldExpr>(var->GetFieldPosition());
                                i->SetToken(pp);
                                inst->AddNext3(move(i));
                                // TODO: can "super" accesses occur here? Check for them. (3rd parameter)
                                if (CBotFieldExpr::CheckProtectionError(pStk, preVal, "", var))
                                {
                                    pStk->SetError(CBotErrPrivate, pp);
                                    goto err;
                                }
                            }
                            else
                            {
                                val = CBotTypResult(CBotTypVoid); // indicate error
                            }
                        }

                        if (val.GetType() != CBotTypVoid)
                        {
                            p = p->GetNext();
                            continue;
                        }
                        pStk->SetError(CBotErrUndefItem, p);
                        goto err;
                    }
                    pStk->SetError(CBotErrUndefClass, p);
                    goto err;
                }
            }
            break;
        }

        pStk->SetVarType(val);
        if (pStk->IsOk()) return pStack->Return(move(inst), pStk);

        pStk->SetError(CBotErrUndefVar, p);
err:
        return pStack->Return(nullptr, pStk);
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprRetVar::Execute(CBotStack* &pj)
{

    CBotStack* pile = pj->AddStack();
    CBotStack* pile1 = pile;
    CBotVar* pVar;

    if (pile1->GetState() == 0)
    {
        pVar = pj->GetVar();
        pVar->Update(pj->GetUserPtr());
        if (pVar->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) == CBotTypNullPointer)
        {
            pile1->SetError(CBotErrNull, &m_token);
            return pj->Return(pile1);
        }

        if ( !m_next3->ExecuteVar(pVar, pile, &m_token, true, false) )
            return false;

        if (pVar)
            pile1->SetCopyVar(pVar);
        else
            return pj->Return(pile1);

        pile1->IncState();
    }
    pVar = pile1->GetVar();

    if (pVar == nullptr)
    {
        return pj->Return(pile1);
    }

    if (pVar->IsUndefined())
    {
        pile1->SetError(CBotErrNotInit, &m_token);
        return pj->Return(pile1);
    }
    return pj->Return(pile1);
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprRetVar::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack* pile = pj->RestoreStack();
    if ( pile == nullptr ) return;

    if (pile->GetState() == 0)
        m_next3->RestoreStateVar(pile, bMain);
}

std::string CBotExprRetVar::GetDebugData()
{
    std::stringstream ss;
    ss << m_token.GetString() << "func(...).something" << std::endl;
    return ss.str();
}

} // namespace CBot
