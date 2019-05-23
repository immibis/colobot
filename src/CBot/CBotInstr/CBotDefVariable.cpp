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

#include "CBot/CBotInstr/CBotDefVariable.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"
#include "CBot/CBotInstr/CBotLeftExprVar.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"
#include "CBot/CBotInstr/CBotDefArray.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotDefVariable::CBotDefVariable()
{
    m_var = m_expr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotDefVariable::~CBotDefVariable()
{
    delete m_var;
    delete m_expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotDefVariable::CompileAfterType(CBotToken* &p, CBotCStack* pStack, CBotTypResult &baseType, bool noskip)
{
    // TODO: if type is a CBotTypArrayPointer then we should make sure this behaves like CBotInstr::CompileArray did
    // and delete CBotInstr::CompileArray

    CBotCStack* pStk = pStack->TokenStack(p);

    CBotToken *vartoken = p; // so we can rewind if it's an array

    // TODO: adjust return type so cast isn't needed
    CBotLeftExprVar *left_var = static_cast<CBotLeftExprVar*>(CBotLeftExprVar::Compile(p, pStk));
    if (left_var == nullptr)
    {
        return pStack->Return(nullptr, pStk);
    }

    CBotInstr *inst;
    if (IsOfType(p,  ID_OPBRK) || baseType.GetType() == CBotTypArrayPointer)
    {
        p = vartoken; // return to the variable name and compile as an array declaration instead
        inst = CBotDefArray::Compile(p, pStk, baseType);
    }
    else
    {
        left_var->m_typevar = baseType;

        if (pStk->CheckVarLocal(vartoken)) // check for redefinition
        {
            pStk->SetError(CBotErrRedefVar, vartoken);
            return pStack->Return(nullptr, pStk);
        }

        CBotDefVariable *inst_defvar = new CBotDefVariable();
        inst_defvar->m_var = left_var;
        inst_defvar->SetToken(vartoken);

        if (IsOfType(p,  ID_ASS))
        {
            pStk->SetStartError(p->GetStart());
            if ( IsOfType(p, ID_SEP) )
            {
                pStk->SetError(CBotErrNoExpression, p->GetStart());
                delete inst_defvar;
                return pStack->Return(nullptr, pStk);
            }

            inst_defvar->m_expr = CBotTwoOpExpr::Compile( p, pStk );
            if (nullptr == inst_defvar->m_expr)
            {
                delete inst_defvar;
                return pStack->Return(nullptr, pStk);
            }

            CBotTypResult valueType = pStk->GetTypResult(); // TODO: make TypeCompatible take a const& so this variable isn't needed
            if (!TypeCompatible(valueType, baseType, ID_ASS)) // first parameter is value type, second is variable type
            {
                pStk->SetError(CBotErrBadType1, p->GetStart());
                delete inst_defvar;
                return pStack->Return(nullptr, pStk);
            }
        }

        std::unique_ptr<CBotVar> var = CBotVar::Create(*vartoken, baseType);// create the variable (evaluated after the assignment)
        var->SetInit(inst_defvar->m_expr != nullptr ? CBotVar::InitType::DEF : CBotVar::InitType::UNDEF);
        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst_defvar->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var.release());

        inst = inst_defvar;
    }

    if (pStk->IsOk() && IsOfType(p,  ID_COMMA))
    {
        // TODO: is there a bug here? CompileAfterType could return an error, but as long as there's an ID_SEP afterwards
        // we still return a valid instruction?
        // TODO: potentially invalid cast, to get around 'protected' modifier!
        if (nullptr != ( static_cast<CBotDefVariable*>(inst)->m_next2b = CBotDefVariable::CompileAfterType(p, pStk, baseType, noskip)))
        {
            return pStack->Return(inst, pStk);
        }
        if (!noskip)
        {
            delete inst;
            return pStack->Return(nullptr, pStk);
        }
    }

    if (noskip || IsOfType(p,  ID_SEP))
    {
        return pStack->Return(inst, pStk);
    }

    pStk->SetError(CBotErrNoTerminator, p->GetStart());
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotDefVariable::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    assert(!cont); // unused; TODO remove

    CBotTypResult type;

    if (!cont)
    {
        if (IsOfType(p, ID_BOOLEAN, ID_BOOL))
        {
            type = CBotTypBoolean;
        }
        else if (IsOfType(p, ID_FLOAT))
        {
            type = CBotTypFloat;
        }
        else if (IsOfType(p, ID_INT))
        {
            type = CBotTypInt;
        }
        else if (IsOfType(p, ID_STRING))
        {
            type = CBotTypString;
        }
        else
        {
            return nullptr;
        }

        while (IsOfType(p, ID_OPBRK))
        {
            if (!IsOfType(p, ID_CLBRK))
            {
                pStack->SetError(CBotErrCloseIndex, p->GetStart());
                return nullptr;
            }

            type = CBotTypResult(CBotTypArrayPointer, type);
        }
    }

    // TODO: if type is a CBotTypArrayPointer then we should make sure this behaves like CBotInstr::CompileArray did
    // and delete CBotInstr::CompileArray

    return CompileAfterType(p, pStack, type, noskip);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotDefVariable::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);//essential for SetState()

    if ( pile->GetState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return false;
        m_var->Execute(pile);

        if (!pile->SetState(1)) return false;
    }

    if (pile->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return false;

    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotDefVariable::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile = pj;
    if (bMain)
    {
        pile = pj->RestoreStack(this);
        if (pile == nullptr) return;

        if ( pile->GetState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);        // initial value interrupted?
            return;
        }
    }

    m_var->RestoreState(pile, bMain);

    if (m_next2b)
         m_next2b->RestoreState(pile, bMain);                // other(s) definition(s)
}

std::map<std::string, CBotInstr*> CBotDefVariable::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_var"] = m_var;
    links["m_expr"] = m_expr;
    return links;
}

} // namespace CBot