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

#pragma once

#include "CBot/CBotToken.h"
#include "CBot/CBotDefines.h"
#include "CBot/CBotTypResult.h"
#include "CBot/CBotEnums.h"
#include "CBot/CBotUtils.h"

#include <string>
#include <memory>

namespace CBot
{

class CBotVarClass;
class CBotInstr;
class CBotClass;

class CBotVar;

/**
 * \brief A CBot variable
 */
class CBotVariable
{
public:
    //! \name Creation / destruction
    //@{

    /**
     * \brief Constructor.
     * \param name Variable name
     * \param value Initial value of variable.
     */
    CBotVariable(const CBotToken &name, std::unique_ptr<CBotVar> value);

    /**
     * \brief Constructor.
     * \param name Variable name
     * \param value Initial value of variable.
     */
    CBotVariable(const std::string &name, std::unique_ptr<CBotVar> value);

    /**
     * \brief Copies the name and type of this variable, but not its value.
     *
     * \deprecated
     * \todo TODO: REMOVE ME
     */
    static std::unique_ptr<CBotVariable> Create(CBotVariable* pVar);

    /**
     * \brief Destructor.
     */
    ~CBotVariable();

    //@}

    /// TODO REMOVE ME
    CBotVariable *m_next = nullptr;

    /// TODO DOCUMENT ME
    void Copy(CBotVariable *copyFrom);

    /**
     * \brief Generate next unique identifier
     *
     * Used by both variables (CBotVar) and functions (CBotFunction)
     * TODO REMOVE ME
     */
    static long NextUniqNum();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Variable name and type
    //@{

    /**
     * \brief Returns the name of the variable
     * \return The name of the variable, empty string if unknown
     */
    const std::string& GetName();

    /**
     * \brief SetName Changes the name of the variable
     * \param name New name
     */
    void SetName(const std::string& name);

    /**
     * \brief Returns the CBotToken this variable is associated with
     *
     * This token is either passed in CBotVar::Create() or created from name string
     */
    CBotToken* GetToken();

    //@}

    // TODO make private
    std::unique_ptr<CBotVar> m_value;

    // TODO DOCUMENT ME
    void SetVal(CBotVar *val);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Class member properties
    //! \todo TODO: make a ClassField class
    //@{

    /**
     * \brief Marks the variable as "static"
     *
     * Useful only for class members
     *
     * \param bStatic static or not
     */
    void SetStatic(bool bStatic);

    /**
     * \brief Checks if the variable is static
     *
     * Useful only for class members
     *
     * \return true for static variables
     */
    bool IsStatic();

    /**
     * \brief If this is a static class variable, return the static var from the class
     * \return Static variable from CBotClass instance if this variable is static, or this otherwise
     */
    CBotVariable* GetStaticVar();

    /**
     * \enum ProtectionLevel
     * \brief Class member protection level (public/protected/private)
     */
    enum class ProtectionLevel
    {
        Public = 0,    //!< public variable
        ReadOnly = 1,  //!< read only (can't be set from CBot, only from the engine)
        Protected = 2, //!< protected
        Private = 3    //!< private
    };

    /**
     * \brief Sets variable protection level
     *
     * Useful only for class members
     *
     * \param mPrivate New protection level
     */
    void SetPrivate(ProtectionLevel mPrivate);

    /**
     * \brief Checks if the variable is accessible at the given protection level
     *
     * This means that the variable protection level is greater or equal to given level
     *
     * \param level Protection level to check access at
     */
    bool IsPrivate(ProtectionLevel level = ProtectionLevel::Protected);

    /**
     * \brief Get variable protection level
     * \return Variable protection level
     */
    ProtectionLevel GetPrivate();

    /**
     * \brief Get the class where this variable is defined
     * @return Class in which this variable is defined, if it's a field; nullptr if not.
     */
    CBotClass *GetContainingClass() {return m_pContainingClass;}

    /**
     * \brief Set the class where this variable is defined. Only call this for fields.
     * @param val Class in which this variable is defined.
     * @param position Position of field within class.
     */
    void SetContainingClass(CBotClass *val, int position);

