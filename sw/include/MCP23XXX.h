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

#ifndef MCP23xxx_PINS_H
#define MCP23xxx_PINS_H

#include <stdint.h>
#include <stdbool.h>
#include <functional.h>

///@brief Pin type of MCP23xxx pins
enum class MCP23xxxPinType
{
    UNUSED, // Unused pin
    OUTPUT, // Generic output pin
    INPUT, // Generic input pin
    INPUT_PU, // Generic input pin with pull-up enabled
    SWITCH, // Push-button switch
    ROTENC_PHASE_A, // Rotary encoder phase A generating the interrupt
    ROTENC_PHASE_B, // Rotary encoder phase B indicating the direction of rotation
};

/**
@brief Driver for MCP23xxx family port expander
MCP23xxx GP I/O pins can be used in a flexible way.
Logical pin drivers are provided for the following pin types:
- Generic input pin
- Generic output pin
- Push-button: A registered callback method will be executed on button push
- Rotary encoder phase A: A registered callback method will be executed on a rising edge of phase A. Direction of rotation can be determined by reading  a corresponding phase B pin
- Rotary encoder phase B: This pin retains its logical state on a an interrupt triggered by rotary encoder phase A. Depending on the actual encoder, logical state low and high correspond to clockwise or counter-clockwise rotation of the rotary encoder
*/
class MCP23xxx
{
    protected:

    /**
    @brief Pin configuration class (Default)
    @tparam PinOnDevice Register-level driver class for underlying physical pin on actual MCP23xxx device implementing static methods readINTCAP(), readGPIO() and writeOLAT(bool)
    @tparam t_PinType Pin type
    */
    template <typename PinOnDevice, MCP23xxxPinType t_PinType>
    class Pin
    {
        public:
        
        static constexpr bool s_IODIRBit = false;
        static constexpr bool s_IPOLBit = false;
        static constexpr bool s_GPINTENBit = false;
        static constexpr bool s_DEFVALBit = false;
        static constexpr bool s_INTCONBit = false;
        static constexpr bool s_GPPUBit = false;

        static void notify() __attribute__((always_inline))
        {} // Dummy
    };

    /**
    @brief Pin configuration class (Generic output pin)
    @tparam PinOnDevice Register-level driver class for underlying physical pin on actual MCP23xxx device implementing static methods readINTCAP(), readGPIO() and writeOLAT(bool)
    */
    template <typename PinOnDevice>
    class Pin<PinOnDevice, MCP23xxxPinType::OUTPUT> : PinOnDevice
    {
        public:

        static constexpr bool s_IODIRBit = false;
        static constexpr bool s_IPOLBit = false;
        static constexpr bool s_GPINTENBit = false;
        static constexpr bool s_DEFVALBit = false;
        static constexpr bool s_INTCONBit = false;
        static constexpr bool s_GPPUBit = false;

        static void notify() __attribute__((always_inline))
        {} // Dummy
    };

    /**
    @brief Pin configuration class (Generic input pin)
    @tparam PinOnDevice Register-level driver class for underlying physical pin on actual MCP23xxx device implementing static methods readINTCAP(), readGPIO() and writeOLAT(bool)
    */
    template <typename PinOnDevice>
    class Pin<PinOnDevice, MCP23xxxPinType::INPUT> : PinOnDevice
    {
        public:
        
        static constexpr bool s_IODIRBit = true;
        static constexpr bool s_IPOLBit = false;
        static constexpr bool s_GPINTENBit = false;
        static constexpr bool s_DEFVALBit = false;
        static constexpr bool s_INTCONBit = false;
        static constexpr bool s_GPPUBit = false;

        static void notify() __attribute__((always_inline))
        {} // Dummy
    };

    /**
    @brief Pin configuration class (Generic input pin with pull-up enabled)
    @tparam PinOnDevice Register-level driver class for underlying physical pin on actual MCP23xxx device implementing static methods readINTCAP(), readGPIO() and writeOLAT(bool)
    */
    template <typename PinOnDevice>
    class Pin<PinOnDevice, MCP23xxxPinType::INPUT_PU> : PinOnDevice
    {
        public:
        
        static constexpr bool s_IODIRBit = true;
        static constexpr bool s_IPOLBit = false;
        static constexpr bool s_GPINTENBit = false;
        static constexpr bool s_DEFVALBit = false;
        static constexpr bool s_INTCONBit = false;
        static constexpr bool s_GPPUBit = true;

