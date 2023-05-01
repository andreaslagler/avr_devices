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

#ifndef LINE_DECODER_H
#define LINE_DECODER_H

/**
@brief Driver for N-to-(2^N) line decoder (e.g. 74HC138)
@tparam Port Any GPIOPort/GPIOSubPort driver class implementing static methods getNofPins(), setAsOutput() and write(uint8_t)
*/
template <typename Port>
class LineDecoder
{
    public:

    /**
    @brief Get the number of decoder output lines
    @result Number of decoder output lines
    */
    static constexpr uint8_t nofLines()
    {
        return 1 << nofInputLines();
    }

    /// @brief Initialization
    static void init()
    {
        Port::setAsOutput();
    }
    
    /**
    @brief Select output line
    @param line Selected decoder line
    @note Maximum specified settling time is 45ns @5V, so an additional delay is not needed
    */
    static void selectLine(const uint8_t line)
    {
        Port::write(line);
    }
    
    private:
    
    static constexpr uint8_t nofInputLines()
    {
        return Port::getNofPins();
    }
};

#endif