    /**
     * \brief Get the position of this field within the class.
     *
     * This variable must be a field. If not, this method asserts.
     *
     * @return Field position
     */
    int GetFieldPosition();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Save / restore state
    //@{

    /**
     * \brief Save common variable header (name, type, etc.)
     * \param pf file pointer
     * \return false on write error
     */
    bool SaveState(FILE* pf);

    /**
     * \brief Restore variable
     * \param pf file pointer
     * \param[out] pVar Pointer to recieve the variable
     * \return false on read error
     */
    static bool RestoreState(FILE* pf, CBotVariable* &pVar);

    //@}

    //! Expression describing initial value
    //! TODO: make private
    std::unique_ptr<CBotInstr> m_InitExpr;

    //! Expression describing array limit
    //! TODO: make private
    std::unique_ptr<CBotInstr> m_LimExpr;

private:
    CBotToken m_name;

    //! true if the variable is static (for classes)
    bool m_bStatic;
    //! Element protection level - public, protected or private (for classes)
    ProtectionLevel m_mPrivate;

    //! Next UniqNum
    static long m_identcpt;

    //! Class instance which this variable is a member of
    CBotVarClass* m_pMyThis;

    //! Class in which this variable is defined
    CBotClass *m_pContainingClass;

    //! Position of field within class
    int m_nFieldPosition;

    friend class CBotVar;
    friend class CBotVarClass;
};

/**
 * \brief A CBot variable
 *
 * \todo TODO: remove CBotLinkedList base class
 *
 * \nosubgrouping
 */
class CBotVar : public CBotLinkedList<CBotVar>
{
public:
    //! \name Creation / destruction
    //@{

    /**
     * \brief Constructor. Do not call directly, use CBotVar::Create()
     */
    CBotVar();

    /**
     * \brief Destructor.
     */
    virtual ~CBotVar();

    /**
     * \brief Creates a new value from a type described by CBotTypResult
     *
     * \todo TODO: what value does it have?
     *
     * \param type Variable type
     */
    static std::unique_ptr<CBotVar> Create(CBotTypResult type);

    /**
     * \brief Create a new variable of the same type and name as another one
     *
     * Contents of the variable are NOT copied.
     *
     * \param pVar other variable to take type and name from
     */
    static std::unique_ptr<CBotVar> Create(CBotVar* pVar);

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * \brief Associates an unique identifier to class instance
     *
     * Used only by classes
     *
     * \param UniqId New unique identifier
     * \see SetUniqNum() for another identifier, used for all variable types
     */
    virtual void SetIdent(long UniqId);

    /**
     * \brief Cast to an object. This dereferences CVarPointers.
     *
     * \return 'this', as an object (CBotVarClass*) if it is one, else nullptr.
     */
    virtual CBotVarClass *AsObject();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name User pointer
    //@{

    /**
     * \brief Set a custom pointer associated with this variable
     * \param pUser custom pointer to set
     */
    void SetUserPtr(void* pUser);

    /**
     * \brief Returns the custom pointer associated with this variable
     * \return A pointer set with SetUserPtr()
     */
    void* GetUserPtr();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Variable type
    //@{

    /**
     * \brief Mode for GetType() and GetTypResult()
     */
    enum class GetTypeMode
    {
        NORMAL = 0,
        CLASS_AS_POINTER = 1,
        CLASS_AS_INTRINSIC = 2,
    };

    /**
     * \brief GetType Returns the base type of the variable (::CBotType)
     * \param mode Mode, see GetTypeMode enum
     */
    CBotType GetType(GetTypeMode mode = GetTypeMode::NORMAL);

    /**
     * \brief Returns the complete type of the variable (CBotTypResult)
     * \param mode Mode, see GetTypeMode enum
     */
    CBotTypResult GetTypResult(GetTypeMode mode = GetTypeMode::NORMAL);

    /**
     * \brief Change type of this variable
     * \param type new type
     */
    void SetType(CBotTypResult& type);

