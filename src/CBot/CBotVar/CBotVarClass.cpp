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

#include "CBot/CBotVar/CBotVarClass.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotDefines.h"

#include "CBot/CBotFileUtils.h"

#include "CBot/CBotInstr/CBotInstr.h"

#include <cassert>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
std::set<CBotVarClass*> CBotVarClass::m_instances{};

////////////////////////////////////////////////////////////////////////////////
CBotVarClass::CBotVarClass(const CBotTypResult& type)
{
    if ( !type.Eq(CBotTypClass)        &&
         !type.Eq(CBotTypIntrinsic)    &&                // by convenience there accepts these types
         !type.Eq(CBotTypPointer)      &&
         !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) assert(0);

    m_next        = nullptr;
    m_pUserPtr    = OBJECTCREATED;//nullptr;
    m_type        = type;
    if ( type.Eq(CBotTypArrayPointer) )    m_type.SetType( CBotTypArrayBody );
    else if ( !type.Eq(CBotTypArrayBody) ) m_type.SetType( CBotTypClass );
                                                 // officel type for this object

    m_pClass    = nullptr;
    m_pParent    = nullptr;
    m_binit        = InitType::UNDEF;
    m_bConstructor = false;
    m_CptUse    = 0;
    m_ItemIdent = type.Eq(CBotTypIntrinsic) ? 0 : CBotVariable::NextUniqNum();

    // add to the list
    m_instances.insert(this);

    CBotClass* pClass = type.GetClass();
    if ( pClass != nullptr && pClass->GetParent() != nullptr )
    {
        // also creates an instance of the parent class
        m_pParent = new CBotVarClass( CBotTypResult(type.GetType(), pClass->GetParent()) ); //, nIdent);
    }

    SetClass( pClass );

}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass::~CBotVarClass( )
{
    if ( m_CptUse != 0 )
        assert(0);

    if ( m_pParent ) delete m_pParent;
    m_pParent = nullptr;

    // removes the class list
    m_instances.erase(this);
}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass *CBotVarClass::AsObject()
{
    return this;
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable *CBotVarClass::GetObjectField(int position)
{
    assert(position >= 0);
    assert(static_cast<size_t>(position) < m_pVar.size());
    return m_pVar[position].get();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::ConstructorSet()
{
    m_bConstructor = true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::Copy(CBotVar* pSrc)
{
    pSrc = pSrc->GetPointer();                    // if source given by a pointer

    if ( pSrc->GetType() != CBotTypClass )
        assert(0);

    CBotVarClass*    p = static_cast<CBotVarClass*>(pSrc);

    m_type        = p->m_type;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_pClass    = p->m_pClass;
    if ( p->m_pParent )
    {
        assert(0);       // "que faire du pParent";
    }

//    m_next        = nullptr;
    m_pUserPtr    = p->m_pUserPtr;
    m_ItemIdent = p->m_ItemIdent;

    m_pVar.clear();

    for (std::unique_ptr<CBotVariable> &pv : p->m_pVar)
    {
        std::unique_ptr<CBotVariable> pn = CBotVariable::Create(pv.get());
        pn->m_value->Copy( pv->m_value.get() );
        m_pVar.emplace_back(std::move(pn));
    }
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::SetIdent(long n)
{
    m_ItemIdent = n;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::SetClass(CBotClass* pClass)
{
    m_type.m_class = pClass;

    if ( m_pClass == pClass ) return;

    m_pClass = pClass;

    // initializes the variables associated with this class
    m_pVar.clear();

    if (pClass == nullptr) return;

    InitFieldsForClass(pClass);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::InitFieldsForClass(CBotClass *pClass)
{
    if (pClass->GetParent() != nullptr)
        InitFieldsForClass(pClass->GetParent());

    for (std::unique_ptr<CBotVariable> &pv : pClass->GetVar())
    {
        // seeks the maximum dimensions of the table
        CBotInstr*    p  = pv->m_LimExpr.get();                     // the different formulas
        if ( p != nullptr )
        {
            CBotStack* pile = CBotStack::AllocateStack();    // an independent stack
            int     n = 0;
            int     max[100];

            while (p != nullptr)
            {
                while( pile->IsOk() && !p->Execute(pile) ) ;        // calculate size without interruptions
                CBotVar*    v = pile->GetVar();                     // result
                max[n] = v->GetValInt();                            // value
                n++;
                p = p->GetNext3();
            }
            while (n<100) max[n++] = 0;

            pv->m_value->m_type.SetArray(max);                      // stores the limitations
            pile->Delete();
        }

        std::unique_ptr<CBotVariable> pn = CBotVariable::Create( pv.get() );        // a copy
        pn->SetStatic(pv->IsStatic());
        pn->SetPrivate(pv->GetPrivate());

        if ( pv->m_InitExpr != nullptr )                // expression for initialization?
        {
#if    STACKMEM
            CBotStack* pile = CBotStack::AllocateStack();    // an independent stack

            while(pile->IsOk() && !pv->m_InitExpr->Execute(pile, pn->m_value.get()));    // evaluates the expression without timer

            pile->Delete();
#else
            CBotStack* pile = new CBotStack(nullptr);     // an independent stack
            while(!pv->m_InitExpr->Execute(pile));    // evaluates the expression without timer
            pn->SetVal( pile->GetVar() ) ;
            delete pile;
#endif
        }

        pn->m_pMyThis = this;

        m_pVar.emplace_back(std::move(pn));
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotVarClass::GetClass()
{
    return    m_pClass;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::Update(void* pUser)
{
    // retrieves the user pointer according to the class
    // or according to the parameter passed to CBotProgram::Run()

    if ( m_pUserPtr != nullptr) pUser = m_pUserPtr;
    if ( pUser == OBJECTDELETED ||
         pUser == OBJECTCREATED ) return;
    m_pClass->Update(this, pUser);
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable* CBotVarClass::GetItem(const std::string& name)
{
    // Test names in reverse order, so that derived class fields are found first.
    for (auto iter = m_pVar.rbegin(); iter != m_pVar.rend(); iter++)
    {
        if ( (*iter)->GetName() == name ) return (*iter).get();
    }

    // TODO: remove m_pParent
    if ( m_pParent != nullptr ) return m_pParent->GetItem(name);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable* CBotVarClass::GetItem(int n, bool bExtend)
{
    if ( n < 0 ) return nullptr;
    if ( m_type.GetLimite() >= 0 && n >= m_type.GetLimite() ) return nullptr;
    if ( n >= static_cast<int>(m_pVar.size())) {
        if (bExtend && n <= MAXARRAYSIZE) {
            int oldsize = m_pVar.size();
            m_pVar.resize(n+1);
            for (int k = oldsize; k <= n; k++) {
                m_pVar[k].reset(new CBotVariable("", CBotVar::Create(m_type.GetTypElem())));
            }
        } else {
            return nullptr;
        }
    }

    return m_pVar[n].get();
}

////////////////////////////////////////////////////////////////////////////////
std::vector<std::unique_ptr<CBotVariable>>& CBotVarClass::GetItemList()
{
    return m_pVar;
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarClass::GetValString()
{
    std::string    res;

    if ( m_pClass != nullptr )                        // not used for an array
    {
        res = m_pClass->GetName() + std::string("( ");

        CBotVarClass*    my = this;
        while ( my != nullptr )
        {
            for (std::unique_ptr<CBotVariable> &pv : my->m_pVar)
            {
                res += pv->GetName() + std::string("=");

                if ( pv->IsStatic() )
                {
                    CBotVar* pvv = my->m_pClass->GetItem(pv->GetName())->m_value.get();
                    res += pvv->GetValString();
                }
                else
                {
                    res += pv->m_value->GetValString();
                }
                if (&pv != &my->m_pVar.back()) // not last element
                    res += ", ";
            }
            my = my->m_pParent;
            if ( my != nullptr )
            {
                res += ") extends ";
                res += my->m_pClass->GetName();
                res += " (";
            }
        }

        res += " )";
    }
    else
    {
        res = "{ ";

        for (std::unique_ptr<CBotVariable> &pv : m_pVar)
        {
            res += pv->m_value->GetValString();
            if (&pv != &m_pVar.back()) // not last element
                res += ", ";
        }

        res += " }";
    }

    return    res;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::IncrementUse()
{
    m_CptUse++;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::DecrementUse()
{
    m_CptUse--;
    if ( m_CptUse == 0 )
    {
        // if there is one, call the destructor
        // but only if a constructor had been called.
        if ( m_bConstructor )
        {
            m_CptUse++;    // does not return to the destructor

            // m_error is static in the stack
            // saves the value for return
            CBotError err;
            int start, end;
            CBotStack*    pile = nullptr;
            err = pile->GetError(start,end);    // stack == nullptr it does not bother!

            pile = CBotStack::AllocateStack();        // clears the error
            CBotVar*    ppVars[1];
            ppVars[0] = nullptr;

            std::unique_ptr<CBotVar> pThis  = CBotVar::Create(CBotTypNullPointer);
            pThis->SetPointer(this);

            std::string    nom = std::string("~") + m_pClass->GetName();
            long        ident = 0;

            CBotToken token(nom); // TODO

            while ( pile->IsOk() && !m_pClass->ExecuteMethode(ident, pThis.get(), ppVars, CBotTypResult(CBotTypVoid), pile, &token)) ;    // waits for the end

            pile->ResetError(err, start,end);

            pile->Delete();

            // Must be before m_CptUse--, so that when this next line calls DecrementUse again it will
            // see the object is still in use. Otherwise, we get in an infinite loop of calling the destructor.
            pThis.reset();

            m_CptUse--;
        }

        delete this; // self-destructs!
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVarClass::GetPointer()
{
    return this;
}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVarClass::Find(long id)
{
    for (CBotVarClass* p : m_instances)
    {
        if (p->m_ItemIdent == id) return p;
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarClass::Eq(CBotVar* left, CBotVar* right)
{
    // TODO: shouldn't we check the type is the same?
    std::vector<std::unique_ptr<CBotVariable>> &l = left->GetItemList();
    std::vector<std::unique_ptr<CBotVariable>> &r = right->GetItemList();

    auto itl = l.begin(), itr = r.begin();
    while (itl != l.end() && itr != r.end())
    {
        if ((*itl)->m_value->Ne((*itl)->m_value.get(), (*itr)->m_value.get()))
            return false;
        itl++;
        itr++;
    }

    return itl == l.end() && itr == r.end();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarClass::Ne(CBotVar* left, CBotVar* right)
{
    return !Eq(left, right);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarClass::Save1State(FILE* pf)
{
    if ( !WriteType(pf, m_type) ) return false;
    if ( !WriteLong(pf, m_ItemIdent) ) return false;

    return SaveVars(pf, m_pVar);                                // content of the object
}

} // namespace CBot
