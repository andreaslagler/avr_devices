/*
Copyright (C) 2022 Andreas Lagler

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MCP23S17_H
#define MCP23S17_H

#include <stdint.h>
#include <stdbool.h>
#include "MCP23XXX.h"

#define MCP23S17_MAX_SPI_CLOCK 10000000UL // 10 MHz

/**
@brief MCP23S17 pin index A0..7 and B0..7
*/
enum class MCP23S17PinIdx
{
    B0 = 0,
    B1 = 1,
    B2 = 2,
    B3 = 3,
    B4 = 4,
    B5 = 5,
    B6 = 6,
    B7 = 7,
    A0 = 8,
    A1 = 9,
    A2 = 10,
    A3 = 11,
    A4 = 12,
    A5 = 13,
    A6 = 14,
    A7 = 15
};

/**
@brief MCP23S17 Pin Configuration
Each used GP I/O pin of a MCP23S17 device has to be assigned a Pin Type
@tparam t_pinIdx Index A0..7 and B0..7 of a used GP I/O pin
@tparam t_pinType Pin type assigned to pin index
*/
template <MCP23S17PinIdx t_pinIdx, MCP23xxxPinType t_pinType>
struct MCP23S17PinConfig
{
    static constexpr MCP23S17PinIdx s_pinIdx = t_pinIdx;
    static constexpr MCP23xxxPinType s_pinType = t_pinType;
};

template <typename T>
concept DrvSPIMaster = requires(uint8_t a)
{
    T::put(a);
    a = T::get();
};

template <typename T>
concept DrvGPIOPin = requires
{
     T::high();
     T::low();
};

template <typename T>
concept PinConfiguration = requires
{
    T::s_pinIdx;
    T::s_pinType;
};

/**
@brief Driver for SPI port expander MCP23S17
@tparam SPIMaster SPI master driver class implementing static methods put(uint8_t) and get()
@tparam SSPin Pin driver class implementing static methods high() and low()
@tparam PinConfig Pack of MCP23S17PinConfig specializations for all used GP I/O pins
*/
template <
DrvSPIMaster SPIMaster,
DrvGPIOPin SSPin,
PinConfiguration ... PinConfig>
class MCP23S17 : MCP23xxx
{
    private:
    
    /**
    @brief Register-level driver class for MCP23S17 GP I/O pin
    @tparam t_pinIdx Pin index A0..A7 and B0..B7
    */
    template <MCP23S17PinIdx t_pinIdx, bool t_portA = (static_cast<uint8_t>(t_pinIdx) & 0b1000)>
    class PinRegisterAccess;

    /**
    @brief Register-level driver class for MCP23S17 GP I/O pin. Specialization for pins on I/O port A
    @tparam t_pinIdx Pin index A0..A7 and B0..B7
    */
    template <MCP23S17PinIdx t_pinIdx>
    class PinRegisterAccess<t_pinIdx, true>
    {
        protected:
        
        static bool readINTCAP() __attribute__((always_inline))
        {
            return readRegister(INTCAPA) & getBitmask();
        }
        
        static bool readGPIO() __attribute__((always_inline))
        {
            return readRegister(GPIOA) & getBitmask();
        }
        
        static void writeOLATA(const bool bValue)
        {
            if (bValue)
            {
                writeRegister(OLATA, readRegister(OLATA) | getBitmask());
            }
            else
            {
                writeRegister(OLATA, readRegister(OLATA) & (~getBitmask()));
            }
        }
        
        private:
        
        static constexpr uint8_t getBitmask()
        {
            static_assert(static_cast<uint8_t>(t_pinIdx) < 16, "Invalid pin number");
            return _BV(static_cast<uint8_t>(t_pinIdx) & 0b111);
        }
    };

    /**
    @brief Register-level driver class for MCP23S17 GP I/O pin. Specialization for pins on I/O port B
    @tparam t_pinIdx Pin index A0..A7 and B0..B7
    */
    template <MCP23S17PinIdx t_pinIdx>
    class PinRegisterAccess<t_pinIdx, false>
    {
        protected:
        
