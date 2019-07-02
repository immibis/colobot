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

#include "CBot/CBotInstr/CBotInstr.h"

namespace CBot
{

/**
 * \brief A call to class method - var.func()
 */
class CBotInstrMethode : public CBotInstr
{
public:
    CBotInstrMethode();
    ~CBotInstrMethode();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \param varType Type of object method is being called on
     * \param bMethodChain If true, allows chaining methods only
     * \param bIsSuperCall If true, don't do a virtual call
     * \return
     */
    static std::unique_ptr<CBotInstr> Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult varType, bool bMethodChain, bool bIsSuperCall);

    /*!
     * \brief Execute
     * \param pj
     * \return
     */
    bool Execute(CBotStack* &pj) override;

    /*!
     * \brief ExecuteVar Execute the method call.
     * \param pVar
     * \param pj
     * \param prevToken
     * \param bStep
     * \param bExtend
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep, bool bExtend) override;

    /*!
     * \brief RestoreStateVar
     * \param pj
     * \param bMain
     */
    void RestoreStateVar(CBotStack* &pj, bool bMain) override;

protected:
    virtual const std::string GetDebugName() override { return "CBotInstrMethode"; }
    virtual std::string GetDebugData() override;
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! The parameters to be evaluated.
    std::unique_ptr<CBotInstr> m_parameters;
    //! Complete type of the result.
    CBotTypResult m_typRes;
    //! Name of the method.
    std::string m_methodName;
    //! Identifier of the method.
    long m_MethodeIdent;
    //! Name of the class.
    std::string m_className;
    //! True if we shouldn't do a virtual call (e.g. true for "super" calls)
    bool m_bNonVirtualCall;

    //! Instruction to return a member of the returned object.
    std::unique_ptr<CBotInstr> m_exprRetVar;

};

} // namespace CBot
