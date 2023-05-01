/*
Copyright (C) 2022  Andreas Lagler

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _74HC148_H
#define _74HC148_H

#include <stdint.h>
#include <stdbool.h>
#include "subject.h"

/**
@brief Driver for 8-to-3 line priority decoder 74HC148
@tparam Port Any 3-pin GPIOSubPort driver class implementing static methods setAsInput() and read()
@tparam PinConfig
*/
template <typename Port>
class _74HC148
{
    public:
    /**
    @brief Get the number of decoder output lines
    @result Number of decoder output lines
    */
    static constexpr uint8_t getNofLines()
    {
        return 8;
    }

    /**
    @brief get active input line
    @result zero-based index of active input line (0..7)
    @note Maximum specified settling time is 45ns @5V, so an additional delay is not needed
    */
    static uint8_t getLine()
    {
        return Port::read();
    }
    
    /// @brief Initialization
    static void init()
    {
        static_assert(Port::getNofPins() == 3, "Invalid port configuration. Number of pins must be 3");
        Port::setAsInput();
    }
};

#endif