        static bool readINTCAP() __attribute__((always_inline))
        {
            return readRegister(INTCAPB) & getBitmask();
        }
        
        static bool readGPIO() __attribute__((always_inline))
        {
            return readRegister(GPIOB) & getBitmask();
        }
        
        static void writeOLAT(const bool bValue)
        {
            if (bValue)
            {
                writeRegister(OLATB, readRegister(OLATB) | getBitmask());
            }
            else
            {
                writeRegister(OLATB, readRegister(OLATB) & (~getBitmask()));
            }
        }
        
        private:
        
        static constexpr uint8_t getBitmask()
        {
            static_assert(static_cast<uint8_t>(t_pinIdx) < 16, "Invalid pin number");
            return _BV(static_cast<uint8_t>(t_pinIdx) & 0b111);
        }
    };
    
    // Struct retrieving the pin type from the pin configuration for a given pin index
    template <MCP23S17PinIdx t_pinIdx, typename CurrentPinConfig, typename ... NextPinConfig>
    struct PinIdxToPinType
    {
        // Search the parameter pack for configuration of given pin index
        static constexpr MCP23xxxPinType value = (t_pinIdx == CurrentPinConfig::s_pinIdx) ? CurrentPinConfig::s_pinType : PinIdxToPinType<t_pinIdx, NextPinConfig ...>::value;
    };
    
    // Struct retrieving the pin type from the pin configuration for a given pin index
    // Termination for last configured pin
    template <MCP23S17PinIdx t_pinIdx, typename CurrentPinConfig>
    struct PinIdxToPinType <t_pinIdx, CurrentPinConfig>
    {
        // If a configuration for given pin index does not exist, the pin is considered as unused
        static constexpr MCP23xxxPinType value = (t_pinIdx == CurrentPinConfig::s_pinIdx) ? CurrentPinConfig::s_pinType : MCP23xxxPinType::UNUSED;
    };
    
    public:

    /**
    @brief Pin interface combining pin type and register access for selected pin
    @tparam t_pinIdx Pin index A0..A7 and B0..B7
    */
    template <MCP23S17PinIdx t_pinIdx>
    using Pin = MCP23xxx::Pin<PinRegisterAccess<t_pinIdx>, PinIdxToPinType<t_pinIdx, PinConfig ...>::value>;

