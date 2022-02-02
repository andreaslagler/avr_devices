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

#ifndef DSPIC33_H
#define DSPIC33_H

#include <stdint.h>

#define DSPIC33_MAX_SPI_CLOCK 10000000UL // 10 MHz

/**
@brief Driver for 8 bit parameter transfer to dsPIC33 via SPI
@tparam SPIMaster SPI master driver class implementing static methods put(uint8_t)
@tparam SSPin Pin driver class implementing static methods high() and low()
 */
template <
typename SPIMaster,
typename SSPin>
class DSPIC33
{
    public:

    /**
    @brief Write a parameter to DSP
    @param address Address of parameter to be sent to device
    @param value Value of parameter to be sent to device
    @note Address byte + data byte will be handled as one 16 bit word on the dsPIC33, so access to a received parameter will always be atomic
    */
    static void write(const uint8_t address, const uint8_t value)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Transfer parameter address
        SPIMaster::put(address);
        
        // Transfer parameter value
        SPIMaster::put(value);
        
        // Disable device (active low)
        SSPin::high();
    }
};

#endif