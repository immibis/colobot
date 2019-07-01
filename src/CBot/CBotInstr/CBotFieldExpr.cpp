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

#include "CBot/CBotInstr/CBotFieldExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotVar/CBotVarClass.h"

#include <cassert>
#include <sstream>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotFieldExpr::CBotFieldExpr(int nFieldPosition)
{
    m_nFieldPosition = nFieldPosition;
}

////////////////////////////////////////////////////////////////////////////////
CBotFieldExpr::~CBotFieldExpr()
{
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFieldExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    if (pVar->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) != CBotTypPointer)
        assert(0);

    CBotVarClass *asObject = pVar->AsObject();
    if (asObject == nullptr)
    {
        // TODO: what's the right error code if the thing to the left isn't an object?
        // should be an internal error since the type checker should've caught it
        assert(false);
        pile->SetError(CBotErrUndefItem, &m_token);
        return false;
    }

    CBotVariable *pNextVar = asObject->GetObjectField(m_nFieldPosition);
    pVar = pNextVar ? pNextVar->m_value.get() : nullptr;
    if (pVar == nullptr)
    {
        pile->SetError(CBotErrUndefItem, &m_token);
        return false;
    }

    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pile) ) return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFieldExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend)
{
    CBotStack*    pj = pile;
    pile = pile->AddStack(this);    // changes in output stack


    if (pVar->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) != CBotTypPointer)
        assert(0);

    CBotVarClass* pItem = pVar->GetPointer();
    if (pItem == nullptr)
    {
        pile->SetError(CBotErrNull, prevToken);
        return pj->Return(pile);
    }
    if (pItem->GetUserPtr() == OBJECTDELETED)
    {
        pile->SetError(CBotErrDeletedPtr, prevToken);
        return pj->Return(pile);
    }

    if (bStep && pile->IfStep()) return false;

    CBotVarClass *asObject = pVar->AsObject();
    if (asObject == nullptr)
    {
        // TODO: what's the right error code if the thing to the left isn't an object?
        // should be an internal error since the type checker should've caught it
        assert(false);
        pile->SetError(CBotErrUndefItem, &m_token);
        return false;
    }

    CBotVariable *pVar2 = asObject->GetObjectField(m_nFieldPosition);
    pVar = (pVar2 ? pVar2->m_value.get() : nullptr);
    if (pVar == nullptr)
    {
        pile->SetError(CBotErrUndefItem, &m_token);
        return pj->Return(pile);
    }

    if (pVar2->IsStatic())
    {
        // for a static variable, takes it in the class itself
        CBotClass* pClass = pItem->GetClass();
        pVar = pClass->GetItem(m_token.GetString())->m_value.get();
    }

    // request the update of the element, if applicable
    pVar->Update(pile->GetUserPtr());

    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pile, &m_token, bStep, bExtend) ) return false;

    // does not release the stack
    // to maintain the state SetState () corresponding to step

    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotFieldExpr::RestoreStateVar(CBotStack* &pj, bool bMain)
{
    pj = pj->RestoreStack(this);
    if (pj == nullptr) return;

    if (m_next3 != nullptr)
         m_next3->RestoreStateVar(pj, bMain);
}

std::string CBotFieldExpr::GetDebugData()
{
    std::stringstream ss;
    ss << "FieldPosition = " << m_nFieldPosition;
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFieldExpr::CheckProtectionError(CBotCStack* pStack, CBotTypResult prevType, const std::string &prevName, CBotVariable* pVar, bool bCheckReadOnly)
{
    CBotVariable::ProtectionLevel varPriv = pVar->GetPrivate();

    if (bCheckReadOnly && varPriv == CBotVariable::ProtectionLevel::ReadOnly)
        return true;

    if (varPriv == CBotVariable::ProtectionLevel::Public) return false;

    // TODO: can we refactor this?

    // implicit 'this.' var (prevType is void meaning no value),  this.var,  or super.var
    if (prevType.GetType() == CBotTypVoid || prevName == "this" || prevName == "super") // member of the current class
    {
        if (varPriv == CBotVariable::ProtectionLevel::Private)  // var is private ?
        {
            CBotToken  token("this");
            CBotVar*   pThis = pStack->FindVar(token)->m_value.get();

            // Theoretically can't happen; to be in this code path we need to be dereferencing a field on "this" or "super"
            // (or implicitly "this") which means there must be a "this" variable.
            // TODO: Can the user declare their own variable called "super"? Will that break this assumption?
            // TODO: There should be a better way to get the current class, than looking for variables called "this"
            assert(pThis != nullptr);

            CBotClass* pClass = pThis->GetClass();         // the current class
            if (pVar->GetContainingClass() != pClass)
                return true;
        }
    }
    else                                                                // any other context
    {
        if (pVar->IsPrivate())    // var is protected or private ?
        {
            CBotToken token("this");
            CBotVariable*  pThis = pStack->FindVar(token);

            if (pThis == nullptr) return true;                   // inside a function ?
            if (pThis->m_value->GetType() != CBotTypPointer) return true;

            CBotClass* pClass = pThis->m_value->GetClass();               // the current class

            // TODO: Is this rule for 'protected' accurate? See how it works in Java for example.
            if (!pClass->IsChildOf(prevType.GetClass()))     // var is member of some other class ?
                return true;

            if (varPriv == CBotVariable::ProtectionLevel::Private &&  // private member of a parent class
                pClass != prevType.GetClass()) return true;
        }
    }

    return false;
}

} // namespace CBot
