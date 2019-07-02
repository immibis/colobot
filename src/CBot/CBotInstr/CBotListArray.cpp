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

#include "CBot/CBotInstr/CBotListArray.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotInstr/CBotExprLitNull.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include "common/make_unique.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotListArray::CBotListArray()
{
    m_expr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotListArray::~CBotListArray()
{
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CBotListArray::Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type)
{
    CBotCStack* pStk = pStack->TokenStack(p);

    CBotToken* pp = p;

    if (IsOfType( p, ID_NULL ) || (IsOfType(p, ID_OPBLK) && IsOfType(p, ID_CLBLK)))
    {
        std::unique_ptr<CBotInstr> inst = MakeUnique<CBotExprLitNull>();
        inst->SetToken(pp);
        return pStack->Return(move(inst), pStk);            // ok with empty element
    }
    p = pp;

    std::unique_ptr<CBotListArray> inst = MakeUnique<CBotListArray>();

    if (IsOfType( p, ID_OPBLK ))
    {
        // each element takes the one after the other
        if (type.Eq( CBotTypArrayPointer ))
        {
            pStk->SetStartError(p->GetStart());
            if (nullptr == ( inst->m_expr = CBotListArray::Compile( p, pStk, type.GetTypElem() ) ))
            {
                if (pStk->IsOk())
                {
                    inst->m_expr = CBotTwoOpExpr::Compile(p, pStk);
                    if (inst->m_expr == nullptr || !pStk->GetVarType().Compare(type))  // compatible type ?
                    {
                        pStk->SetError(CBotErrBadType1, p->GetStart());
                        goto error;
                    }
                }
            }

            while (IsOfType( p, ID_COMMA ))                                     // other elements?
            {
                pStk->SetStartError(p->GetStart());

                std::unique_ptr<CBotInstr> i = nullptr;
                if (nullptr == ( i = CBotListArray::Compile(p, pStk, type.GetTypElem() ) ))
                {
                    if (pStk->IsOk())
                    {
                        i = CBotTwoOpExpr::Compile(p, pStk);
                        if (i == nullptr || !pStk->GetVarType().Compare(type))  // compatible type ?
                        {
                            pStk->SetError(CBotErrBadType1, p->GetStart());
                            goto error;
                        }
                    }
                }

                inst->m_expr->AddNext3b(move(i));

                if ( p->GetType() == ID_COMMA ) continue;
                if ( p->GetType() == ID_CLBLK ) break;

                pStk->SetError(CBotErrClosePar, p);
                goto error;
            }
        }
        else
        {
            pStk->SetStartError(p->GetStart());
            if (nullptr == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }

            CBotTypResult valType = pStk->GetVarType();
            if(valType.GetType() == CBotTypClass)
                valType.SetType(CBotTypIntrinsic);

            if (!TypeCompatible(valType, type, ID_ASS) )
            {
                pStk->SetError(CBotErrBadType1, p->GetStart());
                goto error;
            }

            while (IsOfType( p, ID_COMMA ))                                      // other elements?
            {
                pStk->SetStartError(p->GetStart());

                std::unique_ptr<CBotInstr> i = CBotTwoOpExpr::Compile(p, pStk) ;
                if (nullptr == i)
                {
                    goto error;
                }

                CBotTypResult valType = pStk->GetVarType();
                if(valType.GetType() == CBotTypClass)
                    valType.SetType(CBotTypIntrinsic);

                if (!TypeCompatible(valType, type, ID_ASS) )
                {
                    pStk->SetError(CBotErrBadType1, p->GetStart());
                    goto error;
                }
                inst->m_expr->AddNext3b(move(i));

                if (p->GetType() == ID_COMMA) continue;
                if (p->GetType() == ID_CLBLK) break;

                pStk->SetError(CBotErrClosePar, p);
                goto error;
            }
        }

        if (!IsOfType(p, ID_CLBLK) )
        {
            pStk->SetError(CBotErrClosePar, p->GetStart());
            goto error;
        }

        return pStack->Return(move(inst), pStk);
    }

error:
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotListArray::Execute(CBotStack* &pj, CBotVar* pVar)
{
    CBotStack*    pile1 = pj->AddStack();
    CBotVar* pVar2;

    CBotInstr* p = m_expr.get();

    int n = 0;

    for (; p != nullptr ; n++, p = p->GetNext3b())
    {
        if (pile1->GetState() > n) continue;

        CBotVariable *pVar2b = pVar->GetItem(n, true);

        if (pVar2b == nullptr)
        {
            pj->SetError(CBotErrOutArray, p->GetToken());
            return false;
        }
        pVar2 = pVar2b->m_value.get();
        CBotTypResult type = pVar2->GetTypResult();

        if (!p->Execute(pile1, pVar2)) return false;        // evaluate expression

        if (type.Eq(CBotTypPointer)) pVar2->SetType(type);  // keep pointer type

        pile1->IncState();
    }

    return pj->Return(pile1);
}

////////////////////////////////////////////////////////////////////////////////
void CBotListArray::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain)
    {
        CBotStack*    pile  = pj->RestoreStack(this);
        if (pile == nullptr) return;

        CBotInstr* p = m_expr.get();

        int    state = pile->GetState();

        while(state-- > 0) p = p->GetNext3b() ;

        p->RestoreState(pile, bMain);                    // size calculation //interrupted!
    }
}

std::map<std::string, CBotInstr*> CBotListArray::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_expr"] = m_expr.get();
    return links;
}

} // namespace CBot