        static void notify() __attribute__((always_inline))
        {} // Dummy
    };

    /**
    @brief Pin configuration class (Push-button switch)
    @tparam PinOnDevice Register-level driver class for underlying physical pin on actual MCP23xxx device implementing static methods readINTCAP(), readGPIO() and writeOLAT(bool)
    */
    template <typename PinOnDevice>
    class Pin<PinOnDevice, MCP23xxxPinType::SWITCH> : PinOnDevice
    {
        public:
        
        /**
        Register a callback for transition of rotary encoder phase A
        @tparam Callback Type of callback to be registered
        @param callback Callback to be registered
        */
        template <typename Callback>
        static void registerCallback(Callback&& callback)
        {
            s_callback = callback;
        }
        
        static constexpr bool s_IODIRBit = true;
        static constexpr bool s_IPOLBit = true; // Switch is connected to ground
        static constexpr bool s_GPINTENBit = true;
        static constexpr bool s_DEFVALBit = false;
        static constexpr bool s_INTCONBit = false;
        static constexpr bool s_GPPUBit = true;

        static function<void()> s_callback;
        
        static void notify() __attribute__((always_inline))
        {
            if (PinOnDevice::readINTCAP())
            {
                s_callback();
            }
        }
    };

    /**
    @brief Pin configuration class (Rotary encoder phase A generating the interrupt)
    @tparam PinOnDevice Register-level driver class for underlying physical pin on actual MCP23xxx device implementing static methods readINTCAP(), readGPIO() and writeOLAT(bool)
    @note Both rotary encoder phases A and B must be connected to the same MCP23xxx device
    */
    template <typename PinOnDevice>
    class Pin<PinOnDevice, MCP23xxxPinType::ROTENC_PHASE_A> : PinOnDevice
    {
        public:
        
        /**
        Register a callback for transition of rotary encoder phase A
        @tparam Callback Type of callback to be registered
        @param callback Callback to be registered
        */
        template <typename Callback>
        static void registerCallback(Callback&& callback)
        {
            s_callback = callback;
        }
        
        static constexpr bool s_IODIRBit = true;
        static constexpr bool s_IPOLBit = true; // Encoder phase is connected to ground
        static constexpr bool s_GPINTENBit = true;
        static constexpr bool s_DEFVALBit = false;
        static constexpr bool s_INTCONBit = false;
        static constexpr bool s_GPPUBit = true;

        static function<void()> s_callback;
        
        static void notify() __attribute__((always_inline))
        {
            // Notify observer on rising edge of phase A
            if (PinOnDevice::readINTCAP())
            {
                s_callback();
            }
        }
    };

    /**
    @brief Pin configuration class (Rotary encoder phase B indicating the direction)
    @tparam PinOnDevice Register-level driver class for underlying physical pin on actual MCP23xxx device implementing static methods readINTCAP(), readGPIO() and writeOLAT(bool)
    @note Both rotary encoder phases A and B must be connected to the same MCP23xxx device
    */
    template <typename PinOnDevice>
    class Pin<PinOnDevice, MCP23xxxPinType::ROTENC_PHASE_B> : PinOnDevice
    {
        public:
        
        /**
        @brief Read the B phase of a rotary encoder
        @result Logical state of the rotary encoder B phase at the time the A phase interrupt occurred
        @note Instead of the actual logical state in the moment of register access, the logical state captured by the last interrupt will be returned
        */
        static bool read() __attribute__((always_inline))
        {
            return PinOnDevice::readINTCAP();
        }
        
        static constexpr bool s_IODIRBit = true;
        static constexpr bool s_IPOLBit = true; // Encoder phase is connected to ground
        static constexpr bool s_GPINTENBit = false;
        static constexpr bool s_DEFVALBit = false;
        static constexpr bool s_INTCONBit = false;
        static constexpr bool s_GPPUBit = true;

        static void notify() __attribute__((always_inline))
        {} // Dummy
    };
};

// Static initialization
template <typename PinOnDevice>
function<void()> MCP23xxx::Pin<PinOnDevice, MCP23xxxPinType::SWITCH>::s_callback;

// Static initialization
template <typename PinOnDevice>
function<void()> MCP23xxx::Pin<PinOnDevice, MCP23xxxPinType::ROTENC_PHASE_A>::s_callback;

#endif