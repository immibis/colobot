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

#include "CBot/CBotVar/CBotVar.h"
#include "CBot/CBotProgram.h"

#include <memory>

namespace CBot
{

class CBotInstr;
class CBotDefParam;
class CBotToken;

/*!
 * \brief The CBotCStack class Management of the stack of compilation.
 *
 * This is used to keep track of local variables, and also (for some reason) to pass CBotVars up the call stack.
 * Generally each function on the call stack of the recursive descent parser has an associated instance of
 * CBotCStack which holds some context for that rule. Each instance of CBotCStack is called a stack level.
 * Each level holds a pointer to the adjacent levels outside (prev) and inside it (next). It's not clear why the next pointer is needed.
 *
 * Levels where m_bBlock is true correspond to blocks in the source code, and can have local variables associated with them.
 * When a variable declaration is encountered, the variable gets added to the innermost enclosing block.
 *
 * The TokenStack method, which must be called on the innermost level, creates and returns a new inner level.
 * The Return or ReturnFunc method can be used to destroy the innermost level and go back to the level outside it.
 *
 * Each stack level has a m_var which is used for various purposes.
 * It's not clear why we stash these into the stack instead of returning them.
 * The CBotCStack has ownership of the object m_var points to.
 *
 * Each stack level has an error start position, end position and error type.
 * It's not clear why we stash these into the stack instead of returning them or throwing an exception.
 * For some reason, two of these are static.
 *
 * There's also the current function's return type, m_retTyp, and the current program, m_prog.
 *
 * Typically, a parser function will take the current stack level as a parameter, create its own stack level,
 * and delete its own stack level when returning.
 * Typically, the function will return a pointer to some object, or if there's an error, it will set the error
 * in the stack (which propagates up the stack) and return null.
 *
 * The compile stack also maintains labels for all loops/switches that are currently being compiled.
 * This is used to check whether a 'break' or 'continue' statement has a valid label.
 */
class CBotCStack
{

    /*!
     * \brief Creates a stack level.
     *
     * \link TokenStack(CBotToken*, bool) \endlink is used to create new stack levels under existing levels, and
     * \link BeginNewStack(CBotProgram*) \endlink is used to create the first level of a new stack.
     * This constructor is only used internally.
     *
     * \param pParent Parent stack level
     */
    CBotCStack(CBotCStack* pParent);

public:

    /*!
     * \brief Creates a new stack.
     *
     * \param pProgram Pointer to the program
     */
    static std::unique_ptr<CBotCStack> BeginNewStack(CBotProgram *pProgram);

    /*!
     * \brief Destructor.
     */
    ~CBotCStack();

    /*!
     * \brief Deletes all stack levels below this one. Errors and vars are not passed up to this level.
     */
    void DeleteChildLevels();

    /*!
     * \brief Checks if an error occurred
     *
     * \return True if no error occurred
     */
    bool IsOk();

    /*!
     * \brief Get error type
     *
     * \return Error type
     */
    CBotError GetError();

    /*!
     * \brief Get error type and location
     *
     * \param start [out] Start location of error
     * \param end   [out] End location of error
     * \return Error type
     */
    CBotError GetError(int& start, int& end);

    /*!
     * \brief Set m_var's type.
     *
     * This is identical to GetVar()->\link CBotVar::SetType(CBotTypResult&) SetType(type)\endlink, unless m_var is null in which case it does nothing.
     *
     * \param type New type
     */
    void SetType(CBotTypResult& type);

    /*!
     * \brief Gets type of m_var.
     *
     * This is identical to GetVar()->\link CBotVar::GetTypResult(CBotVar::GetTypeMode) GetTypResult(mode)\endlink unless m_var is null.
     * If m_var is null, this returns nullptr.
     *
     * \param mode
     * \return m_var's type
     */
    CBotTypResult GetTypResult(CBotVar::GetTypeMode mode = CBotVar::GetTypeMode::NORMAL);