    /**
     * \brief Set class this variable is instance of
     *
     * Used by instance variables, NOT class members
     */
    virtual void SetClass(CBotClass* pClass);

    /**
     * \brief Return class this variable is instance of
     *
     * Used by instance variables, NOT class members
     */
    virtual CBotClass* GetClass();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Initialization status
    //@{

    /**
     * \brief Variable initialization status
     *
     * \see GetInit()
     */
    enum class InitType : int
    {
        UNDEF = 0,      //!< the variable value is currently not defined
        DEF = 1,        //!< the variable value is defined
        IS_POINTER = 2, //!< the variable value is as a pointer
        IS_NAN = 999    //!< the variable value is NAN
    };

    /**
     * \brief Changes variable init status
     * \param initType New init status
     */
    void SetInit(InitType initType);

    /**
     * \brief Returns the current init state of the variable
     */
    InitType GetInit() const;

    /**
     * \brief Checks if the variable is currently "undefined"
     * \see InitType::UNDEF
     */
    bool IsUndefined() const { return GetInit() == InitType::UNDEF; }

    /**
     * \brief Checks if the variable is currently "defined"
     * \see InitType::DEF
     */
    bool IsDefined() const { return GetInit() == InitType::DEF; }

    /**
     * \brief Checks if the variable is currently NAN
     * \return InitType::NAN
     */
    bool IsNAN() const { return GetInit() == InitType::IS_NAN; }

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * \brief Called after constructor has been called
     *
     * This is used internally by the engine to mark the constructor as called.
     *
     * This allows the destructor to be called later.
     */
    virtual void ConstructorSet();

    /**
     * \brief Call the class update function
     *
     * \param pUser User pointer to pass to the update function
     * \see CBotClass::SetUpdateFunc()
     */
    virtual void Update(void* pUser);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Class / array member access
    //@{

    /**
     * \brief Returns class member by name
     * \param name Name of member to get
     * \return CBotVariable representing the class member
     */
    virtual CBotVariable* GetItem(const std::string& name);

    /**
     * \brief Returns element of the array by index
     *
     * TODO: Appears to be also implemented in CBotVarClass, but I'm not sure what is it used for there. Looks like CBotVarArray stores data internally in CBotVarClass or something like that. Needs futher investigation.
     *
     * \param index Index of the element to get
     * \param grow true to grow the array automatically if the index is out of range
     * \return CBotVar representing the array element, or nullptr on error (for example going out of range)
     */
    virtual CBotVariable* GetItem(int index, bool grow = false);

    /**
     * \brief Return all elements of this variable as a list. Works for both classes and arrays.
     * \return All elements of this variable
     */
    virtual std::vector<std::unique_ptr<CBotVariable>>& GetItemList();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * \name Value management
     *
     * Always make sure that the variable has correct type before calling these functions!
     *
     * Some variable types have multiple getters/setters and do automatic conversion.
     *
     * Using one that is not implemented will result in a failed assertion.
     */
    //@{

    operator int();
    operator float();
    operator std::string();
    void operator=(const CBotVar& var);
    void operator=(int x);
    void operator=(float x);
    void operator=(const std::string &x);

    /**
     * \brief Set the value
     * \param var Another variable to copy value from
     */
    void SetVal(CBotVar* var);

    /**
     * \brief Copy from another variable
     * \param pSrc Variable to copy from
     * \param bName true if you want to also copy the name
     */
    virtual void Copy(CBotVar* pSrc);

    /**
     * \brief Set value as an integer
     *
     * This one should be used for boolean values, too
     *
     * \param val New value
     * \param name Used when you assign a constant value - makes the value appear as "name" instead of number in the debugger
     */
    virtual void SetValInt(int val, const std::string& name = "");

    /**
     * \brief Set value as float
     * \param val New value
     */
    virtual void SetValFloat(float val);

    /**
     * \brief Set value as string
     * \param val New value
     */
    virtual void SetValString(const std::string& val);

    /**
     * \brief Get value as integer
     * \return Current value
     */
    virtual int GetValInt();