    /**
    @brief Initialization
    */
    static void init(const bool intActiveHigh = true)
    {
        checkConfig(); // Will evaluate at compile time and assert in case something is wrong with the pin configuration

        // Interrupt output active high --> set INTPOL bit
        uint8_t valueIOCON = _BV(SEQOP) | _BV(MIRROR) | (intActiveHigh ? _BV(INTPOL) : 0);
        writeRegister(IOCON, valueIOCON);
        
        writeRegisterPair(IODIRA,
        (Pin<MCP23S17PinIdx::B0>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B0)) : 0) +
        (Pin<MCP23S17PinIdx::B1>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B1)) : 0) +
        (Pin<MCP23S17PinIdx::B2>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B2)) : 0) +
        (Pin<MCP23S17PinIdx::B3>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B3)) : 0) +
        (Pin<MCP23S17PinIdx::B4>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B4)) : 0) +
        (Pin<MCP23S17PinIdx::B5>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B5)) : 0) +
        (Pin<MCP23S17PinIdx::B6>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B6)) : 0) +
        (Pin<MCP23S17PinIdx::B7>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B7)) : 0) +
        (Pin<MCP23S17PinIdx::A0>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A0)) : 0) +
        (Pin<MCP23S17PinIdx::A1>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A1)) : 0) +
        (Pin<MCP23S17PinIdx::A2>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A2)) : 0) +
        (Pin<MCP23S17PinIdx::A3>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A3)) : 0) +
        (Pin<MCP23S17PinIdx::A4>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A4)) : 0) +
        (Pin<MCP23S17PinIdx::A5>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A5)) : 0) +
        (Pin<MCP23S17PinIdx::A6>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A6)) : 0) +
        (Pin<MCP23S17PinIdx::A7>::s_IODIRBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A7)) : 0));
        
        writeRegisterPair(IPOLA,
        (Pin<MCP23S17PinIdx::B0>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B0)) : 0) +
        (Pin<MCP23S17PinIdx::B1>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B1)) : 0) +
        (Pin<MCP23S17PinIdx::B2>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B2)) : 0) +
        (Pin<MCP23S17PinIdx::B3>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B3)) : 0) +
        (Pin<MCP23S17PinIdx::B4>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B4)) : 0) +
        (Pin<MCP23S17PinIdx::B5>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B5)) : 0) +
        (Pin<MCP23S17PinIdx::B6>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B6)) : 0) +
        (Pin<MCP23S17PinIdx::B7>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B7)) : 0) +
        (Pin<MCP23S17PinIdx::A0>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A0)) : 0) +
        (Pin<MCP23S17PinIdx::A1>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A1)) : 0) +
        (Pin<MCP23S17PinIdx::A2>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A2)) : 0) +
        (Pin<MCP23S17PinIdx::A3>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A3)) : 0) +
        (Pin<MCP23S17PinIdx::A4>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A4)) : 0) +
        (Pin<MCP23S17PinIdx::A5>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A5)) : 0) +
        (Pin<MCP23S17PinIdx::A6>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A6)) : 0) +
        (Pin<MCP23S17PinIdx::A7>::s_IPOLBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A7)) : 0));
        
        writeRegisterPair(GPINTENA,
        (Pin<MCP23S17PinIdx::B0>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B0)) : 0) +
        (Pin<MCP23S17PinIdx::B1>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B1)) : 0) +
        (Pin<MCP23S17PinIdx::B2>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B2)) : 0) +
        (Pin<MCP23S17PinIdx::B3>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B3)) : 0) +
        (Pin<MCP23S17PinIdx::B4>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B4)) : 0) +
        (Pin<MCP23S17PinIdx::B5>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B5)) : 0) +
        (Pin<MCP23S17PinIdx::B6>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B6)) : 0) +
        (Pin<MCP23S17PinIdx::B7>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B7)) : 0) +
        (Pin<MCP23S17PinIdx::A0>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A0)) : 0) +
        (Pin<MCP23S17PinIdx::A1>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A1)) : 0) +
        (Pin<MCP23S17PinIdx::A2>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A2)) : 0) +
        (Pin<MCP23S17PinIdx::A3>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A3)) : 0) +
        (Pin<MCP23S17PinIdx::A4>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A4)) : 0) +
        (Pin<MCP23S17PinIdx::A5>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A5)) : 0) +
        (Pin<MCP23S17PinIdx::A6>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A6)) : 0) +
        (Pin<MCP23S17PinIdx::A7>::s_GPINTENBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A7)) : 0));
        
        writeRegisterPair(DEFVALA,
        (Pin<MCP23S17PinIdx::B0>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B0)) : 0) +
        (Pin<MCP23S17PinIdx::B1>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B1)) : 0) +
        (Pin<MCP23S17PinIdx::B2>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B2)) : 0) +
        (Pin<MCP23S17PinIdx::B3>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B3)) : 0) +
        (Pin<MCP23S17PinIdx::B4>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B4)) : 0) +
        (Pin<MCP23S17PinIdx::B5>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B5)) : 0) +
        (Pin<MCP23S17PinIdx::B6>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B6)) : 0) +
        (Pin<MCP23S17PinIdx::B7>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B7)) : 0) +
        (Pin<MCP23S17PinIdx::A0>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A0)) : 0) +
        (Pin<MCP23S17PinIdx::A1>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A1)) : 0) +
        (Pin<MCP23S17PinIdx::A2>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A2)) : 0) +
        (Pin<MCP23S17PinIdx::A3>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A3)) : 0) +
        (Pin<MCP23S17PinIdx::A4>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A4)) : 0) +
        (Pin<MCP23S17PinIdx::A5>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A5)) : 0) +
        (Pin<MCP23S17PinIdx::A6>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A6)) : 0) +
        (Pin<MCP23S17PinIdx::A7>::s_DEFVALBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A7)) : 0));
        
        writeRegisterPair(INTCONA,
        (Pin<MCP23S17PinIdx::B0>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B0)) : 0) +
        (Pin<MCP23S17PinIdx::B1>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B1)) : 0) +
        (Pin<MCP23S17PinIdx::B2>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B2)) : 0) +
        (Pin<MCP23S17PinIdx::B3>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B3)) : 0) +
        (Pin<MCP23S17PinIdx::B4>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B4)) : 0) +
        (Pin<MCP23S17PinIdx::B5>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B5)) : 0) +
        (Pin<MCP23S17PinIdx::B6>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B6)) : 0) +
        (Pin<MCP23S17PinIdx::B7>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B7)) : 0) +
        (Pin<MCP23S17PinIdx::A0>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A0)) : 0) +
        (Pin<MCP23S17PinIdx::A1>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A1)) : 0) +
        (Pin<MCP23S17PinIdx::A2>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A2)) : 0) +
        (Pin<MCP23S17PinIdx::A3>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A3)) : 0) +
        (Pin<MCP23S17PinIdx::A4>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A4)) : 0) +
        (Pin<MCP23S17PinIdx::A5>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A5)) : 0) +
        (Pin<MCP23S17PinIdx::A6>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A6)) : 0) +
        (Pin<MCP23S17PinIdx::A7>::s_INTCONBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A7)) : 0));
        
        writeRegisterPair(GPPUA,
        (Pin<MCP23S17PinIdx::B0>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B0)) : 0) +
        (Pin<MCP23S17PinIdx::B1>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B1)) : 0) +
        (Pin<MCP23S17PinIdx::B2>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B2)) : 0) +
        (Pin<MCP23S17PinIdx::B3>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B3)) : 0) +
        (Pin<MCP23S17PinIdx::B4>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B4)) : 0) +
        (Pin<MCP23S17PinIdx::B5>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B5)) : 0) +
        (Pin<MCP23S17PinIdx::B6>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B6)) : 0) +
        (Pin<MCP23S17PinIdx::B7>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::B7)) : 0) +
        (Pin<MCP23S17PinIdx::A0>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A0)) : 0) +
        (Pin<MCP23S17PinIdx::A1>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A1)) : 0) +
        (Pin<MCP23S17PinIdx::A2>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A2)) : 0) +
        (Pin<MCP23S17PinIdx::A3>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A3)) : 0) +
        (Pin<MCP23S17PinIdx::A4>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A4)) : 0) +
        (Pin<MCP23S17PinIdx::A5>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A5)) : 0) +
        (Pin<MCP23S17PinIdx::A6>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A6)) : 0) +
        (Pin<MCP23S17PinIdx::A7>::s_GPPUBit ? _BV(static_cast<uint16_t>(MCP23S17PinIdx::A7)) : 0));
        
        reArmInterrupt();
    }
    
    /**
    @brief Callback for MCP23xxx interrupt
    */
    static void onInterrupt() __attribute__((always_inline))
    {
        // Read interrupt flags and propagate interrupt to corresponding pin class
        const uint16_t INTF = readRegisterPair(INTFA);
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::B0)))
        {
            Pin<MCP23S17PinIdx::B0>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::B1)))
        {
            Pin<MCP23S17PinIdx::B1>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::B2)))
        {
            Pin<MCP23S17PinIdx::B2>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::B3)))
        {
            Pin<MCP23S17PinIdx::B3>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::B4)))
        {
            Pin<MCP23S17PinIdx::B4>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::B5)))
        {
            Pin<MCP23S17PinIdx::B5>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::B6)))
        {
            Pin<MCP23S17PinIdx::B6>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::B7)))
        {
            Pin<MCP23S17PinIdx::B7>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::A0)))
        {
            Pin<MCP23S17PinIdx::A0>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::A1)))
        {
            Pin<MCP23S17PinIdx::A1>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::A2)))
        {
            Pin<MCP23S17PinIdx::A2>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::A3)))
        {
            Pin<MCP23S17PinIdx::A3>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::A4)))
        {
            Pin<MCP23S17PinIdx::A4>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::A5)))
        {
            Pin<MCP23S17PinIdx::A5>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::A6)))
        {
            Pin<MCP23S17PinIdx::A6>::notify();
        }
        
        if (INTF & _BV(static_cast<uint16_t>(MCP23S17PinIdx::A7)))
        {
            Pin<MCP23S17PinIdx::A7>::notify();
        }
    }

    /**
    @brief Re-Arm the interrupt
    */
    static void reArmInterrupt()
    {
        // Reading the INTCAP register re-arms the interrupt
        readRegisterPair(INTCAPA);
    }
    
    /**
    @brief Read I/O port A+B
    */
    static uint16_t read()
    {
        return readRegisterPair(GPIOA);
    }
    
    /**
    @brief Read I/O port A
    */
    static uint8_t readA()
    {
        return readRegister(GPIOA);
    }
    
    /**
    @brief Read I/O port B
    */
    static uint8_t readB()
    {
        return readRegister(GPIOB);
    }
       
    private:
    
    static constexpr void checkConfig()
    {
        ///@todo Doppelte Benutzung von pins checken
    }

    // Register definitions for BANK MODE == 0
    enum
    {
        IODIRA = 0x00,
        IODIRB = 0x01,
        IPOLA = 0x02,
        IPOLB = 0x03,
        GPINTENA = 0x04,
        GPINTENB = 0x05,
        DEFVALA = 0x06,
        DEFVALB = 0x07,
        INTCONA = 0x08,
        INTCONB = 0x09,
        IOCON = 0x0A,
        //IOCON = 0x0B,
        GPPUA = 0x0C,
        GPPUB = 0x0D,
        INTFA = 0x0E,
        INTFB = 0x0F,
        INTCAPA = 0x10,
        INTCAPB = 0x11,
        GPIOA = 0x12,
        GPIOB = 0x13,
        OLATA = 0x14,
        OLATB = 0x15
    };

    // Op codes
    enum
    {
        OPCODE_WRITE = 0b01000000,
        OPCODE_READ = 0b01000001
    };

    // IOCON register bits
    enum
    {
        INTPOL = 1,
        ODR = 2,
        HAEN = 3,
        DISSLW = 4,
        SEQOP = 5,
        MIRROR = 6,
        BANK = 7
    };
    
    // Write value to register
    static void writeRegister(const uint8_t t_registerAddress, const uint8_t value)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Transfer opcode
        SPIMaster::put(OPCODE_WRITE);
        
        // Transfer register address
        SPIMaster::put(t_registerAddress);
        
        // Transfer register value
        SPIMaster::put(value);
        
        // Disable device (active low)
        SSPin::high();
    }

    // Read value from register
    static uint8_t readRegister(const uint8_t t_registerAddress)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Transfer opcode
        SPIMaster::put(OPCODE_READ);
        
        // Transfer register address
        SPIMaster::put(t_registerAddress);
        
        // Transfer register value
        const uint8_t value = SPIMaster::get();
        
        // Disable device (active low)
        SSPin::high();

        return value;
    }

    // Write value to register pair
    static void writeRegisterPair(const uint8_t t_registerAddress, const uint16_t value)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Transfer opcode
        SPIMaster::put(OPCODE_WRITE);
        
        // Transfer register address
        SPIMaster::put(t_registerAddress);
        
        // Transfer register value A (MSB)
        SPIMaster::put(value >> 8);
        
        // Transfer register value B (LSB)
        SPIMaster::put(value);
        
        // Disable device (active low)
        SSPin::high();
    }

    // Read value from register pair
    static uint16_t readRegisterPair(const uint8_t t_registerAddress)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Transfer opcode
        SPIMaster::put(OPCODE_READ);
        
        // Transfer register address
        SPIMaster::put(t_registerAddress);
        
        // Transfer register value A (MSB)
        const uint16_t valueA = SPIMaster::get();

        // Transfer register value B (LSB)
        const uint16_t valueB = SPIMaster::get();
        
        // Disable device (active low)
        SSPin::high();

        return (valueA << 8) + valueB;
    }
};

#endif