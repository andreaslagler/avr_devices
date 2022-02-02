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

#ifndef HD44780_H
#define HD44780_H

#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "memcopy.h"
#include "div.h"

// LCD routines need correct CPU clock for proper timing
#ifndef F_CPU
#warning "LCD.h : F_CPU not defined, using default F_CPU = 16MHz"
#define F_CPU 16000000UL
#endif

///@brief Number of characters controlled by HD44780 device
enum class HD44780_NofCharacters : uint8_t
{
    _1x16,
    _2x16
};

/**
@brief HD44780 Display Configuration
This class provides all data regarding the size of the display
@tparam t_nofCharacter Number of characters on display
*/
template <HD44780_NofCharacters t_nofCharacter>
struct HD44780_Configuration;

/**
@brief HD44780 Configuration for 2x16 display
*/
template <>
class HD44780_Configuration<HD44780_NofCharacters::_2x16>
{
    protected:
    
    /**
    @brief Get number of display rows
    @result Number of display rows
    */
    static constexpr uint8_t getNofRows()
    {
        return 2;
    }

    /**
    @brief Get number of display columns
    @result Number of display columns
    */
    static constexpr uint8_t getNofColumns()
    {
        return 16;
    }

    /**
    @brief Get address offset for given row
    @param row Selected row (0..1)
    @result Address offset for selected row
    */
    static uint8_t getRowAddress(const uint8_t row)
    {
        static const uint8_t rowAddress[2] PROGMEM = {0x00, 0x40};
        return memread_P(&rowAddress[row]);
    }
    
    /**
    @brief Get control word for number of rows
    @result Control word for number of rows
    */
    static constexpr uint8_t getNofRowsControlWord()
    {
        return 0x08; // 2 rows
    }
};

/**
@brief Low-level driver for HD44780 LCD controller in 4 bit mode connected to a GPIO port.
This class contains all methods to configure and use the port.
@tparam PB_Port Driver for any 4-pin GPIO_SubPort connected to data port pins D4:7 of the display
@tparam EN_Pin Driver for any GPIO_Pin connected to EN pin of the display
@tparam RS_Pin Driver for any GPIO_Pin connected to RS pin of the display
*/
template <typename PB_Port, typename EN_Pin, typename RS_Pin>
class HD44780_ParallelPort
{
    protected:
    
    /**
    @brief Initialization
    */
    static void init()
    {
        // Use control pins as outputs
        PB_Port::set_as_output();
        EN_Pin::set_as_output();
        RS_Pin::set_as_output();
        
        // clear all outputs
        PB_Port::write(0);
        EN_Pin::low();
        RS_Pin::low();
        
        // Init the actual LCD controller
        
        // Wait 15 ms until LCD is ready
        _delay_ms(15);
        
        // Send soft reset three times
        write4Bit(0x30);
        _delay_ms(5);
        writeEnable();
        _delay_ms(1);
        writeEnable();
        _delay_ms(1);
        
        // Enable 4-bit mode
        write4Bit(0x20);
        _delay_ms(5);
    }

    /**
    @brief Send a data byte to the LCD
    @param data Data byte to be sent to LCD
    */
    static void writeData(const uint8_t data)
    {
        // RS = 1 for data
        write8Bit(data, true);
        
        // 46us delay
        _delay_us(46);
    }

    /**
    @brief Send a command byte to the LCD
    @param command Command byte to be sent to LCD
    */
    static void writeCommand(const uint8_t command)
    {
        // RS = 0 for command
        write8Bit(command, false);
        
        // 42us delay
        _delay_us(42);
    }

    private:

    /**
    @brief Create an Enable pulse
    */
    static void writeEnable()
    {
        EN_Pin::high();
        _delay_us(20); // Enable pulse is 20 us
        EN_Pin::low();
    }
    
    /**
    @brief Send 4bit word to LCD
    @param data Byte to be sent to LCD
    @note Upper nibble of data byte will be sent to LCD
    */
    static void write4Bit(const uint8_t data)
    {
        // Set data bits
        PB_Port::write(data >> 4);
        
        // Send enable
        writeEnable();
    }

    /**
    @brief Send byte to LCD
    @param data Byte to be sent to LCD
    @param RS flag indicating if RS pin should be driven high
    */
    static void write8Bit(const uint8_t data, const bool RS)
    {
        // Set RS
        RS_Pin::write(RS);
        
        // Send upper nibble first, then lower nibble
        write4Bit(data);
        write4Bit(data << 4);
    }
};

/**
@brief Low-level driver for configuration of an HD44780 LCD controller in 4 bit mode connected to a 74HC595 8bit serial --> parallel shift register
This class contains all methods to configure and use the port.
@tparam SPIMaster Any SPI master driver implementing a static put() method
@tparam SS_Pin Any output pin driver implementing stating low() and high() methods
*/
template <typename SPIMaster, typename SS_Pin>
class HD44780_Configuration_74HC595
{
    protected:

    /**
    @brief Initialization
    @note SPIMaster and SS_Pin drivers have to be initialized beforehand
    */
    static void init()
    {
        // Wait 15 ms until LCD is ready
        _delay_ms(15);
        
        // Send soft reset three times
        write4Bit(0x30);
        _delay_ms(5);
        writeEnable();
        _delay_ms(1);
        writeEnable();
        _delay_ms(1);
        
        // Enable 4-bit mode
        write4Bit(0x20);
        _delay_ms(5);
    }

    /**
    @brief Send a data byte to LCD
    @param data Data byte to be sent to LCD
    */
    static void writeData(const uint8_t data)
    {
        // RS = 1 for data
        write8Bit(data, true);
        
        // 46us delay
        _delay_us(46);
    }