    /**
     * \brief Get value as float
     * \return Current value
     */
    virtual float GetValFloat();

    /**
     * \brief Get value as string
     *
     * This one is supported by most types of variables.
     *
     * Automatically converts the value to string if needed.
     *
     * \return Current value
     */
    virtual std::string GetValString();

    /**
     * \brief Set value for pointer types
     * \param p Variable to point to
     */
    virtual void SetPointer(CBotVar* p);

    /**
     * \brief Get value for pointer types
     * \return Variable that this variable points to
     */
    virtual CBotVarClass* GetPointer();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * \name Math operations
     *
     * All these functions operate on the "left" variable, taking "right" as the argument.
     *
     * The C++ equivalent would be the +=, -=, *=, /= etc. operations
     */
    //@{

    //! \brief Addition
    virtual void Add(CBotVar* left, CBotVar* right);
    //! \brief Subtraction
    virtual void Sub(CBotVar* left, CBotVar* right);
    //! \brief Multiplication
    virtual void Mul(CBotVar* left, CBotVar* right);
    //! \brief Division
    virtual CBotError Div(CBotVar* left, CBotVar* right);
    //! \brief Modulo (remainder of division)
    virtual CBotError Modulo(CBotVar* left, CBotVar* right);
    //! \brief Power
    virtual void Power(CBotVar* left, CBotVar* right);

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Comparation functions
    //@{

    //! \brief left < right
    virtual bool Lo(CBotVar* left, CBotVar* right);
    //! \brief left > right
    virtual bool Hi(CBotVar* left, CBotVar* right);
    //! \brief left <= right
    virtual bool Ls(CBotVar* left, CBotVar* right);
    //! \brief left >= right
    virtual bool Hs(CBotVar* left, CBotVar* right);
    //! \brief left == right
    virtual bool Eq(CBotVar* left, CBotVar* right);
    //! \brief left != right
    virtual bool Ne(CBotVar* left, CBotVar* right);

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * \name Logical or bitwise functions
     *
     * Can be either depending on variable type.
     *
     * For boolean, those are logical functions, for int they are bitwise.
     */

    //! \brief left && right or left & right
    virtual void And(CBotVar* left, CBotVar* right);
    //! \brief left || right or left | right
    virtual void Or(CBotVar* left, CBotVar* right);
    //! \brief left ^ right (also for boolean!)
    virtual void XOr(CBotVar* left, CBotVar* right);
    //!\brief !this or ~this
    virtual void Not();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Bitwise shift
    //@{

    //! \brief left >> right
    virtual void ASR(CBotVar* left, CBotVar* right);
    //! \brief left >>> right
    /**
     * This is unsigned shift to right
     */
    virtual void SR(CBotVar* left, CBotVar* right);
    //! \brief left << right
    virtual void SL(CBotVar* left, CBotVar* right);

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Negation / increment / decrement
    //@{

    //! \brief -this
    virtual void Neg();
    //! \brief ++this
    virtual void Inc();
    //! \brief --this
    virtual void Dec();

    //@}


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Save / restore state
    //@{

    /**
     * \brief Save variable data
     *
     * Overriden in child classes
     *
     * \param pf file pointer
     * \return false on write error
     */
    bool Save0State(FILE* pf);
    virtual bool Save1State(FILE* pf) = 0;

    /**
     * \brief Load variable data
     *
     * TODO change to unique_ptr
     */
    static bool RestoreState(FILE *pf, CBotVar*& outValue);

    //@}

protected:
    //! Type of value.
    CBotTypResult m_type;
    //! Initialization status
    InitType m_binit;
    //! User pointer if specified
    /**
     * \see SetUserPtr()
     * \see GetUserPtr()
     */
    void* m_pUserPtr;

    friend class CBotStack;
    friend class CBotCStack;
    friend class CBotInstrCall;
    friend class CBotProgram;
    friend class CBotClass;
    friend class CBotVarClass;
    friend class CBotVarPointer;
    friend class CBotVarArray;
};

} // namespace CBot
