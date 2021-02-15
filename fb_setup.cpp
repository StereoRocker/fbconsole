// Copyright 2021 Dominic Houghton. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// fb_setup.cpp: Sets up an I_Framebuffer, attaches an FBConsole instance, and
//               implements a Pico SDK stdio driver to interface with FBConsole.

#include "FBConsole.hpp"
#include "pico/stdio/driver.h"
#include "pico/stdio.h"

#include "ili9341.hpp"
#include "gamefont.hpp"

FBConsole<uint16_t> *fb;

// FBConsole specific
void fb_out_chars(const char *buf, int len)
{
    for (int i = 0; i < len; i++)
        fb->put_char(buf[i]);
}

stdio_driver_t stdio_fb = {
    .out_chars = fb_out_chars,
    .out_flush = 0,
    .in_chars = 0,
    .next = 0,

#if PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = false
#endif

};


// ILI9341 pin definitions:
// We are going to use SPI 0, and allocate it to the following GPIO pins.
// Pins can be changed, see the GPIO function select table in the datasheet
// for information on GPIO assignments.
#define SPI_PORT spi0
#define PIN_MISO 4
#define PIN_SCK  6
#define PIN_MOSI 7
#define PIN_CS   27
#define PIN_DC   26
#define PIN_RST  22

ILI9341* display;

void fb_setup()
{
    display = new ILI9341(SPI_PORT, PIN_MISO, PIN_MOSI, PIN_SCK,
                            PIN_CS, PIN_DC, PIN_RST);

    fb = new FBConsole<uint16_t>(display, (uint8_t*)&font);

    stdio_set_driver_enabled(&stdio_fb, true);
}