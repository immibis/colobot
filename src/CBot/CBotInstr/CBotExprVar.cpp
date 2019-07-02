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
#include "CBot/CBotInstr/CBotExprVar.h"
#include "CBot/CBotInstr/CBotInstrMethode.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotIndexExpr.h"
#include "CBot/CBotInstr/CBotFieldExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotVar/CBotVarArray.h"

#include "common/make_unique.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprVar::CBotExprVar()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotExprVar::~CBotExprVar()
{
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CBotExprVar::Compile(CBotToken*& p, CBotCStack* pStack, bool bCheckReadOnly)
{
//    CBotToken*    pDebut = p;
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GetStart());

    // is it a variable?
    if (p->GetType() == TokenTypVar)
    {
        std::unique_ptr<CBotInstr> inst = MakeUnique<CBotExprVar>(); // create the object

        inst->SetToken(p);

        CBotVariable*     var;

        if (nullptr != (var = pStk->FindVar(p)))   // seek if known variable
        {
            if (var->GetContainingClass() != nullptr) // implicit "this" access
            {
                if (CBotFieldExpr::CheckProtectionError(pStk, CBotTypVoid, "", var, bCheckReadOnly))
                {
                    pStk->SetError(CBotErrPrivate, p);
                    goto err;
                }

                // This is an element of the current class
                // ads the equivalent of this. before
                CBotToken token("this");
                // invisible 'this.' highlights member token on error
                token.SetPos(p->GetStart(), p->GetEnd());
                inst->SetToken(&token);

                std::unique_ptr<CBotFieldExpr> i = MakeUnique<CBotFieldExpr>(var->GetFieldPosition());     // new element
                i->SetToken(p);     // keeps the name of the token
                inst->AddNext3(move(i)); // added after
            }

            p = p->GetNext();   // next token

            while (true)
            {
                if (var->m_value->GetType() == CBotTypArrayPointer)
                {
                    if (IsOfType( p, ID_OPBRK ))    // check if there is an aindex
                    {
                        std::unique_ptr<CBotIndexExpr> i = MakeUnique<CBotIndexExpr>();
                        i->m_expr = CBotExpression::Compile(p, pStk);   // compile the formula

                        var = (static_cast<CBotVarArray*>(var->m_value.get()))->GetItem(0,true);    // gets the component [0]

                        if (i->m_expr == nullptr)
                        {
                            pStk->SetError(CBotErrBadIndex, p->GetStart());
                            goto err;
                        }
                        if (!pStk->IsOk() || !IsOfType( p, ID_CLBRK ))
                        {
                            pStk->SetError(CBotErrCloseIndex, p->GetStart());
                            goto err;
                        }
                        inst->AddNext3(move(i));  // add to the chain
                        continue;
                    }
                }
                if (var->m_value->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) == CBotTypPointer)  // for classes
                {
                    if (IsOfType(p, ID_DOT))
                    {
                        CBotToken* pp = p;

                        if (p->GetType() == TokenTypVar)    // must be a name
                        {
                            if (p->GetNext()->GetType() == ID_OPENPAR)  // a method call?
                            {
                                if (bCheckReadOnly) goto err; // don't allow increment a method call "++"

                                std::unique_ptr<CBotInstr> i = CBotInstrMethode::Compile(p, pStk, var->m_value->GetTypResult(), false,
                                        var->GetContainingClass() == nullptr && var->GetName() == "super");
                                if (!pStk->IsOk()) goto err;
                                inst->AddNext3(move(i));  // added after
                                return pStack->Return(move(inst), pStk);
                            }
                            else
                            {
                                CBotVariable*   preVar = var;
                                var = var->m_value->GetItem(p->GetString());         // get item correspondent
                                if (var != nullptr)
                                {
                                    std::unique_ptr<CBotFieldExpr> i = MakeUnique<CBotFieldExpr>(var->GetFieldPosition());     // new element
                                    i->SetToken(pp);                            // keeps the name of the token
                                    inst->AddNext3(move(i));                    // add after
                                    if (CBotFieldExpr::CheckProtectionError(pStk, preVar->m_value->GetTypResult(), preVar->GetName(), var, bCheckReadOnly))
                                    {
                                        pStk->SetError(CBotErrPrivate, pp);
                                        goto err;
                                    }
                                }
                            }


                            if (var != nullptr)
                            {
                                p = p->GetNext();   // skips the name
                                continue;
                            }
                            pStk->SetError(CBotErrUndefItem, p);
                            goto err;
                        }
                        pStk->SetError(CBotErrUndefClass, p->GetStart());
                        goto err;
                    }
                }

                break;
            }

            pStk->SetVarType(var->m_value->GetTypResult());
            if (pStk->IsOk()) return pStack->Return(move(inst), pStk);
        }
        pStk->SetError(CBotErrUndefVar, p);
err:
        return pStack->Return(nullptr, pStk);
    }

    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CBotExprVar::CompileMethode(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*    pp = p;
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(pp->GetStart());

    // is it a variable ?
    if (pp->GetType() == TokenTypVar)
    {
        CBotToken pthis("this");
        CBotVariable* var = pStk->FindVar(pthis);
        if (var == nullptr) return pStack->Return(nullptr, pStk);

        std::unique_ptr<CBotInstr> inst = MakeUnique<CBotExprVar>();

        // this is an element of the current class
        // adds the equivalent of this. before

        // invisible 'this.' highlights member token on error
        pthis.SetPos(pp->GetStart(), pp->GetEnd());
        inst->SetToken(&pthis);

        CBotToken* pp = p;

        if (pp->GetType() == TokenTypVar)
        {
            if (pp->GetNext()->GetType() == ID_OPENPAR)        // a method call?
            {
                std::unique_ptr<CBotInstr> i = CBotInstrMethode::Compile(pp, pStk, var->m_value->GetTypResult(), false,
                        var->GetContainingClass() == nullptr && var->GetName() == "super");
                if (pStk->IsOk())
                {
                    inst->AddNext3(move(i));                       // add after
                    p = pp;                                        // previous instruction
                    return pStack->Return(move(inst), pStk);
                }
                pStk->SetError(CBotNoErr, 0);                            // the error is not adressed here
            }
        }
    }
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprVar::Execute(CBotStack* &pj)
{
    CBotVar*     pVar = nullptr;
    CBotStack*     pile  = pj->AddStack(this);

    CBotStack*     pile1 = pile;

    if (pile1->GetState() == 0)
    {
        if (!ExecuteVar(pVar, pile, nullptr, true)) return false;        // Get the variable fields and indexes according

        if (pVar) pile1->SetCopyVar(pVar);                            // place a copy on the stack
        else
        {
            return pj->Return(pile1);
        }
        pile1->IncState();
    }

    pVar = pile1->GetVar();

    if (pVar == nullptr)
    {
        return pj->Return(pile1);
    }

    if (pVar->IsUndefined())
    {
        CBotToken* pt = &m_token;
        while (pt->GetNext() != nullptr) pt = pt->GetNext();
        pile1->SetError(CBotErrNotInit, pt);
        return pj->Return(pile1);
    }
    return pj->Return(pile1);   // operation completed
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprVar::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*     pile  = pj->RestoreStack(this);
    if (pile == nullptr) return;

    CBotStack*     pile1 = pile;

    if (pile1->GetState() == 0)
    {
        RestoreStateVar(pile, bMain);   // retrieves the variable fields and indexes according
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprVar::ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep)
{
    CBotStack*    pile = pj;
    pj = pj->AddStack(this);

    if (bStep && pj->IfStep()) return false;

    CBotVariable *pRealVar = pj->FindVar(m_token, true);     // tries with the variable update if necessary
    pVar = (pRealVar ? pRealVar->m_value.get() : nullptr);
    if (pVar == nullptr)
    {
        assert(false);
        //pj->SetError(static_cast<CBotError>(1), &m_token); // TODO: yeah, don't care that this exception doesn't exist ~krzys_h
        return false;
    }
    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pj, &m_token, bStep, false) )
            return false;   // field of an instance, table, methode

    return pile->ReturnKeep(pj);    // does not put on stack but get the result if a method was called
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprVar::RestoreStateVar(CBotStack* &pj, bool bMain)
{
    pj = pj->RestoreStack(this);
    if (pj == nullptr) return;

    if (m_next3 != nullptr)
         m_next3->RestoreStateVar(pj, bMain);
}

std::string CBotExprVar::GetDebugData()
{
    std::stringstream ss;
    ss << m_token.GetString() << std::endl;
    //ss << "VarID = " << m_nIdent;
    return ss.str();
}

} // namespace CBot
