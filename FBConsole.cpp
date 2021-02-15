// Copyright 2021 Dominic Houghton. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// Framebuffer console driver, using the I_Framebuffer interface

#include "FBConsole.hpp"

template <class T>
FBConsole<T>::FBConsole(I_Framebuffer<T>* framebuffer, uint8_t* font, uint8_t scale)
{
    // These will hold the display's actual dimensions while initialising
    uint16_t display_width, display_height;

    // Set the constants within the class
    _FRAMEBUFFER = framebuffer;
    _FONT = font;
    _SCALE = scale;

    // Calculate the console width and height, store them within the class
    _FRAMEBUFFER->get_dimensions(&display_width, &display_height);
    _WIDTH = display_width / (8 * _SCALE);
    _HEIGHT = display_height / (8 * _SCALE);

    /* Create a buffer of pixels, large enough to hold a single character.
    * The put_char function will use this array, so as to maintain a consistent
    * memory footprint.
    * 
    * Scaling the font to be larger will increase the memory footprint
    * exponentially.
    */
    _CHARBUF = new T[(8 * _SCALE) * (8 * _SCALE)];

    // Set sane defaults for the runtime variables
    console_background = _FRAMEBUFFER->get_color(0x00,0x00,0x00);   // Black
    console_foreground = _FRAMEBUFFER->get_color(0xFF,0xFF,0xFF);   // White
    console_x = 0;
    console_y = 0;
}

template <class T>
void FBConsole<T>::put_char(char c)
{
    // Determine the character to draw
    uint16_t charindex = 0;
    bool drawchar = true;

    if (c == '\n') {
        drawchar = false;
        console_x = 0;
        console_y++;
    }
    else if (c == '\t') {
        int count = _TABSTOP - ((console_x) % _TABSTOP);
        
        for (int i = 0; i < count; i++)
            put_char(' ');
        drawchar = false;
    }
    else if (c >= 0x20 || c <= 0x7E) {
        charindex = (c - 0x20);
    }
    else {
        charindex = 95;
    }
    
    // Fill the character buffer

    // Iterate through the character data
    if (drawchar) {
        T* color;
        for (int cy = 0; cy < 8; cy++)
        {
            for (int cx = 0; cx < 8; cx++)
            {
                // Test the bit
                if ( ((_FONT[(charindex * 8) + cy] << cx) & 0x80) == 0x80 )
                    color = &console_foreground;
                else 
                    color = &console_background;

                // Plot the color in the character buffer
                for (int by = 0; by < _SCALE; by++)
                {
                    for (int bx = 0; bx < _SCALE; bx++)
                    {
                        //_CHARBUF[((cy + by) * 8 * _SCALE) + (cx * _SCALE) + bx] = *color;
                        //_CHARBUF[(cy * 8 * _SCALE) + (cx * _SCALE) + bx] = *color;
                        _CHARBUF[ (((cy * _SCALE) + by) * (8 * _SCALE)) + (cx * _SCALE) + bx] = *color;
                    }
                }
            }
        }

        // Plot the character buffer
        uint16_t dx, dy;
        dx = (console_x * 8 * _SCALE);
        dy = (console_y * 8 * _SCALE);
        _FRAMEBUFFER->plot_block(dx, dy,
                        dx + (8 * _SCALE) - 1, dy + (8 * _SCALE) - 1,
                        _CHARBUF, (8 * _SCALE) * (8 * _SCALE));

        // Increase console_x
        console_x++;
    }

    // Test console_x, increment console_y if necessary
    if (console_x >= _WIDTH)
    {
        console_x = 0;
        console_y++;
    }

    // Test console_y, call scroll_vertical if necessary
    if (console_y >= _HEIGHT)
    {
        _FRAMEBUFFER->scroll_vertical(8 * _SCALE);

        // If scroll_vertical was called, decrement console_y and clear the row
        console_y--;

        // Set character with only background
        for (int cy = 0; cy < 8; cy++)
        {
            for (int cx = 0; cx < 8; cx++)
            {
                // Plot the color in the character buffer
                for (int by = 0; by < _SCALE; by++)
                {
                    for (int bx = 0; bx < _SCALE; bx++)
                    {
                        _CHARBUF[ (((cy * _SCALE) + by) * (8 * _SCALE)) + (cx * _SCALE) + bx] = console_background;
                    }
                }
            }
        }

        // Clear the row with the background color
        int dy = (console_y * 8 * _SCALE);
        int dx;
        for (int x = 0; x < _WIDTH; x++)
        {
            dx = (x * 8 * _SCALE);
        
            _FRAMEBUFFER->plot_block(dx, dy,
                        dx + (8 * _SCALE) - 1, dy + (8 * _SCALE) - 1,
                        _CHARBUF, (8 * _SCALE) * (8 * _SCALE));
        }
    }

    
}

template <class T>
void FBConsole<T>::put_string(const char* str)
{
    int i = 0;
    for (i = 0; str[i] != 0; i++)
        put_char(str[i]);
}

template class FBConsole<uint8_t>;
template class FBConsole<uint16_t>;
template class FBConsole<uint32_t>;