// Copyright 2021 Dominic Houghton. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// Framebuffer console driver, using the I_Framebuffer interface

#ifndef FBCONSOLE_H
#define FBCONSOLE_H

#include "I_Framebuffer.hpp"

template <class T>
class FBConsole {
    public:
        FBConsole(I_Framebuffer<T>* framebuffer, uint8_t* font, uint8_t scale = 1);

        void put_char(char c);
        void put_string(const char* str);
        void clear();

        void set_location(uint16_t x, uint16_t y);
        void set_background(T);
        void set_foreground(T);

        void get_dimensions(uint16_t* width, uint16_t* height);
        
    private:
        I_Framebuffer<T>* _FRAMEBUFFER;
        uint8_t* _FONT;
        uint16_t _WIDTH;
        uint16_t _HEIGHT;
        uint8_t _SCALE;
        T console_background;
        T console_foreground;
        uint16_t console_x;
        uint16_t console_y;

        T* _CHARBUF;

        const uint16_t _TABSTOP = 8;
};

#endif