    /*!
     * \brief Gets type of m_var.
     *
     * This is identical to GetVar()->\link CBotVar::GetType(CBotVar::GetTypeMode) GetType(mode)\endlink unless m_var is null.
     * If m_var is null, this returns nullptr.
     *
     * \param mode
     * \return m_var's type
     */
    int GetType(CBotVar::GetTypeMode mode = CBotVar::GetTypeMode::NORMAL);

    /*!
     * \brief Gets type of m_var.
     *
     * This is identical to GetVar()->\link CBotVar::GetClass() GetClass()\endlink if m_var is not null and has a class type.
     * Otherwise, this returns nullptr.
     *
     * \return m_var's class
     */
    CBotClass* GetClass();

    /*!
     * \brief Adds a local variable.
     *
     * If this stack level is not a block, it walks up the stack and adds the variable to the nearest enclosing block.
     * Takes ownership of p.
     *
     * \param p The new variable.
     */
    void AddVar(CBotVariable* p);

    /*!
     * \brief Adds a local variable.
     *
     * If this stack level is not a block, it walks up the stack and adds the variable to the nearest enclosing block.
     * Takes ownership of p.
     *
     * \param p The new variable.
     */
    void AddVar(std::unique_ptr<CBotVariable> p);

    /*!
     * \brief Finds a local variable by name.
     *
     * The token type is ignored as long as p->\link CBotToken::GetString() GetString()\endlink returns a variable name.
     * \param p Token to search
     * \return Variable found, or null if not found.
     */
    CBotVariable* FindVar(CBotToken* &p);

    /*!
     * \brief Finds a local variable by name.
     *
     * The token type is ignored as long as Token.\link CBotToken::GetString() GetString()\endlink returns a variable name.
     * \param Token Token to search
     * \return Variable found, or null if not found.
     */
    CBotVariable* FindVar(CBotToken& Token);

    /*!
     * \brief Test whether a local variable is already defined.
     *
     * The token type is ignored as long as pToken->\link CBotToken::GetString() GetString()\endlink returns a variable name.
     * This only searches the innermost scope. It is used to see whether a variable can be defined, and shadowing is allowed.
     *
     * \param pToken Token to search
     * \return True if a variable with the same name is already defined in this scope.
     */
    bool CheckVarLocal(CBotToken* &pToken);

    /*!
     * \brief Finds a variable by name.
     *
     * This calls \link FindVar(CBotToken&) \endlink and if found, makes a copy.
     *
     * \param Token
     * \return Copy of found variable, or null if not found.
     */
    std::unique_ptr<CBotVariable> CopyVar(CBotToken& Token);

    /*!
     * \brief Creates a new stack level.
     *
     * \param pToken If not null, the child's startError is pToken->\link CBotToken::GetStart() GetStart()\endlink
     * \param bBlock Whether the new level starts a block.
     * \return
     */
    CBotCStack* TokenStack(CBotToken* pToken = nullptr, bool bBlock = false);

    /*!
     * \brief Exits a stack level.
     *
     * Moves var and error from pChild to this.
     * Then deletes pChild.
     * pChild must be the innermost level and this must be its parent.
     *
     * \param p       Not used except as the return value from this function.
     * \param pChild  Child stack level.
     * \return        Same value passed as first parameter.
     */
    CBotInstr* Return(CBotInstr* p, CBotCStack* pChild);

    /*!
     * \brief Exits a stack level.
     *
     * Moves var and error from pChild to this.
     * Then deletes pChild.
     * pChild must be the innermost level and this must be its parent.
     * Same as Return but passes through a CBotFunction* instead of a CBotInstr*
     *
     * \param p       Not used except as the return value from this function.
     * \param pChild  Child stack level.
     * \return        Same value passed as first parameter
     */
    CBotFunction* ReturnFunc(CBotFunction* p, CBotCStack* pChild);

    /*!
     * \brief Sets m_var.
     *
     * Sets m_var to var. Deletes any previous m_var.
     *
     * \param var
     */
    void SetVar( std::unique_ptr<CBotVar> var );

    /*!
     * \brief Sets m_var.
     *
     * Sets m_var to a copy of var. Deletes any previous m_var. Does not takes ownership of var.
     * var must not be null, or else m_var is left in an invalid state!
     *
     * \param var
     */
    void SetCopyVar( CBotVar* var );

