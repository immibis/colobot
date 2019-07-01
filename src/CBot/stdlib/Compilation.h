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

#include "CBot/CBotTypResult.h"
#include <vector>

namespace CBot
{

class CBotVar;

// Commonly used functions for parameter compilation
// type "number" is anything > CBotTypDouble

// float foo()
CBotTypResult cNull(const std::vector<CBotTypResult> &var, void* user);
// float foo(number)
CBotTypResult cOneFloat(const std::vector<CBotTypResult> &var, void* user);
// float foo(number, number)
CBotTypResult cTwoFloat(const std::vector<CBotTypResult> &var, void* user);
// float foo(string)
CBotTypResult cString(const std::vector<CBotTypResult> &var, void* user);
// string foo(string)
CBotTypResult cStringString(const std::vector<CBotTypResult> &var, void* user);
// int foo(int)
CBotTypResult cOneInt(const std::vector<CBotTypResult> &var, void* user);
// bool foo(int)
CBotTypResult cOneIntReturnBool(const std::vector<CBotTypResult> &var, void* user);


// string foo(string)
CBotTypResult cStrStr(const std::vector<CBotTypResult> &var, void* user);
// int foo(string, string)
CBotTypResult cIntStrStr(const std::vector<CBotTypResult> &var, void* user);
// float foo(string)
CBotTypResult cFloatStr(const std::vector<CBotTypResult> &var, void* user);
// string foo(string, number[, number])
CBotTypResult cStrStrIntInt(const std::vector<CBotTypResult> &var, void* user);
// string foo(string, number)
CBotTypResult cStrStrInt(const std::vector<CBotTypResult> &var, void* user);
// int foo(string)
CBotTypResult cIntStr(const std::vector<CBotTypResult> &var, void* user);

} // namespace CBot
