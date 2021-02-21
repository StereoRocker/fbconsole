// Copyright 2021 Dominic Houghton. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

/* VGA display driver for Raspberry Pico, using pico_scanvideo_dpi, conforming
 * to I_Framebuffer interface
 * 
 * Some functions derived from the mandelbrot example by Raspberry Pi foundation
 * which is also governed by a BSD-style license. Mandelbrot example:
 * https://github.com/raspberrypi/pico-playground/tree/master/scanvideo/mandelbrot
 * 
 * Please note the following caveats to using the VGAFB interface:
 * -> core1 is required to process VGA scanlines without impacting the program.
 *    A consuming program must therefore not use core1.
 * -> A framebuffer is created in memory, currently using 16 bit words for each
 *    pixel. 320x240 resolution consumes 150K memory. 640x480 is not possible.
 * -> width and height are derived from the VGA mode passed to the constructor.
 * -> If waitForSetupComplete is set to false, the constructor will return
 *    quicker than otherwise - however, the pico_scanvideo_dpi library outputs
 *    to stdio during initialisation - these messages will interrupt your own
 *    stdio output, and be visible on screen if using FBConsole.
 */

#ifndef VGAFB_H
#define VGAFB_H

#include <stdint.h>
#include "I_Framebuffer.hpp"
#include "pico/scanvideo.h"

class VGAFB : public I_Framebuffer<uint16_t> {
    public:
        // Constructor
        VGAFB(const scanvideo_mode_t* vga_mode,
              bool waitForSetupComplete = true);

        // I_Framebuffer required methods
        uint16_t get_color(uint8_t r, uint8_t g, uint8_t b);
        void get_dimensions(uint16_t* width, uint16_t* height);


        void plot_block(uint16_t x0, uint16_t y0,
                        uint16_t x1, uint16_t y1,
                        uint16_t* pixeldata, uint32_t len);

        void scroll_vertical(uint16_t pixels);

        // Called by core1_func in vgafb.cpp, avoids exposing class variables
        void core1_inner_func();
    private:
        uint16_t _WIDTH;
        uint16_t _HEIGHT;
        const scanvideo_mode_t* _vga_mode;
        uint16_t* _framebuffer;
};

#endif