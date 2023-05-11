/*
Copyright (C) 2023  Andreas Lagler

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

#ifndef LINE_ENCODER_H
#define LINE_ENCODER_H

/**
@brief Driver for (2^N)-to-N line encoder (e.g. 74HC148)
@tparam Port Any GPIOPort/GPIOSubPort driver class implementing static methods getNofPins(), setAsOutput() and read()
*/
template <typename Port>
class LineEncoder
{
    public:
    /**
    @brief Get the number of decoder output lines
    @result Number of decoder output lines
    */
    static constexpr uint8_t getNofLines()
    {
        return 1 << nofOutputLines();
    }

    /**
    @brief get active input line
    @result zero-based index of active input line
    @note Maximum specified settling time is 45ns @5V, so an additional delay is not needed
    */
    static uint8_t getLine()
    {
        return Port::read();
    }
    
    /// @brief Initialization
    static void init()
    {
        Port::setAsInput();
    }
    
    private:
    
    static constexpr uint8_t nofOutputLines()
    {
        return Port::getNofPins();
    }};

#endif