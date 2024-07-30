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

#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H


#include <stdint.h>

/**
@brief Driver for shift register connected to SPI master
@tparam SPIMaster SPI master driver class implementing a static method put(uint8_t)
@tparam SSPin Pin driver class implementing static methods high() and low()
@tparam t_nofBytes Number of bytes
*/
template <typename SPIMaster, typename SSPin, uint8_t t_nofBytes = 1>
class ShiftRegister
{
    public:

    /**
    @brief Get the width of the shift register
    @result Number of bytes
    */
    static consteval uint8_t getNofBytes()
    {
        return t_nofBytes;
    }

    /**
    @brief Put data to shift register
    @param data Data to be sent to shift register
    @note Make sure data has sufficient length of t_nofBytes bytes!
    */
    static void put(const uint8_t * const data)
    {
        // Enable shift register (active low)
        SSPin::low();
        
        // Transmit Data
        putNextByte(data);
        
        // Disable shift register (active low)
        SSPin::high();
    }
    
    private:

    // Previous shift register in daisy chain should be allowed to call putNextByte()
    friend class ShiftRegister<SPIMaster, SSPin, t_nofBytes+1>;
    
    // Next shift register in daisy chain
    typedef ShiftRegister<SPIMaster, SSPin, t_nofBytes-1> Next;

    // This is the actual SPI output method
    // Data for all daisy-chained devices is transferred by static loop unrolling
    static void putNextByte(const uint8_t * const data)
    {
        // SPI transmission
        SPIMaster::put(*data);

        // Iterate to next shift register
        Next::putNextByte(data+1);
    }
};

/**
@brief Driver for serial --> parallel shift register 74HC595. Specialization for one shift register
@tparam SPIMaster SPI master driver class implementing a static method put(uint8_t)
@tparam SSPin Pin driver class implementing static methods high() and low()
*/
template <typename SPIMaster, typename SSPin>
class ShiftRegister<SPIMaster, SSPin, 1>
{
    public:

    /**
    @brief Get the number of daisy-chained 74HC595 devices
    @result Number of daisy-chained devices
    */
    static consteval uint8_t getNofBytes()
    {
        return 1;
    }

    /**
    @brief Put data to shift register
    @param data Data to be sent to shift register
    @note Make sure data has sufficient length of t_nofBytes bytes!
    */
    static void put(const uint8_t * data)
    {
        // Enable shift register (active low)
        SSPin::low();
        
        // Write Data
        putNextByte(data);
        
        // Disable shift register (active low)
        SSPin::high();
    }

    /**
    @brief Put data to shift register
    @param data Data to be sent to shift register
    */
    static void put(const uint8_t data)
    {
        put(&data);
    }
    
    private:
    
    // Previous shift register in daisy chain should be allowed to call putNextByte()
    friend class ShiftRegister<SPIMaster, SSPin, 2>;
    
    // This is the actual SPI output method
    // Data for all daisy-chained devices is transferred by static loop unrolling
    static void putNextByte(const uint8_t * const data)
    {
        // SPI transmission
        SPIMaster::put(*data);
    }
};

/**
@brief Driver for serial --> parallel shift register 74HC595. Specialization for no shift register
@tparam SPIMaster SPI master driver class implementing a static method put(uint8_t)
@tparam SSPin Pin driver class implementing static methods high() and low()
*/
template <typename SPIMaster, typename SSPin>
class ShiftRegister<SPIMaster, SSPin, 0>;

#endif