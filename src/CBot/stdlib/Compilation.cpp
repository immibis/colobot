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

#include "CBot/stdlib/Compilation.h"

#include "CBot/CBot.h"

namespace CBot
{

// Compiling a procedure without any parameters.

CBotTypResult cNull(const std::vector<CBotTypResult> &var, void* user)
{
    if ( var.size() > 0 )  return CBotErrOverParam;
    return CBotTypFloat;
}

// Compiling a procedure with a single real number.

CBotTypResult cOneFloat(const std::vector<CBotTypResult> &var, void* user)
{
    if ( var.size() < 1 )  return CBotTypResult(CBotErrLowParam);
    if ( var.size() > 1 )  return CBotTypResult(CBotErrOverParam);
    if ( var[0].GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);

    return CBotTypFloat;
}

// Compiling a procedure with two real numbers.

CBotTypResult cTwoFloat(const std::vector<CBotTypResult> &var, void* user)
{
    if ( var.size() < 2 )  return CBotTypResult(CBotErrLowParam);
    if ( var.size() > 2 )  return CBotTypResult(CBotErrOverParam);

    if ( var[0].GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    if ( var[1].GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);

    return CBotTypFloat;
}


// Compiling a procedure with a single string.

CBotTypResult cString(const std::vector<CBotTypResult> &var, void* user)
{
    if ( var.size() < 1 )  return CBotTypResult(CBotErrLowParam);
    if ( var.size() > 1 )  return CBotTypResult(CBotErrOverParam);

    if ( var[0].GetType() != CBotTypString &&
         var[0].GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);

    return CBotTypFloat;
}

// Compiling a procedure with a single string, returning string.

CBotTypResult cStringString(const std::vector<CBotTypResult> &var, void* user)
{
    if ( var.size() < 1 )  return CBotTypResult(CBotErrLowParam);
    if ( var.size() > 1 )  return CBotTypResult(CBotErrOverParam);
    if ( var[0].GetType() != CBotTypString &&
         var[0].GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);

    return CBotTypString;
}

// compilation of instruction with one int returning int

CBotTypResult cOneInt(const std::vector<CBotTypResult> &var, void* user)
{
    if ( var.size() < 1 )  return CBotTypResult(CBotErrLowParam);
    if ( var.size() > 1 )  return CBotTypResult(CBotErrOverParam);
    if ( var[0].GetType() != CBotTypInt )  return CBotTypResult(CBotErrBadNum);
    return CBotTypInt;
}

// compilation of instruction with one int returning boolean

CBotTypResult cOneIntReturnBool(const std::vector<CBotTypResult> &var, void* user)
{
    if ( var.size() < 1 )  return CBotTypResult(CBotErrLowParam);
    if ( var.size() > 1 )  return CBotTypResult(CBotErrOverParam);
    if ( var[0].GetType() != CBotTypInt )  return CBotTypResult(CBotErrBadNum);
    return CBotTypBoolean;
}



CBotTypResult cStrStr(const std::vector<CBotTypResult> &var, void* user)
{
    // it takes a parameter
    if ( var.size() < 1 )  return CBotTypResult(CBotErrLowParam);
    // no second parameter
    if ( var.size() > 1 )  return CBotTypResult(CBotErrOverParam);

    // to be a string
    if ( var[0].GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // the end result is a string
    return CBotTypString;
}

CBotTypResult cIntStrStr(const std::vector<CBotTypResult> &var, void* user)
{
    // it takes a parameter
    // it takes a second parameter
    if ( var.size() < 2 )  return CBotTypResult(CBotErrLowParam);
    // no third parameter
    if ( var.size() > 2 )  return CBotTypResult(CBotErrOverParam);

    // to be a string
    if ( var[0].GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // to be a string
    if ( var[1].GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // the end result is a number
    return CBotTypInt;
}

CBotTypResult cFloatStr(const std::vector<CBotTypResult> &var, void* user)
{
    // it takes a parameter
    if ( var.size() < 1 )  return CBotTypResult(CBotErrLowParam);
    // no second parameter
    if ( var.size() > 1 )  return CBotTypResult(CBotErrOverParam);

    // to be a string
    if ( var[0].GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // the end result is a number
    return CBotTypFloat;
}

CBotTypResult cStrStrIntInt(const std::vector<CBotTypResult> &var, void* user)
{
    // it takes a parameter
    // it takes a second parameter
    if ( var.size() < 2 ) return CBotTypResult( CBotErrLowParam );

    // to be a string
    if ( var[0].GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // it takes a second parameter
    // which must be a number
    if ( var[1].GetType() > CBotTypDouble )
        return CBotTypResult( CBotErrBadNum );

    // third parameter optional
    if ( var.size() > 2 )
    {
        // which must be a number
        if ( var[2].GetType() > CBotTypDouble )
            return CBotTypResult( CBotErrBadNum );

        // no fourth parameter
        if ( var.size() > 3 ) return CBotTypResult( CBotErrOverParam );
    }

    // the end result is a string
    return CBotTypString;
}

CBotTypResult cStrStrInt(const std::vector<CBotTypResult> &var, void* user)
{
    // it takes a parameter
    // it takes a second parameter
    if ( var.size() < 2 ) return CBotTypResult( CBotErrLowParam );
    // no third parameter
    if ( var.size() > 2 ) return CBotTypResult( CBotErrOverParam );

    // to be a string
    if ( var[0].GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // it takes a second parameter
    // which must be a number
    if ( var[1].GetType() > CBotTypDouble )
        return CBotTypResult( CBotErrBadNum );

    // the end result is a string
    return CBotTypString;
}

CBotTypResult cIntStr(const std::vector<CBotTypResult> &var, void* user)
{
    // it takes a parameter
    if ( var.size() < 1 ) return CBotTypResult( CBotErrLowParam );
    // no second parameter
    if ( var.size() > 1 ) return CBotTypResult( CBotErrOverParam );

    // to be a string
    if ( var[0].GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadParam );

    // the end result is an integer
    return CBotTypInt;
}

} // namespace CBot
