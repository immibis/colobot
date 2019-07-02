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
 * \brief Instruction "case", part of "switch" structure
 *
 * \see CBotSwitch
 */
class CBotCase : public CBotInstr
{

public:
    CBotCase();
    ~CBotCase();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \return
     */
    static std::unique_ptr<CBotInstr> Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute Execution of instruction "case".
     * \param pj
     * \return
     */
    bool Execute(CBotStack* &pj) override;

    /*!
     * \brief RestoreState
     * \param pj
     * \param bMain
     */
    void RestoreState(CBotStack* &pj, bool bMain) override;

    /*!
     * \brief CompCase Routine to find the entry point of "case" corresponding
     * to the value seen.
     * \param pj
     * \param val
     * \return
     */
    bool CompCase(CBotStack* &pj, int val) override;

protected:
    virtual const std::string GetDebugName() override { return "CBotCase"; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! Value to compare.
    std::unique_ptr<CBotInstr> m_value;
};

} // namespace CBot
