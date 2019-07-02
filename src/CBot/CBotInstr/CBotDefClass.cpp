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

#include "CBot/CBotInstr/CBotDefClass.h"

#include "CBot/CBotInstr/CBotExprRetVar.h"
#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotInstr/CBotLeftExprVar.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"
#include "CBot/CBotInstr/CBotDefArray.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotVar/CBotVarPointer.h"
#include "CBot/CBotVar/CBotVarClass.h"

#include "common/make_unique.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotDefClass::CBotDefClass()
{
    m_next          = nullptr;
    m_var           = nullptr;
    m_parameters    = nullptr;
    m_expr          = nullptr;
    m_hasParams     = false;
    m_nMethodeIdent = 0;
    m_exprRetVar    = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotDefClass::~CBotDefClass()
{
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotInstr> CBotDefClass::Compile(CBotToken* &p, CBotCStack* pStack, CBotClass* pClass)
{
    // seeks the corresponding classes
    if ( pClass == nullptr )
    {
        pStack->SetStartError(p->GetStart());
        pClass = CBotClass::Find(p);
        if ( pClass == nullptr )
        {
            // not found? is bizare
            pStack->SetError(CBotErrNotClass, p);
            return nullptr;
        }
        p = p->GetNext();
    }

    bool        bIntrinsic = pClass->IsIntrinsic();
    CBotTypResult type = CBotTypResult( bIntrinsic ? CBotTypIntrinsic : CBotTypPointer, pClass );
    {
        std::unique_ptr<CBotInstr> inst = CompileArray(p, pStack, type);
        if ( inst != nullptr || !pStack->IsOk() ) return inst;
    }

    CBotCStack* pStk = pStack->TokenStack();

    std::unique_ptr<CBotDefClass> instAsClass = MakeUnique<CBotDefClass>();
    /// TODO Need to be revised and fixed after adding unit tests
    CBotToken token(pClass->GetName(), std::string(), p->GetStart(), p->GetEnd());
    instAsClass->SetToken(&token);
    CBotToken*  vartoken = p;

    std::unique_ptr<CBotInstr> inst;

    if ( nullptr != (instAsClass->m_var = CBotLeftExprVar::Compile( p, pStk )) )
    {
        (static_cast<CBotLeftExprVar*>(instAsClass->m_var.get()))->m_typevar = type;
        if (pStk->CheckVarLocal(vartoken))                  // redefinition of the variable
        {
            pStk->SetStartError(vartoken->GetStart());
            pStk->SetError(CBotErrRedefVar, vartoken->GetEnd());
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))                         // with any clues?
        {
            instAsClass.reset();                            // is not type CBotDefInt
            p = vartoken;                                   // returns to the variable name

            // compiles declaration an array

            inst = CBotDefArray::Compile(p, pStk, type);

        }
        else
        {
            CBotVariable*    var = new CBotVariable(vartoken->GetString(), CBotVar::Create(type)); // creates the instance
    //      var->SetClass(pClass);
            pStack->AddVar(var);                                // placed on the stack

            // look if there are parameters
            instAsClass->m_hasParams = (p->GetType() == ID_OPENPAR);

            std::vector<CBotTypResult> ppVars;
            instAsClass->m_parameters = CompileParams(p, pStk, ppVars);
            if ( !pStk->IsOk() ) goto error;

            // if there are parameters, is the equivalent to the stament "new"
            // CPoint A ( 0, 0 ) is equivalent to
            // CPoint A = new CPoint( 0, 0 )

    //      if ( nullptr != inst->m_parameters )
            if ( instAsClass->m_hasParams )
            {
                // the constructor is there?
    //          std::string  noname;
                CBotTypResult r = pClass->CompileMethode(&token, var->m_value->GetTypResult(), ppVars, pStk, instAsClass->m_nMethodeIdent);
                pStk->DeleteChildLevels();                          // releases the supplement stack
                int typ = r.GetType();

                if (typ == CBotErrUndefCall)
                {
                    // si le constructeur n'existe pas
                    if (instAsClass->m_parameters != nullptr)                 // with parameters
                    {
                        pStk->SetError(CBotErrNoConstruct, vartoken);
                        goto error;
                    }
                    typ = 0;
                }

                if (typ>20) // XXX
                {
                    pStk->SetError(static_cast<CBotError>(typ), vartoken->GetEnd());
                    goto error;
                }

                pStk->SetVarType(var->m_value->GetTypResult());
                // chained method ?
                if (nullptr != (instAsClass->m_exprRetVar = CBotExprRetVar::Compile(p, pStk, true, false)))
                {
                    instAsClass->m_exprRetVar->SetToken(vartoken);
                    pStk->DeleteChildLevels();
                }
                pStk->SetVarType(CBotTypResult());

                if ( !pStk->IsOk() ) goto error;
            }

            if (IsOfType(p,  ID_ASS))                           // with a assignment?
            {
                pStk->SetStartError(p->GetStart());
                if (instAsClass->m_hasParams)
                {
                    pStk->SetError(CBotErrNoTerminator, p->GetStart());
                    goto error;
                }

                if ( IsOfType(p, ID_SEP) )
                {
                    pStk->SetError(CBotErrNoExpression, p->GetStart());
                    goto error;
                }

                if ( nullptr == ( instAsClass->m_expr = CBotTwoOpExpr::Compile( p, pStk )) )
                {
                    goto error;
                }
                CBotClass* result = pStk->GetVarType().GetClass();



                if ( !pStk->GetVarType().Eq(CBotTypNullPointer) &&
                   ( (!pStk->GetVarType().Eq(CBotTypPointer) &&
                     !pStk->GetVarType().Eq(CBotTypClass)) ||
                     ( result != nullptr && !(pClass->IsChildOf(result) ||
                                              result->IsChildOf(pClass)))))     // type compatible ?
                {
                    // TODO: pClass->IsChildOf(result) means we can assign from a superclass? what happens if it's not an instance of the subclass at runtime?
                    pStk->SetError(CBotErrBadType1, p->GetStart());
                    goto error;
                }
    //          if ( !bIntrinsic ) var->SetPointer(pStk->GetVar()->GetPointer());
                if ( !bIntrinsic )
                {
                    // does not use the result on the stack, to impose the class
                    std::unique_ptr<CBotVar> pvar = CBotVar::Create(CBotTypResult(CBotTypClass, pClass));
                    var->m_value->SetPointer( pvar.release() );              // variable already declared instance pointer
                }
                var->m_value->SetInit(CBotVar::InitType::DEF);                         // marks the pointer as init
            }
            else if (instAsClass->m_hasParams)
            {
                // creates the object on the stack
                // with a pointer to the object
                if ( !bIntrinsic )
                {
                    std::unique_ptr<CBotVar> pvar = CBotVar::Create(CBotTypResult(CBotTypClass, pClass));
                    var->m_value->SetPointer( pvar.release() );              // variable already declared instance pointer
                }
                var->m_value->SetInit(CBotVar::InitType::IS_POINTER);                            // marks the pointer as init
            }

            inst = std::move(instAsClass);
        }

        if (pStk->IsOk() && IsOfType(p,  ID_COMMA))         // several chained definitions
        {
            if ( nullptr != ( inst->m_next = CBotDefClass::Compile(p, pStk, pClass) ))    // compiles the following
            {
                return pStack->Return(move(inst), pStk);
            }
        }

        if (!pStk->IsOk() || IsOfType(p,  ID_SEP))          // complete instruction
        {
            return pStack->Return(move(inst), pStk);
        }

        pStk->SetError(CBotErrNoTerminator, p->GetStart());
    }

error:
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotDefClass::Execute(CBotStack* &pj)
{
    CBotVariable*    pThis = nullptr;

    CBotStack*  pile = pj->AddStack(this);//essential for SetState()
//  if ( pile == EOX ) return true;

    if (m_exprRetVar != nullptr) // Class c().method();
    {
        if (pile->GetState() == 4)
        {
            if (pile->IfStep()) return false;
            CBotStack* pile3 = pile->AddStack();
            if (!m_exprRetVar->Execute(pile3)) return false;
            pile3->SetVar(nullptr);
            pile->Return(pile3); // release pile3 stack
            pile->SetState(5);
        }
    }

    CBotToken*  pt = &m_token;
    CBotClass*  pClass = CBotClass::Find(pt);

    bool bIntrincic = pClass->IsIntrinsic();

    // creates the variable of type pointer to the object

    if ( pile->GetState()==0)
    {
        std::string  name = m_var->m_token.GetString();
        if ( bIntrincic )
        {
            pThis = new CBotVariable(name, CBotVar::Create(CBotTypResult( CBotTypIntrinsic, pClass )));
        }
        else
        {
            pThis = new CBotVariable(name, CBotVar::Create(CBotTypResult( CBotTypPointer, pClass )));
        }

        pile->AddVar(pThis);                                    // place on the stack
        pile->IncState();
    }

    if ( pThis == nullptr ) pThis = pile->FindVar(m_var->m_token, false);

    if ( pile->GetState()<3)
    {
        // ss there an assignment or parameters (contructor)

//      CBotVarClass* pInstance = nullptr;

        if ( m_expr != nullptr )
        {
            // evaluates the expression for the assignment
            if (!m_expr->Execute(pile)) return false;

            CBotVar* pv = pile->GetVar();

            if ( bIntrincic )
            {
                if ( pv == nullptr || pv->GetPointer() == nullptr )
                {
                    pile->SetError(CBotErrNull, &m_token);
                    return pj->Return(pile);
                }
                pThis->m_value->Copy(pile->GetVar());
            }
            else
            {
                if ( !(pv == nullptr || pv->GetPointer() == nullptr) )
                {
                    if ( !pv->GetClass()->IsChildOf(pClass))
                    {
                        pile->SetError(CBotErrBadType1, &m_token);
                        return pj->Return(pile);
                    }
                }

                CBotVarClass* pInstance;
                pInstance = pv->GetPointer();    // value for the assignment
                CBotTypResult type = pThis->m_value->GetTypResult();
                pThis->m_value->SetPointer(pInstance);
                pThis->m_value->SetType(type);        // keep pointer type
            }
            pThis->m_value->SetInit(CBotVar::InitType::DEF);
        }

        else if ( m_hasParams )
        {
            // evaluates the constructor of an instance

            if ( !bIntrincic && pile->GetState() == 1)
            {
                CBotToken*  pt = &m_token;
                CBotClass* pClass = CBotClass::Find(pt);

                // creates an instance of the requested class

                CBotVarClass* pInstance;
                pInstance = static_cast<CBotVarClass*>(CBotVar::Create(CBotTypResult(CBotTypClass, pClass)).release());
                pThis->m_value->SetPointer(pInstance);
                delete pInstance;

                pile->IncState();
            }

            CBotVar*    ppVars[1000];
            CBotStack*  pile2 = pile;

            int     i = 0;

            CBotInstr*  p = m_parameters.get();
            // evaluates the parameters
            // and places the values ​​on the stack
            // to (can) be interrupted (broken) at any time

            if ( p != nullptr) while ( true )
            {
                pile2 = pile2->AddStack();                      // place on the stack for the results
                if ( pile2->GetState() == 0 )
                {
                    if (!p->Execute(pile2)) return false;       // interrupted here?
                    pile2->SetState(1);
                }
                ppVars[i++] = pile2->GetVar();
                p = p->GetNext();
                if ( p == nullptr) break;
            }
            ppVars[i] = nullptr;

            // creates a variable for the result
            if ( !pClass->ExecuteMethode(m_nMethodeIdent, pThis->m_value.get(), ppVars, CBotTypResult(CBotTypVoid), pile2, GetToken())) return false; // interrupt

            pThis->m_value->SetInit(CBotVar::InitType::DEF);
            pThis->m_value->ConstructorSet();        // indicates that the constructor has been called
            pile->Return(pile2);                                // releases a piece of stack

//          pInstance = pThis->GetPointer();

        }

//      if ( !bIntrincic ) pThis->SetPointer(pInstance);        // a pointer to the instance

        pile->SetState(3);                                  // finished this part
    }

    if (m_exprRetVar != nullptr && pile->GetState() == 3) // Class c().method();
    {
        CBotStack* pile3 = pile->AddStack();
        pile3->SetCopyVar(pThis->m_value.get());
        pile->SetState(4);
        return false;              // go back to the top ^^^
    }

    if ( pile->IfStep() ) return false;

    if ( m_next2b != nullptr &&
        !m_next2b->Execute(pile)) return false;             // other (s) definition (s)

    return pj->Return( pile );                              // transmits below (further)
}

////////////////////////////////////////////////////////////////////////////////
void CBotDefClass::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotVariable*    pThis = nullptr;

    CBotStack*  pile = pj;
    if ( bMain ) pile = pj->RestoreStack(this);
    if ( pile == nullptr ) return;

    // creates the variable of type pointer to the object
    {
        std::string  name = m_var->m_token.GetString();
        pThis = pile->FindVar(name);
    }

    if (m_exprRetVar != nullptr) // Class c().method();
    {
        if (pile->GetState() == 4)
        {
            CBotStack* pile3 = pile->RestoreStack();
            m_exprRetVar->RestoreState(pile3, bMain);
            return;
        }
    }

    CBotToken*  pt = &m_token;
    CBotClass*  pClass = CBotClass::Find(pt);
    bool bIntrincic = pClass->IsIntrinsic();

    if ( bMain && pile->GetState()<3)
    {
        // is there an assignment or parameters (constructor)

//      CBotVarClass* pInstance = nullptr;

        if ( m_expr != nullptr )
        {
            // evaluates the expression for the assignment
            m_expr->RestoreState(pile, bMain);
            return;
        }

        else if ( m_hasParams )
        {
            // evaluates the constructor of an instance

            if ( !bIntrincic && pile->GetState() == 1)
            {
                return;
            }

            CBotVar*    ppVars[1000];
            CBotStack*  pile2 = pile;

            int     i = 0;

            CBotInstr*  p = m_parameters.get();
            // evaluates the parameters
            // and the values an the stack
            // so that it can be interrupted at any time

            if ( p != nullptr) while ( true )
            {
                pile2 = pile2->RestoreStack();                      // place on the stack for the results
                if ( pile2 == nullptr ) return;

                if ( pile2->GetState() == 0 )
                {
                    p->RestoreState(pile2, bMain);      // interrupted here?
                    return;
                }
                ppVars[i++] = pile2->GetVar();
                p = p->GetNext();
                if ( p == nullptr) break;
            }
            ppVars[i] = nullptr;

            // creates a variable for the result
            pClass->RestoreMethode(m_nMethodeIdent, pt, pThis->m_value.get(), ppVars, pile2);
            return;
        }
    }

    if ( m_next2b != nullptr )
         m_next2b->RestoreState(pile, bMain);                   // other(s) definition(s)
}

std::map<std::string, CBotInstr*> CBotDefClass::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_var"] = m_var.get();
    links["m_parameters"] = m_parameters.get();
    links["m_expr"] = m_expr.get();
    return links;
}

} // namespace CBot