    /*!
     * \brief Gets m_var.
     *
     * \return Value of m_var, previously set by one of the SetVar or Return functions.
     */
    CBotVar* GetVar();

    /*!
     * \brief Set error start location.
     *
     * If an error is already set, does nothing.
     * \param pos Error start location in source code
     */
    void SetStartError(int pos);

    /*!
     * \brief Set error type and end location.
     *
     * If we are setting an error and an error is already set, does nothing.
     * If we are setting no error (n == 0) and an error is already set, unsets the error.
     *
     * \param n   Error type
     * \param pos Error end location in source code
     */
    void SetError(CBotError n, int pos);

    /*!
     * \brief Set error type and location.
     *
     * If an error is already set, does nothing.
     *
     * \param n Error type
     * \param p Token containing error (sets start and end location)
     */
    void SetError(CBotError n, CBotToken* p);

    /*!
     * \brief Set error type and location.
     *
     * If an error is already set, overwrites it.
     *
     * \param n     Error type
     * \param start Error start location in source code
     * \param end   Error end location in source code
     */
    void ResetError(CBotError n, int start, int end);

    /*!
     * \brief Sets m_retTyp.
     *
     * \param type New value for m_retTyp.
     */
    void SetRetType(CBotTypResult& type);

    /*!
     * \brief Gets m_retTyp.
     *
     * \return Value of m_retTyp (previously set with \link SetRetType(CBotTypResult&) \endlink somewhere up the stack)
     */
    CBotTypResult GetRetType();

    /*!
     * \brief Gets the current program.
     *
     * \return Value of m_pProgram (set as parameter to \link BeginNewStack(CBotProgram*)\endlink when creating the top stack level)
     */
    CBotProgram* GetProgram();

    /*!
     * \brief CompileCall
     * \param p
     * \param ppVars
     * \param nIdent
     * \return
     */
    CBotTypResult CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent);

    /*!
     * \brief CheckCall Test if a procedure name is already defined somewhere.
     *
     * \param pToken
     * \param pParam
     * \return
     */
    bool CheckCall(CBotToken* &pToken, CBotDefParam* pParam);

    /*!
     * \brief Advance token pointer.
     *
     * Updates p to point to the next token after p.
     * If there is a next token, returns true.
     * If there is no next token, sets p to null, sets the error, and returns false.
     * In all normal circumstances this should return true.
     *
     * \param p Current token pointer
     * \return Whether the token was updated.
     */
    bool NextToken(CBotToken* &p);

    /*!
     * Marks this stack level as a loop and sets the loop label.
     *
     * \param label The label for the loop; empty string if no label.
     */
    void SetLoop(std::string label);

    /*!
     * Unmarks this stack level as a loop.
     */
    void ClearLoop();

    /**
     * \brief Check validity of break or continue.
     *
     * \param label Label to check for. Empty string ("") if no label.
     * \param type ID_BREAK or ID_CONTINUE
     * \return True if inside a loop with that label. Switches count for ID_BREAK but not for ID_CONTINUE.
     *
     * \todo Switch statements should be able to have labels.
     */
    bool CheckLoop(const std::string& label, int type);

private:
    CBotCStack* m_next;
    CBotCStack* m_prev;

    CBotCStack* m_toplevel; // Outermost level of current stack.

    CBotError m_error;
    int m_end;
    int m_start;

    //! Result of the operations.
    std::unique_ptr<CBotVar> m_var;
    //! Is part of a block (variables are local to this block).
    bool m_bBlock;
    CBotVariable* m_listVar;
    //! List of compiled functions.
    static CBotTypResult m_retTyp; // XXX shouldn't be static

    CBotProgram* m_pProgram; // Only for outermost stack level

    /*! Label for this loop.
     * Set to dummy value "#none" if this is not a loop.
     * Set to "#SWITCH" for a switch statement (for some reason, perhaps because they can't be labelled?)
     * If the loop has a label, this is set to it. An empty string means an unlabelled loop.
     */
    std::string m_loopLabel;
};

} // namespace CBot
