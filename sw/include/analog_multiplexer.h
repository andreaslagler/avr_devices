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

#ifndef ANALOG_MULTIPLEXER_H
#define ANALOG_MULTIPLEXER_H

/**
@brief Driver for 1 to N analog multiplexer
@tparam Port Any GPIOPort/GPIOSubPort driver class implementing static methods getNofPins(), setAsOutput() and write(uint8_t)
*/
template<typename Port>
class AnalogMultiplexer
{
    public:
    
    /**
    @brief Get the number of multiplexer channels
    @result Number of multiplexer channels
    */
    static constexpr uint8_t getNofChannels()
    {
        return 1 << nofInputLines();
    }
    
    /// @brief Initialization
    static void init()
    {
        Port::setAsOutput();
    }
    
    /**
    @brief Select multiplexer channel
    @param channel Selected multiplexer channel
    */
    static void selectChannel(uint8_t channel) __attribute__((always_inline))
    {
        Port::write(channel);
    }
    
    private:
    
    static constexpr uint8_t nofInputLines()
    {
        return Port::getNofPins();
    }

};

#endif
