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


#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotStack.h"

#include "CBot/CBotInstr/CBotInstr.h"
#include "CBot/CBotVar/CBotVarArray.h"
#include "CBot/CBotVar/CBotVarPointer.h"
#include "CBot/CBotVar/CBotVarClass.h"
#include "CBot/CBotVar/CBotVarBoolean.h"
#include "CBot/CBotVar/CBotVarString.h"
#include "CBot/CBotVar/CBotVarFloat.h"
#include "CBot/CBotVar/CBotVarInt.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotToken.h"

#include "CBot/CBotEnums.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>


namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
long CBotVariable::m_identcpt = 0;

////////////////////////////////////////////////////////////////////////////////
CBotVariable::CBotVariable(const CBotToken &name, std::unique_ptr<CBotVar> value)
    : m_value(std::move(value)), m_name(name)
{
    m_next = nullptr;
    m_InitExpr = nullptr;
    m_LimExpr = nullptr;
    m_ident = 0;
    m_bStatic = false;
    m_mPrivate = ProtectionLevel::Public;
    m_pMyThis = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable::CBotVariable(const std::string &name, std::unique_ptr<CBotVar> value)
    : m_value(std::move(value)), m_name(name)
{
    m_next = nullptr;
    m_InitExpr = nullptr;
    m_LimExpr = nullptr;
    m_ident = 0;
    m_bStatic = false;
    m_mPrivate = ProtectionLevel::Public;
    m_pMyThis = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable::~CBotVariable()
{
    delete m_next;
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotVariable> CBotVariable::Create(CBotVariable *pVar)
{
    return std::unique_ptr<CBotVariable>(new CBotVariable(pVar->m_name, CBotVar::Create(pVar->m_value.get())));
}

////////////////////////////////////////////////////////////////////////////////
CBotVar::CBotVar()
{
    m_pUserPtr = nullptr;
    m_type  = -1;
    m_binit = InitType::UNDEF;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar::~CBotVar( )
{
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::ConstructorSet()
{
    // nop
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetUserPtr(void* pUser)
{
    m_pUserPtr = pUser;
    if (m_type.Eq(CBotTypPointer) &&
        (static_cast<CBotVarPointer*>(this))->m_pVarClass != nullptr )
        (static_cast<CBotVarPointer*>(this))->m_pVarClass->SetUserPtr(pUser);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetIdent(long n)
{
    if (m_type.Eq(CBotTypPointer) &&
        (static_cast<CBotVarPointer*>(this))->m_pVarClass != nullptr )
        (static_cast<CBotVarPointer*>(this))->m_pVarClass->SetIdent(n);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVariable::SetUniqNum(long n)
{
    m_ident = n;

    if ( n == 0 ) assert(0);
}

////////////////////////////////////////////////////////////////////////////////
long CBotVariable::NextUniqNum()
{
    if (++m_identcpt < 10000) m_identcpt = 10000;
    return m_identcpt;
}

////////////////////////////////////////////////////////////////////////////////
long CBotVariable::GetUniqNum()
{
    return m_ident;
}

////////////////////////////////////////////////////////////////////////////////
void* CBotVar::GetUserPtr()
{
    return m_pUserPtr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVariable::SaveState(FILE* pf)
{
    // TODO
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVariable::RestoreState(FILE *pf, CBotVariable*& pVar)
{
    // TODO
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Save1State(FILE* pf)
{
    // this routine "virtual" must never be called,
    // there must be a routine for each of the subclasses (CBotVarInt, CBotVarFloat, etc)
    // ( see the type in m_type )
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Update(void* pUser)
{
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotVar> CBotVar::Create(CBotTypResult type)
{
    switch (type.GetType())
    {
    case CBotTypShort:
    case CBotTypInt:
        return std::unique_ptr<CBotVar>(new CBotVarInt());
    case CBotTypFloat:
        return std::unique_ptr<CBotVar>(new CBotVarFloat());
    case CBotTypBoolean:
        return std::unique_ptr<CBotVar>(new CBotVarBoolean());
    case CBotTypString:
        return std::unique_ptr<CBotVar>(new CBotVarString());
    case CBotTypPointer:
    case CBotTypNullPointer:
        return std::unique_ptr<CBotVar>(new CBotVarPointer(type));
    case CBotTypIntrinsic:
        return std::unique_ptr<CBotVar>(new CBotVarClass(type));

    case CBotTypClass:
        // creates a new instance of a class
        // and returns the POINTER on this instance
        {
            CBotVarClass* instance = new CBotVarClass(type);
            CBotVarPointer* pointer = new CBotVarPointer(type);
            pointer->SetPointer( instance );
            return std::unique_ptr<CBotVar>(pointer);
        }

    case CBotTypArrayPointer:
        return std::unique_ptr<CBotVar>(new CBotVarArray(type));

    case CBotTypArrayBody:
        {
            CBotVarClass* instance = new CBotVarClass(type);
            CBotVarArray* array = new CBotVarArray(type);
            array->SetPointer( instance );

            CBotVar*    pv = array;
            while (type.Eq(CBotTypArrayBody))
            {
                type = type.GetTypElem();
                pv = (static_cast<CBotVarArray*>(pv))->GetItem(0, true)->m_value.get();            // creates at least the element [0]
            }

            return std::unique_ptr<CBotVar>(array);
        }
    }

    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotVar> CBotVar::Create( CBotVar* pVar )
{
    return Create(pVar->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC));
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotVar::GetTypResult(GetTypeMode mode)
{
    CBotTypResult    r = m_type;

    if ( mode == GetTypeMode::CLASS_AS_POINTER && m_type.Eq(CBotTypClass) )
        r.SetType(CBotTypPointer);
    if ( mode == GetTypeMode::CLASS_AS_INTRINSIC && m_type.Eq(CBotTypClass) )
        r.SetType(CBotTypIntrinsic);

    return r;
}

////////////////////////////////////////////////////////////////////////////////
CBotType CBotVar::GetType(GetTypeMode mode)
{
    if ( mode == GetTypeMode::CLASS_AS_POINTER && m_type.Eq(CBotTypClass) )
        return CBotTypPointer;
    if ( mode == GetTypeMode::CLASS_AS_INTRINSIC && m_type.Eq(CBotTypClass) )
        return CBotTypIntrinsic;
    return static_cast<CBotType>(m_type.GetType());
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetType(CBotTypResult& type)
{
    m_type = type;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar::InitType CBotVar::GetInit() const
{
    if ( m_type.Eq(CBotTypClass) ) return InitType::DEF;        // always set!

    return m_binit;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetInit(CBotVar::InitType initType)
{
    m_binit = initType;
    if (initType == CBotVar::InitType::IS_POINTER ) m_binit = CBotVar::InitType::DEF;                    // cas spécial

    if ( m_type.Eq(CBotTypPointer) && initType == CBotVar::InitType::IS_POINTER )
    {
        CBotVarClass* instance = GetPointer();
        if ( instance == nullptr )
        {
            instance = new CBotVarClass(m_type);
//            instance->SetClass((static_cast<CBotVarPointer*>(this))->m_classes);
            SetPointer(instance);
        }
        instance->SetInit(CBotVar::InitType::DEF);
    }

    if ( m_type.Eq(CBotTypClass) || m_type.Eq(CBotTypIntrinsic) )
    {
        for(std::unique_ptr<CBotVariable> &p : (static_cast<CBotVarClass*>(this))->m_pVar)
        {
            p->m_value->SetInit(initType);
            p->m_pMyThis = static_cast<CBotVarClass*>(this);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
const std::string& CBotVariable::GetName()
{
    return    m_name.GetString();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVariable::SetName(const std::string& name)
{
    m_name.SetString(name);
}

////////////////////////////////////////////////////////////////////////////////
CBotToken* CBotVariable::GetToken()
{
    return    &m_name;
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable* CBotVar::GetItem(const std::string& name)
{
    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable* CBotVar::GetItemRef(int nIdent)
{
    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<std::unique_ptr<CBotVariable>>& CBotVar::GetItemList()
{
    assert(0);
    // TODO: clean up dummy code
    static std::vector<std::unique_ptr<CBotVariable>> nullItemList;
    nullItemList.clear();
    return nullItemList;
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable* CBotVar::GetItem(int index, bool grow)
{
    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable* CBotVariable::GetStaticVar()
{
    // makes the pointer to the variable if it is static
    if ( m_bStatic == 0 || m_pMyThis == nullptr ) return this;

    CBotClass*    pClass = m_pMyThis->GetClass();
    return pClass->GetItem( m_name.GetString() );
}

////////////////////////////////////////////////////////////////////////////////
void CBotVariable::SetVal(CBotVar* var)
{
    m_value->SetVal(var);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVariable::Copy(CBotVariable* copyFrom)
{
    m_value->Copy(copyFrom->m_value.get());
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetVal(CBotVar* var)
{
    switch (var->GetType())
    {
    case CBotTypBoolean:
        SetValInt(var->GetValInt());
        break;
    case CBotTypInt:
        SetValInt(var->GetValInt(), (static_cast<CBotVarInt*>(var))->m_defnum);
        break;
    case CBotTypFloat:
        SetValFloat(var->GetValFloat());
        break;
    case CBotTypString:
        SetValString(var->GetValString());
        break;
    case CBotTypPointer:
    case CBotTypNullPointer:
    case CBotTypArrayPointer:
        SetPointer(var->GetPointer());
        break;
    case CBotTypClass:
        {
            (static_cast<CBotVarClass*>(this))->m_pVar.clear();
            Copy(var);
        }
        break;
    default:
        assert(0);
    }

    m_binit = var->m_binit;        // copie l'état nan s'il y a
}

////////////////////////////////////////////////////////////////////////////////
void CBotVariable::SetStatic(bool bStatic)
{
    m_bStatic = bStatic;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVariable::SetPrivate(ProtectionLevel mPrivate)
{
    m_mPrivate = mPrivate;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVariable::IsStatic()
{
    return m_bStatic;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVariable::IsPrivate(ProtectionLevel level)
{
    return static_cast<int>(m_mPrivate) >= static_cast<int>(level);
}

////////////////////////////////////////////////////////////////////////////////
CBotVariable::ProtectionLevel CBotVariable::GetPrivate()
{
    return m_mPrivate;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetPointer(CBotVar* pVarClass)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVar::GetPointer()
{
    assert(0);
    return nullptr;
}

// All these functions must be defined in the subclasses
// derived from class CBotVar
////////////////////////////////////////////////////////////////////////////////
int CBotVar::GetValInt()
{
    assert(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
float CBotVar::GetValFloat()
{
    assert(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetValInt(int c, const std::string& s)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetValFloat(float c)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Mul(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Power(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotVar::Div(CBotVar* left, CBotVar* right)
{
    assert(0);
    return CBotNoErr;
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotVar::Modulo(CBotVar* left, CBotVar* right)
{
    assert(0);
    return CBotNoErr;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Add(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Sub(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Lo(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Hi(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Ls(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Hs(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Eq(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Ne(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::And(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Or(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::XOr(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::ASR(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SR(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SL(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Neg()
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Not()
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Inc()
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Dec()
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Copy(CBotVar* pSrc)
{
    //if (bName) m_token = pSrc->m_token; // TODO remove commented code
    m_type = pSrc->m_type;
    m_binit = pSrc->m_binit;
//-    m_bStatic    = pSrc->m_bStatic;
    m_next = nullptr;
    m_pUserPtr = pSrc->m_pUserPtr;

    // keeps indentificator the same (by default)
    // TODO remove commented code
    //if (m_ident == 0) m_ident = pSrc->m_ident;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetValString(const std::string& val)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVar::GetValString()
{
    assert(0);
    return std::string();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetClass(CBotClass* pClass)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotVar::GetClass()
{
    assert(0);
    return nullptr;
}

CBotVar::operator int()
{
    return GetValInt();
}

CBotVar::operator float()
{
    return GetValFloat();
}

CBotVar::operator std::string()
{
    return GetValString();
}

void CBotVar::operator=(const CBotVar &var)
{
    SetVal(const_cast<CBotVar*>(&var));
}

void CBotVar::operator=(int x)
{
    SetValInt(x);
}

void CBotVar::operator=(float x)
{
    SetValFloat(x);
}

void CBotVar::operator=(const std::string &x)
{
    SetValString(x);
}

} // namespace CBot
