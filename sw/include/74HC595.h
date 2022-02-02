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

#ifndef _74HC595_H
#define _74HC595_H

#define _74HC595_MAX_SPI_CLOCK 100000000UL // 100 MHz

#include <stdint.h>

/**
@brief Driver for serial --> parallel shift register 74HC595
@tparam SPIMaster SPI master driver class implementing a static method put(uint8_t)
@tparam SSPin Pin driver class implementing static methods high() and low()
@tparam t_nofDevices Number of daisy-chained 74HC595 devices (default is one device)
*/
template <typename SPIMaster, typename SSPin, uint8_t t_nofDevices = 1>
class _74HC595
{
    public:

    /**
    @brief Get the number of daisy-chained 74HC595 devices
    @result Number of daisy-chained devices
    */
    static constexpr uint8_t getNofDevices()
    {
        return t_nofDevices;
    }

    /**
    @brief Put data to device
    @param data Data to be sent to device
    @note Make sure data has sufficient length of t_nofDevices bytes!
    */
    static void put(const uint8_t * const data)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Transmit Data
        putDaisyChain(data);
        
        // Disable device (active low)
        SSPin::high();
    }
    
    private:

    // Previous device in daisy chain should be allowed to call putDaisyChain()
    friend class _74HC595<SPIMaster, SSPin, t_nofDevices+1>;
    
    // Next device in daisy chain
    typedef _74HC595<SPIMaster, SSPin, t_nofDevices-1> NextDevice;

    // This is the actual SPI output method
    // Data for all daisy-chained devices is transferred by static loop unrolling
    static void putDaisyChain(const uint8_t * const data)
    {
        // SPI transmission
        SPIMaster::put(*data);

        // Iterate to next device
        NextDevice::putDaisyChain(data+1);
    }
};

/**
@brief Driver for serial --> parallel shift register 74HC595. Specialization for one device
@tparam SPIMaster SPI master driver class implementing a static method put(uint8_t)
@tparam SSPin Pin driver class implementing static methods high() and low()
*/
template <typename SPIMaster, typename SSPin>
class _74HC595<SPIMaster, SSPin, 1>
{
    public:

    /**
    @brief Get the number of daisy-chained 74HC595 devices
    @result Number of daisy-chained devices
    */
    static constexpr uint8_t getNofDevices()
    {
        return 1;
    }

    /**
    @brief Put data to device
    @param data Data to be sent to device
    @note Make sure data has sufficient length of t_nofDevices bytes!
    */
    static void put(const uint8_t * data)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Write Data
        putDaisyChain(data);
        
        // Disable device (active low)
        SSPin::high();
    }

    /**
    @brief Put data to device
    @param data Data to be sent to device
    */
    static void put(const uint8_t data)
    {
        put(&data);
    }
    
    private:
    
    // Previous device in daisy chain should be allowed to call putDaisyChain()
    friend class _74HC595<SPIMaster, SSPin, 2>;
    
    // This is the actual SPI output method
    // Data for all daisy-chained devices is transferred by static loop unrolling
    static void putDaisyChain(const uint8_t * const data)
    {
        // SPI transmission
        SPIMaster::put(*data);
    }
};

/**
@brief Driver for serial --> parallel shift register 74HC595. Specialization for no device
@tparam SPIMaster SPI master driver class implementing a static method put(uint8_t)
@tparam SSPin Pin driver class implementing static methods high() and low()
*/
template <typename SPIMaster, typename SSPin>
class _74HC595<SPIMaster, SSPin, 0>;

#endif