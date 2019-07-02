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

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotToken.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotTypResult.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CompileParams(CBotToken* &p, CBotCStack* pStack, std::vector<CBotTypResult> &ppVars)
{
    bool        first = true;
    std::unique_ptr<CBotInstr> ret = nullptr;   // to return to the list

    CBotCStack*    pile = pStack;

    ppVars.clear();

    if (IsOfType(p, ID_OPENPAR))
    {
        int    start, end;
        if (!IsOfType(p, ID_CLOSEPAR)) while (true)
        {
            start = p->GetStart();
            pile = pile->TokenStack();  // keeps the result on the stack

            if (first) pStack->SetStartError(start);
            first = false;

            std::unique_ptr<CBotInstr> param = CBotExpression::Compile(p, pile);
            end      = p->GetStart();

            if (!pile->IsOk())
            {
                return pStack->Return(nullptr, pile);
            }

            if (param != nullptr)
            {
                if (pile->GetVarType().Eq(CBotTypVoid))
                {
                    pStack->DeleteChildLevels();
                    pStack->SetError(CBotErrVoid, p->GetStart());
                    return nullptr;
                }
                ppVars.emplace_back(pile->GetVarType());
                // TODO: was commented out as part of value/variable refactor. What does this do?
                (void)end; //ppVars[i]->GetToken()->SetPos(start, end);

                if (ret == nullptr) ret = move(param);
                else ret->AddNext(move(param));   // construct the list

                if (IsOfType(p, ID_COMMA)) continue;    // skips the comma
                if (IsOfType(p, ID_CLOSEPAR)) break;
            }

            pStack->SetError(CBotErrClosePar, p->GetStart());
            pStack->DeleteChildLevels();
            return nullptr;
        }
    }
    return    ret;
}

////////////////////////////////////////////////////////////////////////////////
bool TypeCompatible(CBotTypResult& type1, CBotTypResult& type2, int op)
{
    int    t1 = type1.GetType();
    int    t2 = type2.GetType();

    // void used to be 99 here due to special handling in CBotCStack; now it's 0
    if (t1 == CBotTypVoid || t2 == CBotTypVoid)
        return false; // result is void?

    int max = (t1 > t2) ? t1 : t2;

    // special case for strin concatenation
    if (op == ID_ADD && t1 == CBotTypString) return true;
    if (op == ID_ASSADD && t2 == CBotTypString) return true;
    if (op == ID_ASS && t2 == CBotTypString) return true;

    if (max >= CBotTypBoolean)
    {
        if ( (op == ID_EQ || op == ID_NE) &&
             (t1 == CBotTypPointer && t2 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE || op == ID_ASS) &&
             (t2 == CBotTypPointer && t1 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE) &&
             (t1 == CBotTypArrayPointer && t2 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE || op == ID_ASS) &&
             (t2 == CBotTypArrayPointer && t1 == CBotTypNullPointer)) return true;
        if (t2 != t1) return false;
        if (t1 == CBotTypArrayPointer) return type1.Compare(type2);
        if (t1 == CBotTypPointer ||
            t1 == CBotTypClass   ||
            t1 == CBotTypIntrinsic )
        {
            CBotClass*    c1 = type1.GetClass();
            CBotClass*    c2 = type2.GetClass();

            return c1->IsChildOf(c2) || c2->IsChildOf(c1);
            // accept the case in reverse
            // the transaction will be denied at runtime if the pointer is not
            // compatible
        }

        return true;
    }

    type1.SetType(max);
    type2.SetType(max);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool TypesCompatibles(const CBotTypResult& type1, const CBotTypResult& type2)
{
    int    t1 = type1.GetType();
    int    t2 = type2.GetType();

    if (t1 == CBotTypIntrinsic) t1 = CBotTypClass;
    if (t2 == CBotTypIntrinsic) t2 = CBotTypClass;

    // This function used to get 99 for 'void', now it gets CBotTypVoid
    if (t1 == CBotTypVoid || t2 == CBotTypVoid)
        return false; // result is void?

    int max = (t1 > t2) ? t1 : t2;

    if (max >= CBotTypBoolean)
    {
        if (t1 == CBotTypPointer && t2 == CBotTypNullPointer) return true;
        if (t2 != t1) return false;

        if (max == CBotTypPointer)
        {
            CBotClass*    c1 = type1.GetClass();
            CBotClass*    c2 = type2.GetClass();
            return c2->IsChildOf(c1);
        }

        if (max == CBotTypArrayPointer)
            return TypesCompatibles(type1.GetTypElem(), type2.GetTypElem());

        if (max == CBotTypClass)
            return type1.GetClass() == type2.GetClass() ;

        return true ;
    }
    return true;
}

} // namespace CBot
