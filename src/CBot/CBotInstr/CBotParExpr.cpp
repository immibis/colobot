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

#include "CBot/CBotInstr/CBotParExpr.h"

#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotExprLitBool.h"
#include "CBot/CBotInstr/CBotExprLitNan.h"
#include "CBot/CBotInstr/CBotExprLitNull.h"
#include "CBot/CBotInstr/CBotExprLitNum.h"
#include "CBot/CBotInstr/CBotExprLitString.h"
#include "CBot/CBotInstr/CBotExprUnaire.h"
#include "CBot/CBotInstr/CBotExprVar.h"
#include "CBot/CBotInstr/CBotInstrCall.h"
#include "CBot/CBotInstr/CBotNew.h"
#include "CBot/CBotInstr/CBotPostIncExpr.h"
#include "CBot/CBotInstr/CBotPreIncExpr.h"

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotCStack.h"

#include "common/make_unique.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CBotParExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GetStart());

    // is it an expression in parentheses?
    if (IsOfType(p, ID_OPENPAR))
    {
        std::unique_ptr<CBotInstr> inst = CBotExpression::Compile(p, pStk);

        if (nullptr != inst)
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                return pStack->Return(move(inst), pStk);
            }
            pStk->SetError(CBotErrClosePar, p->GetStart());
        }
        return pStack->Return(nullptr, pStk);
    }

    // is this a unary operation?
    std::unique_ptr<CBotInstr> inst = CBotExprUnaire::Compile(p, pStk);
    if (inst != nullptr || !pStk->IsOk())
        return pStack->Return(move(inst), pStk);

    // is it a variable name?
    if (p->GetType() == TokenTypVar)
    {
        // this may be a method call without the "this." before
        inst =  CBotExprVar::CompileMethode(p, pStk);
        if (inst != nullptr) return pStack->Return(move(inst), pStk);


        // is it a procedure call?
        inst =  CBotInstrCall::Compile(p, pStk);
        if (inst != nullptr || !pStk->IsOk())
            return pStack->Return(move(inst), pStk);


        CBotToken* pvar = p;
        // no, it an "ordinaty" variable
        inst =  CBotExprVar::Compile(p, pStk);

        CBotToken* pp = p;
        // post incremented or decremented?
        if (IsOfType(p, ID_INC, ID_DEC))
        {
            // recompile the variable for read-only
            p = pvar;
            inst = CBotExprVar::Compile(p, pStk, true);
            // TODO: is there a possibility that something like ((((i++)++)++)++)... will recurse exponentially?
            // since each X++ parses X twice (once with bCheckReadOnly=false, once with true)
            if (pStk->GetVarType().GetType() >= CBotTypBoolean || pStk->GetVarType().GetType() == CBotTypVoid)
            {
                pStk->SetError(CBotErrBadType1, pp);
                return pStack->Return(nullptr, pStk);
            }
            p = p->GetNext();

            // TODO: use unique_ptr here
            std::unique_ptr<CBotPostIncExpr> i = MakeUnique<CBotPostIncExpr>();
            i->SetToken(pp);
            i->m_instr = move(inst);    // associated statement
            return pStack->Return(move(i), pStk);
        }
        return pStack->Return(move(inst), pStk);
    }

    // pre increpemted or pre decremented?
    CBotToken* pp = p;
    if (IsOfType(p, ID_INC, ID_DEC))
    {
        if (p->GetType() == TokenTypVar)
        {
            if (nullptr != (inst = CBotExprVar::Compile(p, pStk, true)))
            {
                if (pStk->GetVarType().GetType() < CBotTypBoolean && pStk->GetVarType().GetType() != CBotTypVoid) // a number ?
                {
                    std::unique_ptr<CBotPreIncExpr> i = MakeUnique<CBotPreIncExpr>();
                    i->SetToken(pp);
                    i->m_instr = move(inst);
                    return pStack->Return(move(i), pStk);
                }
            }
        }
        pStk->SetError(CBotErrBadType1, pp);
        return pStack->Return(nullptr, pStk);
    }

    pStack->Return(nullptr, pStk);
    return CBotParExpr::CompileLitExpr(p, pStack);
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CBotParExpr::CompileLitExpr(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    CBotToken* pp = p;

    // is this a unary operation?
    std::unique_ptr<CBotInstr> inst = CBotExprUnaire::Compile(p, pStk, true);
    if (inst != nullptr || !pStk->IsOk())
        return pStack->Return(move(inst), pStk);

    // is it a number or DefineNum?
    if (p->GetType() == TokenTypNum ||
        p->GetType() == TokenTypDef )
    {
        inst = CBotExprLitNum::Compile(p, pStk);
        return pStack->Return(move(inst), pStk);
    }

    // is this a chaine?
    if (p->GetType() == TokenTypString)
    {
        inst = CBotExprLitString::Compile(p, pStk);
        return pStack->Return(move(inst), pStk);
    }

    // is a "true" or "false"
    if (p->GetType() == ID_TRUE ||
        p->GetType() == ID_FALSE )
    {
        inst = CBotExprLitBool::Compile(p, pStk);
        return pStack->Return(move(inst), pStk);
    }

    // is an object to be created with new
    if (p->GetType() == ID_NEW)
    {
        inst = CBotNew::Compile(p, pStk);
        return pStack->Return(move(inst), pStk);
    }

    // is a null pointer
    if (IsOfType(p, ID_NULL))
    {
        inst = MakeUnique<CBotExprLitNull>();
        inst->SetToken(pp);
        pStk->SetVarType(CBotTypNullPointer);
        return pStack->Return(move(inst), pStk);
    }

    // is a number nan
    if (IsOfType(p, ID_NAN))
    {
        inst = MakeUnique<CBotExprLitNan>();
        inst->SetToken(pp);
        pStk->SetVarType(CBotTypInt);
        return pStack->Return(move(inst), pStk);
    }


    return pStack->Return(nullptr, pStk);
}

} // namespace CBot
