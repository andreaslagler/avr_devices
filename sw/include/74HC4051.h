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

#ifndef _74HC4051_H
#define _74HC4051_H

/**
@brief Driver for single 8 channel analog multiplexer 74HC4051
@tparam Port Any 3-pin GPIOSubPort driver class implementing static methods setAsOutput() and write(uint8_t)
*/
template<typename Port>
class _74HC4051
{
    public:
    
    /**
    @brief Get the number of multiplexer channels
    @result Number of multiplexer channels
    */
    static constexpr uint8_t getNofChannels()
    {
        return 8;
    }
    
    /**
    @brief Select multiplexer channel
    @param channel Selected multiplexer channel
    @note Maximum specified settling time is 63ns @5V, so an additional delay is not needed
    */
    static void selectChannel(uint8_t channel) __attribute__((always_inline))
    {
        Port::write(channel);
    }
};

#endif
