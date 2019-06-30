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

#include "CBot/CBotInstr/CBotFunction.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotEmpty.h"
#include "CBot/CBotInstr/CBotListArray.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"
#include "CBot/CBotDefParam.h"
#include "CBot/CBotUtils.h"

#include "CBot/CBotVar/CBotVar.h"

#include <cassert>
#include <sstream>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotFunction::CBotFunction()
{
    m_param = nullptr;            // empty parameter list
    m_block = nullptr;            // the instruction block
    m_bPublic    = false;           // function not public
    m_bExtern    = false;           // function not extern
    m_pProg      = nullptr;
//  m_nThisIdent = 0;
    m_nFuncIdent = 0;
    m_bSynchro    = false;
}

////////////////////////////////////////////////////////////////////////////////
std::set<CBotFunction*> CBotFunction::m_publicFunctions{};

////////////////////////////////////////////////////////////////////////////////
CBotFunction::~CBotFunction()
{
    delete m_param;                // empty parameter list
    delete m_block;                // the instruction block

    // remove public list if there is
    if (m_bPublic)
    {
        m_publicFunctions.erase(this);
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::IsPublic()
{
    return m_bPublic;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::IsExtern()
{
    return m_bExtern;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::GetPosition(int& start, int& stop, CBotGet modestart, CBotGet modestop)
{
    start = m_extern.GetStart();
    stop = m_closeblk.GetEnd();

    if (modestart == GetPosExtern)
    {
        start = m_extern.GetStart();
    }
    if (modestop == GetPosExtern)
    {
        stop = m_extern.GetEnd();
    }
    if (modestart == GetPosNom)
    {
        start = m_token.GetStart();
    }
    if (modestop == GetPosNom)
    {
        stop = m_token.GetEnd();
    }
    if (modestart == GetPosParam)
    {
        start = m_openpar.GetStart();
    }
    if (modestop == GetPosParam)
    {
        stop = m_closepar.GetEnd();
    }
    if (modestart == GetPosBloc)
    {
        start = m_openblk.GetStart();
    }
    if (modestop == GetPosBloc)
    {
        stop = m_closeblk.GetEnd();
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotFunction::Compile(CBotToken* &p, CBotCStack* pStack, CBotFunction* finput, bool bLocal)
{
    CBotToken*      pp;
    CBotFunction* func = finput;
    if ( func == nullptr ) func = new CBotFunction();

    CBotCStack* pStk = pStack->TokenStack(p, bLocal);

//  func->m_nFuncIdent = CBotVar::NextUniqNum();

    while (true)
    {
        if ( IsOfType(p, ID_PUBLIC) )
        {
            func->m_bPublic = true;
            continue;
        }
        pp = p;
        if ( IsOfType(p, ID_EXTERN) )
        {
            func->m_extern = *pp;        // for the position of the word "extern"
            func->m_bExtern = true;
//          func->m_bPublic = true;     // therefore also public!
            continue;
        }
        break;
    }

    func->m_retToken = *p;
//  CBotClass*  pClass;
    func->m_retTyp = TypeParam(p, pStk);        // type of the result

    if (func->m_retTyp.GetType() >= 0)
    {
        CBotToken*  pp = p;
        func->m_token = *p;

        if ( IsOfType(p, ID_NOT) )
        {
            CBotToken d(std::string("~") + p->GetString());
            d.SetPos(pp->GetStart(), p->GetEnd());
            func->m_token = d;
        }

        // un nom de fonction est-il là ?
        if (IsOfType(p, TokenTypVar))
        {
            if ( IsOfType( p, ID_DBLDOTS ) )        // method for a class
            {
                func->m_MasterClass = pp->GetString();
                func->m_classToken = *pp;
                CBotClass* pClass = CBotClass::Find(pp);
                if ( pClass == nullptr )
                {
                    pStk->SetError(CBotErrNoClassName, pp);
                    goto bad;
                }

//              pp = p;
                func->m_token = *p;
                if (!IsOfType(p, TokenTypVar)) goto bad;

            }
            func->m_openpar = *p;
            delete func->m_param;
            func->m_param = CBotDefParam::Compile(p, pStk );
            func->m_closepar = *(p->GetPrev());
            if (pStk->IsOk())
            {
                pStk->SetRetType(func->m_retTyp);   // for knowledge what type returns

                if (!func->m_MasterClass.empty())
                {
                    // return "this" known
                    CBotVariable* pThis = new CBotVariable("this", CBotVar::Create(CBotTypResult( CBotTypClass, func->m_MasterClass )));
                    pThis->m_value->SetInit(CBotVar::InitType::IS_POINTER);
                    pStk->AddVar(pThis);

                    // initialize variables acording to This
                    // only saves the pointer to the first,
                    // the rest is chained
//                  int num = 1;
                    for (std::unique_ptr<CBotVariable> &pv : pThis->m_value->GetItemList())
                    {
                        std::unique_ptr<CBotVariable> pcopy = CBotVariable::Create(pv.get());
//                      pcopy->SetInit(2);
                        pcopy->m_value->Copy(pv->m_value.get());
                        pcopy->SetPrivate(pv->GetPrivate());
                        pStk->AddVar(std::move(pcopy));
                    }
                }

                // and compiles the following instruction block
                func->m_openblk = *p;
                func->m_block = CBotBlock::Compile(p, pStk, false);
                func->m_closeblk = (p != nullptr && p->GetPrev() != nullptr) ? *(p->GetPrev()) : CBotToken();
                if ( pStk->IsOk() )
                {
                    if (!func->m_retTyp.Eq(CBotTypVoid) && !func->HasReturn())
                    {
                        int errPos = func->m_closeblk.GetStart();
                        pStk->ResetError(CBotErrNoReturn, errPos, errPos);
                        goto bad;
                    }
                    return pStack->ReturnFunc(func, pStk);
                }
            }
        }
bad:
        pStk->SetError(CBotErrNoFunc, p);
    }
    pStk->SetError(CBotErrNoType, p);
    if ( finput == nullptr ) delete func;
    return pStack->ReturnFunc(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotFunction::Compile1(CBotToken* &p, CBotCStack* pStack, CBotClass*  pClass)
{
    CBotFunction* func = new CBotFunction();
    func->m_nFuncIdent = CBotVariable::NextUniqNum();

    CBotCStack* pStk = pStack->TokenStack(p, true);

    while (true)
    {
        if ( IsOfType(p, ID_PUBLIC) )
        {
        //  func->m_bPublic = true;     // will be done in two passes
            continue;
        }
        if ( IsOfType(p, ID_EXTERN) )
        {
            func->m_bExtern = true;
            continue;
        }
        break;
    }

    func->m_retToken = *p;
    func->m_retTyp = TypeParam(p, pStack);      // type of the result

    if (func->m_retTyp.GetType() >= 0)
    {
        CBotToken*  pp = p;
        func->m_token = *p;

        if ( IsOfType(p, ID_NOT) )
        {
            CBotToken d(std::string("~") + p->GetString());
            d.SetPos(pp->GetStart(), p->GetEnd());
            func->m_token = d;
        }

        // un nom de fonction est-il là ?
        if (IsOfType(p, TokenTypVar))
        {
            if ( IsOfType( p, ID_DBLDOTS ) )        // method for a class
            {
                func->m_MasterClass = pp->GetString();
                // existence of the class is checked
                // later in CBotFunction::Compile()
                pp = p;
                func->m_token = *p;
                if (!IsOfType(p, TokenTypVar)) goto bad;

            }

            CBotToken* openPar = p;
            func->m_param = CBotDefParam::Compile(p, pStk); // compile parameters

            if (pStk->IsOk() && pClass != nullptr) // method in a class
            {
                // check if a constructor has return type void
                if (func->GetName() == pClass->GetName() && !func->m_retTyp.Eq(CBotTypVoid))
                {
                    pp = &(func->m_retToken);
                    pStk->SetError(CBotErrFuncNotVoid, pp);
                }

                if (pStk->IsOk() && pp->GetString() == "~") // destructor
                {
                    // check destructor name
                    if (func->GetName() != ("~" + pClass->GetName()))
                        pStk->SetError(CBotErrNoFunc, pp);
                    // confirm no parameters
                    if (pStk->IsOk() && func->m_param != nullptr)
                        pStk->SetError(CBotErrClosePar, openPar->GetNext());
                    // must return void
                    if (pStk->IsOk() && !func->m_retTyp.Eq(CBotTypVoid))
                    {
                        pp = &(func->m_retToken);
                        pStk->SetError(CBotErrFuncNotVoid, pp);
                    }
                }
            }

            if (pStk->IsOk())
            {
                // looks if the function exists elsewhere
                pp = &(func->m_token);
                if (( pClass != nullptr || !pStack->CheckCall(pp, func->m_param)) &&
                    ( pClass == nullptr || !pClass->CheckCall(pStack->GetProgram(), func->m_param, pp)) )
                {
                    if (IsOfType(p, ID_OPBLK))
                    {
                        int level = 1;
                        // and skips the following instruction block
                        do
                        {
                            int type = p->GetType();
                            p = p->GetNext();
                            if (type == ID_OPBLK) level++;
                            if (type == ID_CLBLK) level--;
                        }
                        while (level > 0 && p != nullptr);

                        return pStack->ReturnFunc(func, pStk);
                    }
                    pStk->SetError(CBotErrOpenBlock, p);
                }
            }
            pStk->SetError(CBotErrRedefFunc, pp);
        }
bad:
        pStk->SetError(CBotErrNoFunc, p);
    }
    pStk->SetError(CBotErrNoType, p);
    delete func;
    return pStack->ReturnFunc(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::Execute(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance)
{
    CBotStack*  pile = pj->AddStack(this, CBotStack::BlockVisibilityType::FUNCTION);               // one end of stack local to this function
//  if ( pile == EOX ) return true;

    pile->SetProgram(m_pProg);                              // bases for routines

    if ( pile->IfStep() ) return false;

    if ( pile->GetState() == 0 )
    {
        if (m_param != nullptr)
        {
            // stack for parameters and default args
            CBotStack* pile3b = pile->AddStack();
            pile3b->SetState(1);

            if ( !m_param->Execute(ppVars, pile) ) return false;    // define parameters
            pile3b->Delete(); // done with param stack
        }
        pile->IncState();
    }

    if ( pile->GetState() == 1 && !m_MasterClass.empty() )
    {
        // makes "this" known
        std::unique_ptr<CBotVariable> pThis;
        if ( pInstance == nullptr )
        {
            pThis.reset(new CBotVariable("this", CBotVar::Create(CBotTypResult( CBotTypClass, m_MasterClass ))));
        }
        else
        {
            if (m_MasterClass != pInstance->GetClass()->GetName())
            {
                pile->SetError(CBotErrBadType2, &m_classToken);
                return false;
            }

            pThis.reset(new CBotVariable("this", CBotVar::Create(CBotTypResult( CBotTypPointer, m_MasterClass ))));
            pThis->m_value->SetPointer(pInstance);
        }
        assert(pThis != nullptr);
        pThis->m_value->SetInit(CBotVar::InitType::IS_POINTER);

        pile->AddVar(std::move(pThis));

        pile->IncState();
    }

    if ( !m_block->Execute(pile) )
    {
        if ( pile->GetError() < 0 )
            pile->SetError( CBotNoErr );
        else
            return false;
    }

    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotFunction::RestoreState(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance)
{
    CBotStack*  pile = pj->RestoreStack(this);          // one end of stack local to this function
    if ( pile == nullptr ) return;
    CBotStack*  pile2 = pile;

    pile->SetProgram(m_pProg);                          // bases for routines

    if ( pile->GetBlock() != CBotStack::BlockVisibilityType::FUNCTION)
    {
        CBotStack*  pile2 = pile->RestoreStack(nullptr);       // one end of stack local to this function
        if ( pile2 == nullptr ) return;
        pile->SetState(pile->GetState() + pile2->GetState());
        pile2->Delete();
    }

    if ( pile->GetState() == 0 )
    {
        if (m_param != nullptr)
        {
            CBotStack* pile3b = pile2->RestoreStack();

            if (pile3b != nullptr && pile3b->GetState() == 1)
                m_param->RestoreState(pile2, true); // restore executing default arguments
            else
                m_param->RestoreState(pile2, false); // restore parameter IDs
        }
        return;
    }

    if (m_param != nullptr)
        m_param->RestoreState(pile2, false); // restore parameter IDs

    if ( !m_MasterClass.empty() )
    {
        CBotVariable* pThis = pile->FindVar("this");
        pThis->m_value->SetInit(CBotVar::InitType::IS_POINTER);
        pThis->m_value->SetPointer(pInstance);
    }

    m_block->RestoreState(pile2, true);
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotFunction::CompileCall(const std::list<CBotFunction*>& localFunctionList, const std::string &name, CBotVar** ppVars, long &nIdent)
{
    CBotTypResult type;
    if (!FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type))
    {
        // Reset the identifier to "not found" value
        nIdent = 0;
    }
    return type;
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotFunction::FindLocalOrPublic(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name,
                                              CBotVar** ppVars, CBotTypResult &TypeOrError, bool bPublic)
{
    TypeOrError.SetType(CBotErrUndefCall);      // no routine of the name

    if ( nIdent )
    {
        for (CBotFunction* pt : localFunctionList)
        {
            if (pt->m_nFuncIdent == nIdent)
            {
                TypeOrError = pt->m_retTyp;
                return pt;
            }
        }

        // search the list of public functions
        for (CBotFunction* pt : m_publicFunctions)
        {
            if (pt->m_nFuncIdent == nIdent)
            {
                TypeOrError = pt->m_retTyp;
                return pt;
            }
        }
    }

    if ( name.empty() ) return nullptr;

    std::map<CBotFunction*, int> funcMap;

    for (CBotFunction* pt : localFunctionList)
    {
        if ( pt->m_token.GetString() == name )
        {
            int i = 0;
            int alpha = 0;                          // signature of parameters
            // parameters are compatible?
            CBotDefParam* pv = pt->m_param;         // expected list of parameters
            CBotVar* pw = ppVars[i++];              // provided list parameter
            while ( pv != nullptr && (pw != nullptr || pv->HasDefault()) )
            {
                if (pw == nullptr)     // end of arguments
                {
                    pv = pv->GetNext();
                    continue;          // skip params with default values
                }
                CBotTypResult paramType = pv->GetTypResult();
                CBotTypResult argType = pw->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);

                if (!TypesCompatibles(paramType, argType))
                {
                    if ( funcMap.empty() ) TypeOrError.SetType(CBotErrBadParam);
                    break;
                }

                if (paramType.Eq(CBotTypPointer) && !argType.Eq(CBotTypNullPointer))
                {
                    CBotClass* c1 = paramType.GetClass();
                    CBotClass* c2 = argType.GetClass();
                    while (c2 != c1 && c2 != nullptr)    // implicit cast
                    {
                        alpha += 10;
                        c2 = c2->GetParent();
                    }
                }
                else
                {
                    int d = pv->GetType() - pw->GetType(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);
                    alpha += d>0 ? d : -10*d;       // quality loss, 10 times more expensive!
                }
                pv = pv->GetNext();
                pw = ppVars[i++];
            }
            if ( pw != nullptr )
            {
                if ( !funcMap.empty() ) continue;
                if ( TypeOrError.Eq(CBotErrLowParam) ) TypeOrError.SetType(CBotErrNbParam);
                if ( TypeOrError.Eq(CBotErrUndefCall)) TypeOrError.SetType(CBotErrOverParam);
                continue;                   // too many parameters
            }
            if ( pv != nullptr )
            {
                if ( !funcMap.empty() ) continue;
                if ( TypeOrError.Eq(CBotErrOverParam) ) TypeOrError.SetType(CBotErrNbParam);
                if ( TypeOrError.Eq(CBotErrUndefCall) ) TypeOrError.SetType(CBotErrLowParam);
                continue;                   // not enough parameters
            }
            funcMap.insert( std::pair<CBotFunction*, int>(pt, alpha) );
        }
    }

    if ( bPublic )
    {
        for (CBotFunction* pt : m_publicFunctions)
        {
            if ( pt->m_token.GetString() == name )
            {
                int i = 0;
                int alpha = 0;                          // signature of parameters
                // parameters sont-ils compatibles ?
                CBotDefParam* pv = pt->m_param;         // list of expected parameters
                CBotVar* pw = ppVars[i++];              // list of provided parameters
                while ( pv != nullptr && (pw != nullptr || pv->HasDefault()) )
                {
                    if (pw == nullptr)     // end of arguments
                    {
                        pv = pv->GetNext();
                        continue;          // skip params with default values
                    }
                    CBotTypResult paramType = pv->GetTypResult();
                    CBotTypResult argType = pw->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);

                    if (!TypesCompatibles(paramType, argType))
                    {
                        if ( funcMap.empty() ) TypeOrError.SetType(CBotErrBadParam);
                        break;
                    }

                    if (paramType.Eq(CBotTypPointer) && !argType.Eq(CBotTypNullPointer))
                    {
                        CBotClass* c1 = paramType.GetClass();
                        CBotClass* c2 = argType.GetClass();
                        while (c2 != c1 && c2 != nullptr)    // implicit cast
                        {
                            alpha += 10;
                            c2 = c2->GetParent();
                        }
                    }
                    else
                    {
                        int d = pv->GetType() - pw->GetType(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);
                        alpha += d>0 ? d : -10*d;       // quality loss, 10 times more expensive!
                    }
                    pv = pv->GetNext();
                    pw = ppVars[i++];
                }
                if ( pw != nullptr )
                {
                    if ( !funcMap.empty() ) continue; // previous useable function
                    if ( TypeOrError.Eq(CBotErrLowParam) ) TypeOrError.SetType(CBotErrNbParam);
                    if ( TypeOrError.Eq(CBotErrUndefCall)) TypeOrError.SetType(CBotErrOverParam);
                    continue;                   // to many parameters
                }
                if ( pv != nullptr )
                {
                    if ( !funcMap.empty() ) continue; // previous useable function
                    if ( TypeOrError.Eq(CBotErrOverParam) ) TypeOrError.SetType(CBotErrNbParam);
                    if ( TypeOrError.Eq(CBotErrUndefCall) ) TypeOrError.SetType(CBotErrLowParam);
                    continue;                   // not enough parameters
                }
                funcMap.insert( std::pair<CBotFunction*, int>(pt, alpha) );
            }
        }
    }

    if ( !funcMap.empty() )
    {
        auto it = funcMap.begin();
        CBotFunction* pFunc = it->first;        // the best function found
        signed int    delta = it->second;       // seeks the lowest signature

        for (++it ; it != funcMap.end() ; it++)
        {
            if (it->second < delta) // a better signature?
            {
                TypeOrError.SetType(CBotNoErr);
                pFunc = it->first;
                delta = it->second;
                continue;
            }

            if (it->second == delta) TypeOrError.SetType(CBotErrAmbiguousCall);
        }

        if (TypeOrError.Eq(CBotErrAmbiguousCall)) return nullptr;
        nIdent = pFunc->m_nFuncIdent;
        TypeOrError = pFunc->m_retTyp;
        return pFunc;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
int CBotFunction::DoCall(CBotProgram* program, const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name,
                         CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken)
{
    CBotTypResult   type;
    CBotFunction*   pt = nullptr;
    CBotProgram*    baseProg = pStack->GetProgram(true);

    pt = FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type);

    if ( pt != nullptr )
    {
        CBotStack*  pStk1 = pStack->AddStack(pt, CBotStack::BlockVisibilityType::FUNCTION);    // to put "this"
//      if ( pStk1 == EOX ) return true;

        pStk1->SetProgram(pt->m_pProg);                 // it may have changed module

        if ( pStk1->IfStep() ) return false;

        CBotStack*  pStk3 = pStk1->AddStack(nullptr, CBotStack::BlockVisibilityType::BLOCK);    // parameters

        // preparing parameters on the stack

        if ( pStk1->GetState() == 0 )
        {
            // stack for parameters and default args
            CBotStack* pStk3b = pStk3->AddStack();

            if (pStk3b->GetState() == 0 && !pt->m_MasterClass.empty())
            {
                CBotVar* pInstance = (baseProg != nullptr) ? baseProg->m_thisVar : nullptr;
                // make "this" known
                std::unique_ptr<CBotVariable> pThis;
                if ( pInstance == nullptr )
                {
                    pThis.reset(new CBotVariable("this", CBotVar::Create(CBotTypResult( CBotTypClass, pt->m_MasterClass ))));
                }
                else
                {
                    if (pt->m_MasterClass != pInstance->GetClass()->GetName())
                    {
                        pStack->SetError(CBotErrBadType2, &pt->m_classToken);
                        return false;
                    }

                    pThis.reset(new CBotVariable("this", CBotVar::Create(CBotTypResult( CBotTypPointer, pt->m_MasterClass ))));
                    pThis->m_value->SetPointer(pInstance);
                }
                assert(pThis != nullptr);
                pThis->m_value->SetInit(CBotVar::InitType::IS_POINTER);

                pStk1->AddVar(std::move(pThis));
            }
            pStk3b->SetState(1); // set 'this' was created

            // initializes the variables as parameters
            if (pt->m_param != nullptr)
            {
                if (!pt->m_param->Execute(ppVars, pStk3)) // interupt here
                {
                    if (!pStk3->IsOk() && pt->m_pProg != program)
                    {
                        pStk3->SetPosError(pToken);       // indicates the error on the procedure call
                    }
                    return false;
                }
            }
            pStk3b->Delete(); // done with param stack
            pStk1->IncState();
        }

        // finally execution of the found function

        if ( !pStk3->GetRetVar(                     // puts the result on the stack
            pt->m_block->Execute(pStk3) ))          // GetRetVar said if it is interrupted
        {
            if ( !pStk3->IsOk() && pt->m_pProg != program )
            {
                pStk3->SetPosError(pToken);         // indicates the error on the procedure call
            }
            return false;   // interrupt !
        }

        return pStack->Return( pStk3 );
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
void CBotFunction::RestoreCall(const std::list<CBotFunction*>& localFunctionList,
                               long &nIdent, const std::string &name, CBotVar** ppVars, CBotStack* pStack)
{
    CBotTypResult   type;
    CBotFunction*   pt = nullptr;
    CBotStack*      pStk1;
    CBotStack*      pStk3;
    CBotProgram*    baseProg = pStack->GetProgram(true);

    pt = FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type);

    if ( pt != nullptr )
    {
        pStk1 = pStack->RestoreStack(pt);
        if ( pStk1 == nullptr ) return;

        pStk1->SetProgram(pt->m_pProg);                 // it may have changed module

        if ( pStk1->GetBlock() != CBotStack::BlockVisibilityType::FUNCTION)
        {
            CBotStack* pStk2 = pStk1->RestoreStack(nullptr); // used more
            if ( pStk2 == nullptr ) return;
            pStk3 = pStk2->RestoreStack(nullptr);
            if ( pStk3 == nullptr ) return;
        }
        else
        {
            pStk3 = pStk1->RestoreStack(nullptr);
            if ( pStk3 == nullptr ) return;
        }

        // preparing parameters on the stack

        {
            if ( !pt->m_MasterClass.empty() )
            {
                CBotVar* pInstance = (baseProg != nullptr) ? baseProg->m_thisVar : nullptr;
                // make "this" known
                CBotVariable* pThis = pStk1->FindVar("this");
                pThis->m_value->SetInit(CBotVar::InitType::IS_POINTER);
                pThis->m_value->SetPointer(pInstance);
            }
        }

        if ( pStk1->GetState() == 0 )
        {
            if (pt->m_param != nullptr)
            {
                CBotStack* pStk3b = pStk3->RestoreStack();

                if (pStk3b != nullptr && pStk3b->GetState() == 1)
                    pt->m_param->RestoreState(pStk3, true); // restore executing default arguments
                else
                    pt->m_param->RestoreState(pStk3, false); // restore parameter IDs
            }
            return;
        }

        // initializes the variables as parameters
        if (pt->m_param != nullptr)
            pt->m_param->RestoreState(pStk3, false); // restore parameter IDs
        pt->m_block->RestoreState(pStk3, true);
    }
}

////////////////////////////////////////////////////////////////////////////////
int CBotFunction::DoCall(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name, CBotVar* pThis,
                         CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass)
{
    CBotTypResult   type;
    CBotProgram*    pProgCurrent = pStack->GetProgram();

    CBotFunction*   pt = FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type, false);

    if ( pt != nullptr )
    {
//      DEBUG( "CBotFunction::DoCall" + pt->GetName(), 0, pStack);

        CBotStack*  pStk = pStack->AddStack(pt, CBotStack::BlockVisibilityType::FUNCTION);
//      if ( pStk == EOX ) return true;

        pStk->SetProgram(pt->m_pProg);                  // it may have changed module
        CBotStack*  pStk3 = pStk->AddStack(nullptr, CBotStack::BlockVisibilityType::BLOCK); // to set parameters passed

        // preparing parameters on the stack

        if ( pStk->GetState() == 0 )
        {
            // stack for parameters and default args
            CBotStack* pStk3b = pStk3->AddStack();

            if (pStk3b->GetState() == 0)
            {
                // sets the variable "this" on the stack
                std::unique_ptr<CBotVariable> pthis(new CBotVariable("this", CBotVar::Create(CBotTypNullPointer)));
                pthis->m_value->Copy(pThis);
                pStk->AddVar(std::move(pthis));

                // Note pthis vs pThis
                CBotClass*  pClass = pThis->GetClass()->GetParent();
                if ( pClass )
                {
                    // sets the variable "super" on the stack
                    std::unique_ptr<CBotVariable> psuper(new CBotVariable("super", CBotVar::Create(CBotTypNullPointer)));
                    psuper->m_value->Copy(pThis); // in fact identical to "this"
                    pStk->AddVar(std::move(psuper));
                }
            }
            pStk3b->SetState(1); // set 'this' was created

            // initializes the variables as parameters
            if (pt->m_param != nullptr)
            {
                if (!pt->m_param->Execute(ppVars, pStk3)) // interupt here
                {
                    if (!pStk3->IsOk() && pt->m_pProg != pProgCurrent)
                    {
                        pStk3->SetPosError(pToken);       // indicates the error on the procedure call
                    }
                    return false;
                }
            }
            pStk3b->Delete(); // done with param stack
            pStk->IncState();
        }

        if ( pStk->GetState() == 1 )
        {
            if ( pt->m_bSynchro )
            {
                CBotProgram* pProgBase = pStk->GetProgram(true);
                if ( !pClass->Lock(pProgBase) ) return false; // try to lock, interrupt if failed
            }
            pStk->IncState();
        }
        // finally calls the found function

        if ( !pStk3->GetRetVar(                         // puts the result on the stack
            pt->m_block->Execute(pStk3) ))          // GetRetVar said if it is interrupted
        {
            if ( !pStk3->IsOk() )
            {
                if ( pt->m_bSynchro )
                {
                    pClass->Unlock();                   // release function
                }

                if ( pt->m_pProg != pProgCurrent )
                {
                    pStk3->SetPosError(pToken);         // indicates the error on the procedure call
                }
            }
            return false;   // interrupt !
        }

        if ( pt->m_bSynchro )
        {
            pClass->Unlock();                           // release function
        }

        return pStack->Return( pStk3 );
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::RestoreCall(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name, CBotVar* pThis,
                               CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass)
{
    CBotTypResult   type;
    CBotFunction*   pt = FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type);

    if ( pt != nullptr )
    {
        CBotStack*  pStk = pStack->RestoreStack(pt);
        if ( pStk == nullptr ) return true;
        pStk->SetProgram(pt->m_pProg);                  // it may have changed module

        CBotStack*  pStk3 = pStk->RestoreStack(nullptr);   // to set parameters passed
        if ( pStk3 == nullptr ) return true;

        if ( pStk->GetState() == 0 )
        {
            if (pt->m_param != nullptr)
            {
                CBotStack* pStk3b = pStk3->RestoreStack();
                if (pStk3b != nullptr && pStk3b->GetState() == 1)
                    pt->m_param->RestoreState(pStk3, true); // restore executing default arguments
                else
                    pt->m_param->RestoreState(pStk3, false); // restore parameter IDs
            }
            return true;
        }

        if (pt->m_param != nullptr)
            pt->m_param->RestoreState(pStk3, false); // restore parameter IDs

        if ( pStk->GetState() > 1 &&                        // latching is effective?
             pt->m_bSynchro )
            {
                CBotProgram* pProgBase = pStk->GetProgram(true);
                pClass->Lock(pProgBase);                    // locks the class
            }

        // finally calls the found function

        pt->m_block->RestoreState(pStk3, true);                 // interrupt !
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::CheckParam(CBotDefParam* pParam)
{
    CBotDefParam*   pp = m_param;
    while ( pp != nullptr && pParam != nullptr )
    {
        CBotTypResult type1 = pp->GetTypResult();
        CBotTypResult type2 = pParam->GetTypResult();
        if ( !type1.Compare(type2) ) return false;
        pp = pp->GetNext();
        pParam = pParam->GetNext();
    }
    return ( pp == nullptr && pParam == nullptr );
}

////////////////////////////////////////////////////////////////////////////////
const std::string& CBotFunction::GetName()
{
    return  m_token.GetString();
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotFunction::GetParams()
{
    if (m_param == nullptr ) return std::string("()");

    std::string      params = "( ";
    CBotDefParam*   p = m_param;        // list of parameters

    while (p != nullptr)
    {
        params += p->GetParamString();
        p = p->GetNext();
        if ( p != nullptr ) params += ", ";
    }

    params += " )";
    return params;
}

////////////////////////////////////////////////////////////////////////////////
void CBotFunction::AddPublic(CBotFunction* func)
{
    m_publicFunctions.insert(func);
}

bool CBotFunction::HasReturn()
{
    if (m_block != nullptr) return m_block->HasReturn();
    return false;
}

std::string CBotFunction::GetDebugData()
{
    std::stringstream ss;
    if (IsPublic()) ss << "public ";
    if (IsExtern()) ss << "extern ";
    ss << GetName() << GetParams();
    //ss << "FuncID = " << m_nFuncIdent;
    return ss.str();
}

std::map<std::string, CBotInstr*> CBotFunction::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_block"] = m_block;
    return links;
}

} // namespace CBot
