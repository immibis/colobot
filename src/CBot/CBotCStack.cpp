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


#include "CBot/CBotCStack.h"

#include "CBot/CBotToken.h"
#include "CBot/CBotExternalCall.h"

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotInstr/CBotFunction.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotError CBotCStack::m_error   = CBotNoErr;             // init the static variable
int CBotCStack::m_end      = 0;
CBotTypResult CBotCStack::m_retTyp  = CBotTypResult(0);

////////////////////////////////////////////////////////////////////////////////
CBotCStack::CBotCStack(CBotCStack* ppapa)
{
    m_next = nullptr;
    m_prev = ppapa;
    m_toplevel = (ppapa ? ppapa->m_toplevel : this);
    m_pProgram = nullptr;

    if (ppapa == nullptr)
    {
        m_error = CBotNoErr;
        m_start = 0;
        m_end    = 0;
        m_bBlock = true;
    }
    else
    {
        m_start = ppapa->m_start;
        m_bBlock = false;
    }

    m_listVar = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/*static*/ std::unique_ptr<CBotCStack> CBotCStack::BeginNewStack(CBotProgram *pProgram)
{
    // Can't create a stack with a null program.
    assert(pProgram != nullptr);

    std::unique_ptr<CBotCStack> newStack(new CBotCStack(nullptr));

    newStack->m_pProgram = pProgram;
    return newStack;
}

////////////////////////////////////////////////////////////////////////////////
CBotCStack::~CBotCStack()
{
    if (m_next != nullptr) delete m_next;
    if (m_prev != nullptr) m_prev->m_next = nullptr;        // removes chain

    delete m_listVar;
}

////////////////////////////////////////////////////////////////////////////////
CBotCStack* CBotCStack::TokenStack(CBotToken* pToken, bool bBlock)
{
    if (m_next != nullptr) return m_next;            // include on an existing stack

    CBotCStack*    p = new CBotCStack(this);
    m_next = p;                                    // channel element
    p->m_bBlock = bBlock;

    if (pToken != nullptr) p->SetStartError(pToken->GetStart());

    return    p;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotCStack::Return(CBotInstr* inst, CBotCStack* pfils)
{
    if ( pfils == this ) return inst; // TODO: Can this happen?



    m_var = std::move(pfils->m_var);             // result transmitted

    if (m_error)
    {
        m_start = pfils->m_start;                // retrieves the position of the error
        m_end    = pfils->m_end;
    }

    delete pfils;
    return inst;
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotCStack::ReturnFunc(CBotFunction* inst, CBotCStack* pfils)
{
    m_var = std::move(pfils->m_var);             // result transmitted

    if (m_error)
    {
        m_start = pfils->m_start;                // retrieves the position of the error
        m_end    = pfils->m_end;
    }

    delete pfils;
    return inst;
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotCStack::GetError(int& start, int& end)
{
    start = m_start;
    end      = m_end;
    return m_error;
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotCStack::GetError()
{
    return m_error;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotCStack::GetTypResult(CBotVar::GetTypeMode mode)
{
    if (m_var == nullptr)
        return CBotTypResult(99);
    return    m_var->GetTypResult(mode);
}

////////////////////////////////////////////////////////////////////////////////
int CBotCStack::GetType(CBotVar::GetTypeMode mode)
{
    if (m_var == nullptr)
        return 99;
    return    m_var->GetType(mode);
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotCStack::GetClass()
{
    if ( m_var == nullptr )
        return nullptr;
    if ( m_var->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) != CBotTypPointer ) return nullptr;

    return m_var->GetClass();
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetType(CBotTypResult& type)
{
    if (m_var == nullptr) return;
    m_var->SetType( type );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotCStack::FindVar(CBotToken* &pToken)
{
    CBotCStack*    p = this;
    std::string    name = pToken->GetString();

    while (p != nullptr)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != nullptr)
        {
            if (name == pp->GetName())
            {
                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotCStack::FindVar(CBotToken& Token)
{
    CBotToken*    pt = &Token;
    return FindVar(pt);
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotVar> CBotCStack::CopyVar(CBotToken& Token)
{
    CBotVar*    pVar = FindVar( Token );

    if ( pVar == nullptr) return nullptr;

    std::unique_ptr<CBotVar> pCopy = CBotVar::Create( "", pVar->GetType() );
    pCopy->Copy(pVar);
    return    pCopy;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCStack::IsOk()
{
    return (m_error == 0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetStartError( int pos )
{
    if ( m_error != 0) return;            // does not change existing error
    m_start = pos;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetError(CBotError n, int pos)
{
    if ( n!= 0 && m_error != 0) return;    // does not change existing error
    m_error = n;
    m_end    = pos;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetError(CBotError n, CBotToken* p)
{
    if (m_error) return;    // does not change existing error
    m_error = n;
    m_start    = p->GetStart();
    m_end    = p->GetEnd();
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::ResetError(CBotError n, int start, int end)
{
    m_error = n;
    m_start    = start;
    m_end    = end;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCStack::NextToken(CBotToken* &p)
{
    CBotToken*    pp = p;

    p = p->GetNext();
    if (p!=nullptr) return true;

    SetError(CBotErrNoTerminator, pp->GetEnd());
    return false;
}

////////////////////////////////////////////////////////////////////////////////
CBotProgram* CBotCStack::GetProgram()
{
    return m_toplevel->m_pProgram;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetRetType(CBotTypResult& type)
{
    m_retTyp = type;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotCStack::GetRetType()
{
    return m_retTyp;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetVar( std::unique_ptr<CBotVar> var )
{
    m_var = std::move(var);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetCopyVar( CBotVar* var )
{
    if ( var == nullptr )
    {
        m_var.reset(); // set to null
        return;
    }
    m_var = CBotVar::Create("", var->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC));
    m_var->Copy( var );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotCStack::GetVar()
{
    return m_var.get();
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::AddVar(CBotVar* pVar)
{
    CBotCStack*    p = this;

    // returns to the father element
    while (p != nullptr && p->m_bBlock == 0) p = p->m_prev;

    if ( p == nullptr ) return;

    CBotVar**    pp = &p->m_listVar;
    while ( *pp != nullptr ) pp = &(*pp)->m_next;

    *pp = pVar;                    // added after
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::AddVar(std::unique_ptr<CBotVar> pVar)
{
    AddVar(pVar.release());
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCStack::CheckVarLocal(CBotToken* &pToken)
{
    CBotCStack*    p = this;
    std::string    name = pToken->GetString();

    while (p != nullptr)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != nullptr)
        {
            if (name == pp->GetName())
                return true;
            pp = pp->m_next;
        }
        if ( p->m_bBlock ) return false;
        p = p->m_prev;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotCStack::CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent)
{
    nIdent = 0;
    CBotTypResult val(-1);

    CBotProgram *prog = GetProgram();

    val = prog->GetExternalCalls()->CompileCall(p, nullptr, ppVars, this);
    if (val.GetType() < 0)
    {
        val = CBotFunction::CompileCall(prog->GetFunctions(), p->GetString(), ppVars, nIdent);
        if ( val.GetType() < 0 )
        {
    //        pVar = nullptr;                    // the error is not on a particular parameter
            SetError( static_cast<CBotError>(-val.GetType()), p );
            val.SetType(-val.GetType());
            return val;
        }
    }
    return val;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCStack::CheckCall(CBotToken* &pToken, CBotDefParam* pParam)
{
    std::string    name = pToken->GetString();

    CBotProgram *prog = GetProgram();

    if ( prog->GetExternalCalls()->CheckCall(name) ) return true;

    for (CBotFunction* pp : prog->GetFunctions())
    {
        if ( pToken->GetString() == pp->GetName() )
        {
            // are parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
    }

    for (CBotFunction* pp : CBotFunction::m_publicFunctions)
    {
        if ( pToken->GetString() == pp->GetName() )
        {
            // are parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
    }

    return false;
}

}