    /**
    @brief Send a command byte to LCD
    @param data Command byte to be sent to LCD
    */
    static void writeCommand(const uint8_t data)
    {
        // RS = 0 for command
        write8Bit(data, false);
        
        // 42us delay
        _delay_us(42);
    }

    private:

    /**
    @brief Send an Enable pulse to LCD
    */
    static void writeEnable()
    {
        // Active low
        SS_Pin::low();
        
        _delay_us(20); // Enable pulse is 20 us
        
        SS_Pin::high();
    }
    
    /**
    @brief Send byte to LCD
    @param data Byte to be sent to LCD
    @param RS flag indicating if RS pin should be driven high
    */
    static void write4Bit(const uint8_t data, const bool RS = false)
    {
        // Send data bits and RS bit
        const union
        {
            uint8_t byte;

            struct
            {
                uint8_t dummy:2;
                bool RS:1;
                bool backlight:1; // TODO Entfernen, erfordert HW Änderung wegen RS Pin!
                uint8_t DB:4;
            } data;
        }
        buffer =
        {
            .data =
            {
                .dummy = 0,
                .RS = RS,
                .backlight = false,
                .DB = data
            }
        };
        
        SPIMaster::put(buffer.byte);
        
        // Send enable. This also clocks the serial data to the parallel output of the 74HC595 shift register
        writeEnable();
    }

    /**
    @brief Send byte to LCD
    @param data Byte to be sent to LCD
    @param RS flag indicating if RS pin should be driven high
    @param backlight Optional flag indicating if LCD backlight should be switched on (if connected)
    */
    static void write8Bit(const uint8_t data, const bool RS)
    {
        // Send upper nibble first, then lower nibble. Forward RS bit
        write4Bit(data >> 4, RS);
        write4Bit(data, RS);
    }
};

/**
@brief High-level driver for HD44780 LCD controller in 4 bit mode
@tparam t_nofCharacter Number of characters on display
@tparam Port Physical port where the LCD is connected to (e.g Parallel GP I/O or 74HC595 shift register)
*/
template <HD44780_NofCharacters t_nofCharacters, typename Port>
class HD44780 : Port, HD44780_Configuration<t_nofCharacters>
{
    public:

    using HD44780_Configuration<t_nofCharacters>::getNofRows;
    using HD44780_Configuration<t_nofCharacters>::getNofColumns;

    /**
    @brief Initialization
    */
    static void init()
    {
        // Init physical port
        Port::init();
        
        // 4 bit / 5x7 pixel / given number of rows
        constexpr uint8_t ui2Line = HD44780_Configuration<t_nofCharacters>::getNofRowsControlWord();
        writeCommand(0x20 | ui2Line);

        // Display on / Cursor off / Blink off
        writeCommand(0x08 | 0x04);
        
        // Cursor increment / no scrolling
        writeCommand(0x04 | 0x02);

        clear();
        home();
    }

    /**
    @brief Clear the LCD
    */
    static void clear()
    {
        // clear command is 0x01, delay is 2 ms
        writeCommand(0x01);
        _delay_ms(2);
    }
    
    /**
    @brief Set cursor to home
    */
    static void home()
    {
        // home command is 0x02, delay is 2 ms
        writeCommand(0x02);
        _delay_ms(2);
    }

    /**
    @brief Set cursor to given row / column position
    @param rowIdx Row index (0..t_nofRows-1)
    @param columnIdx Column index (0..t_nofColumns-1)
    */
    static void setCursor(const uint8_t rowIdx, const uint8_t columnIdx)
    {
        writeCommand(0x80 + getRowAddress(rowIdx) + columnIdx);
    }

    /**
    @brief Put single character to LCD
    @param data Character to be sent to LCD
    */
    static void putc(const char data)
    {
        writeData(data);
    }

    /**
    @brief Put zero-terminated string (stored in RAM) to LCD
    @param data Zero-terminated string to be sent to LCD
    */
    static void puts(const char *data)
    {
        while (*data != '\0')
        {
            writeData(*data++);
        }
    }

    /**
    @brief Put zero-terminated string (stored in PROGMEM) to LCD
    @param data Zero-terminated string to be sent to LCD
    */
    static void putsP(const char *data)
    {
        char character = pgm_read_byte(data++);
        while (character != '\0')
        {
            writeData(character);
            character = pgm_read_byte(data++);
        }
    }

    /**
    @brief Write user character data (stored in RAM) to CG RAM
    @param code Character code
    @param data User character data
    @note data consists of 8 bytes
    */
    static void generateChar(const uint8_t code, const uint8_t * data)
    {
        // Index of character
        writeCommand(0x40 | (code<<3));
        
        // Transfer bit pattern
        for (uint8_t cnt = 0; cnt < 8; ++cnt)
        {
            writeData(*data);
            data++;
        }
    }

    /**
    @brief Write user character data (stored in PROGMEM) to CG RAM
    @param code Character code
    @param data User character data
    @note data consists of 8 bytes
    */
    static void generateChar_P(const uint8_t code, const uint8_t * data)
    {
        // Index of character
        writeCommand(0x40 | (code<<3));
        
        // Transfer bit pattern
        for (uint8_t cnt = 0; cnt < 8; ++cnt)
        {
            writeData(pgm_read_byte(data));
            data++;
        }
    }
    
    private:
    
    using Port::writeCommand;
    using Port::writeData;
    using HD44780_Configuration<t_nofCharacters>::getRowAddress;
};

#endif