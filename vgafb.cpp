// Copyright 2021 Dominic Houghton. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

/* VGA display driver for Raspberry Pico, using pico_scanvideo_dpi, conforming to I_Framebuffer interface
 * 
 * Please see vgafb.hpp for a list of considerations
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vgafb.hpp"

#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "pico/multicore.h"

static VGAFB* vgafb;
struct semaphore video_setup_complete;

static inline uint16_t *raw_scanline_prepare(struct scanvideo_scanline_buffer *dest, uint width) {
    assert(width >= 3);
    assert(width % 2 == 0);
    // +1 for the black pixel at the end, -3 because the program outputs n+3 pixels.
    dest->data[0] = COMPOSABLE_RAW_RUN | (width + 1 - 3 << 16);
    // After user pixels, 1 black pixel then discard remaining FIFO data
    dest->data[width / 2 + 2] = 0x0000u | (COMPOSABLE_EOL_ALIGN << 16);
    dest->data_used = width / 2 + 2;
    assert(dest->data_used <= dest->data_max);
    return (uint16_t *) &dest->data[1];
}

static inline void raw_scanline_finish(struct scanvideo_scanline_buffer *dest) {
    // Need to pivot the first pixel with the count so that PIO can keep up
    // with its 1 pixel per 2 clocks
    uint32_t first = dest->data[0];
    uint32_t second = dest->data[1];
    dest->data[0] = (first & 0x0000ffffu) | ((second & 0x0000ffffu) << 16);
    dest->data[1] = (second & 0xffff0000u) | ((first & 0xffff0000u) >> 16);
    dest->status = SCANLINE_OK;
}

// Stub function to launch core1
void core1_func()
{
    // Launch the actual core1 code in VGAFB class
    vgafb->core1_inner_func();
}

void VGAFB::core1_inner_func()
{
    // Initialise scanvideo library
    scanvideo_setup(_vga_mode);
    scanvideo_timing_enable(true);

    sem_release(&video_setup_complete);

    uint32_t offset = 0;

    // Process scanline work queue infinitely
    while (true)
    {
        // Get the next scanline buffer that needs filling, blocking if we're to far ahead
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);

        // Set up scanline_buffer to receive framebuffer pixels, get a reference to the start of pixel data
        uint16_t* color_buf = raw_scanline_prepare(scanline_buffer, 320);

        // Calculate the offset into the framebuffer for this scanline
        offset = (scanvideo_scanline_number(scanline_buffer->scanline_id) * _WIDTH);

        // Copy each pixel from the framebuffer to scanline_buffer
        for (int i = 0; i < 320; i++)
        {
            color_buf[i] = _framebuffer[offset + i];
        }

        // Finish setting up scanline_buffer - this swaps color_buf[1] and the pixel count
        raw_scanline_finish(scanline_buffer);

        // Pass the completed buffer back to the scanvideo code for display at the right time
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

VGAFB::VGAFB(const scanvideo_mode_t* vga_mode, bool waitForSetupComplete)
{
    // Store the passed VGA mode internally
    _vga_mode = vga_mode;

    // Also store the width and height for convenience (GCC might optimise this out?)
    _WIDTH = _vga_mode->width;
    _HEIGHT = _vga_mode->height;

    // Try to allocate the framebuffer
    _framebuffer = 0;
    _framebuffer = (uint16_t*)malloc(_WIDTH * _HEIGHT * 2);
    if (_framebuffer == 0)
    {
        // Print a debug statement to stdout, loop breakpoints forever.
        printf("PANIC: VGAFB - Failed to allocate framebuffer.");
        for (;;)
            __breakpoint();

    }

    // Clear the framebuffer
    memset(_framebuffer, 0, _WIDTH * _HEIGHT * 2);

    // Begin the VGA setup on core1
    sem_init(&video_setup_complete, 0, 1);

    vgafb = this;
    multicore_launch_core1(core1_func);

    if (waitForSetupComplete)
        sem_acquire_blocking(&video_setup_complete);
}

uint16_t VGAFB::get_color(uint8_t r, uint8_t g, uint8_t b)
{
    return PICO_SCANVIDEO_PIXEL_FROM_RGB8(r,g,b);
}

void VGAFB::get_dimensions(uint16_t* width, uint16_t* height)
{
    *width = _vga_mode->width;
    *height = _vga_mode->height;
}



void VGAFB::scroll_vertical(uint16_t pixels)
{
    // We must manipulate the framebuffer directly, as there's no hardware here.

    // Calculate the number of pixels we're scrolling, based on the width of the display
    uint32_t offset = pixels * _WIDTH;

    // Copy from the framebuffer at the offset index, back to the framebuffer at index 0
    memcpy(_framebuffer, &_framebuffer[offset], ((_WIDTH * _HEIGHT) - offset) * 2);

    // I_Framebuffer does not require us to do anything with the shifted pixels, so we're done.
}

void VGAFB::plot_block(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t* pixeldata, uint32_t len)
{
    uint32_t count = 0;

    int yd = y0 < y1 ? 1 : -1;
    int xd = x0 < x1 ? 1 : -1;

    if (yd > 0)
        y1++;
    else
        y1--;

    if (xd > 0)
        x1++;
    else
        x1--;

    for (int y = y0; y != y1; y += yd)
    {
        for (int x = x0; x != x1; x += xd)
        {
            // Plot the pixel
            _framebuffer[(y * _WIDTH) + x] = pixeldata[count++];

            // If we've read all the pixels in pixeldata, return
            if (count == len)
                return;
        }
    }
}