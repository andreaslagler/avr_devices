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

#ifndef _25LC512_H
#define _25LC512_H

#define _25LC512_MAX_SPI_CLOCK 20000000UL // 20 MHz

/**
@brief Driver for SPI EEPROM 25AA512/25LC512
@param SPIMaster Driver class for SPI master implementing a static put() method
@param SSPin Driver class for SS (output) pin
*/
template <typename SPIMaster, typename SSPin>
class _25LC512
{
    public:
    
    /// @brief Data type for memory address/offset
    typedef uint16_t Address;

    /**
    @brief Get 25LC512 EEPROM capacity in bytes
    @result EEPROM capacity in bytes
    */
    static constexpr uint32_t capacity()
    {
        return 65536ULL;
    }

    /**
    @brief Initialize EEPROM device for write operations
    */
    static void init()
    {
        // Enable device (active low)
        SSPin::low();
        
        // Send write enable
        SPIMaster::put(INSTRUCTION_WREN);
        
        // Disable device (active low)
        SSPin::high();
    }

    /**
    @brief Write one byte to EEPROM at given position
    @param address Position in EEPROM (0..65535)
    @param data Byte to be written to EEPROM
    */
    static void write(const Address address, const uint8_t data)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Set Instruction
        SPIMaster::put(INSTRUCTION_WRITE);
        
        // Set Address MSB
        SPIMaster::put(address);
        
        // Set Address LSB
        SPIMaster::put(address >> 8);
        
        // Store data
        SPIMaster::put(data);
        
        // Disable device (active low)
        SSPin::high();
    }

    /**
    @brief Write multiple Bytes to EEPROM starting at given position
    @param address Position of first byte in EEPROM (0..65535)
    @param data Bytes to be written to EEPROM
    @param nofBytes Number of Bytes to be written to EEPROM (1..65535)
    */
    template <typename Length>
    static void write(const Address address, const uint8_t * data, const Length nofBytes)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Set Instruction
        SPIMaster::put(INSTRUCTION_WRITE);
        
        // Set Address MSB
        SPIMaster::put(address);
        
        // Set Address LSB
        SPIMaster::put(address >> 8);
        
        // Store data
        SPIMaster::put(data, nofBytes);
        
        // Disable device (active low)
        SSPin::high();
    }

    /**
    @brief Write one Byte to EEPROM starting at given position
    @param pos Position of first byte in EEPROM (0..65535)
    @param data Bytes to be written to EEPROM
    @param nofBytes Number of Bytes to be filled on EEPROM (1..65535)
    */
    template <typename Length>
    static void fill(const Address address, const uint8_t data, const Length nofBytes)
    {
        // Enable device (active low)
        SSPin::low();

        // Set Instruction
        SPIMaster::put(INSTRUCTION_WRITE);
        
        // Set Address MSB
        SPIMaster::put(address);
        
        // Set Address LSB
        SPIMaster::put(address >> 8);
        
        // Store data
        for (; nofBytes > 0; --nofBytes)
        {
            SPIMaster::put(data);
        }
        
        // Disable device (active low)
        SSPin::high();
    }

    /**
    @brief Read one byte from EEPROM from given position
    @param pos Position in EEPROM (0..65535)
    @result data Byte read from EEPROM
    */
    static uint8_t read(const Address address)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Set Instruction
        SPIMaster::put(INSTRUCTION_READ);
        
        // Set Address MSB
        SPIMaster::put(address);
        
        // Set Address LSB
        SPIMaster::put(address >> 8);
        
        // Load data
        const uint8_t data = SPIMaster::get();
        
        // Disable device (active low)
        SSPin::high();

        return data;
    }

    /**
    @brief Read multiple Bytes from EEPROM starting at given position
    @param pos Position of first byte in EEPROM (0..65535)
    @param data Bytes to be read from EEPROM
    @param nofBytes Number of Bytes to be read from EEPROM (1..65535)
    */
    template <typename Length>
    static void read(const Address address, uint8_t * data, const Length nofBytes)
    {
        // Enable device (active low)
        SSPin::low();
        
        // Set Instruction
        SPIMaster::put(INSTRUCTION_READ);
        
        // Set Address MSB
        SPIMaster::put(address);
        
        // Set Address LSB
        SPIMaster::put(address >> 8);
        
        // Load data
        SPIMaster::get(data, nofBytes);
        
        // Disable device (active low)
        SSPin::high();
    }
    
    private:
    
    enum
    {
        INSTRUCTION_READ = 0b11,
        INSTRUCTION_WRITE = 0b10,
        INSTRUCTION_WREN = 0b110,
        INSTRUCTION_WRDI = 0b100,
        INSTRUCTION_RDSR = 0b101,
        INSTRUCTION_WRSR = 0b1,
        INSTRUCTION_PE = 0b01000010,
        INSTRUCTION_SE = 0b11011000,
        INSTRUCTION_CE = 0b11000111,
        INSTRUCTION_RDID = 0b10101011,
        INSTRUCTION_DPD = 0b10111001
    };
};

#endif