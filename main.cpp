// Copyright 2021 Dominic Houghton. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// Test application for FBConsole using I_Framebuffer interface

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"

#include "fb_setup.hpp"

int main()
{
    // Set all pins high, to avoid chip selecting an incorrect device
    for (int pin = 0; pin < 29; pin++)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 1);
    }
    
    // Initialise all SDK-provided stdio drivers
    stdio_init_all();

    

    // Set up a display, FBConsole, and an stdio driver
    fb_setup();    

    // Test printf
    printf("Hello world!\n\n%s\nint: %i\thex: %X\n\nThe framebuffer console driver supports wrapping. Terminal emulation to come.\n\n", "The meaning of life:", 42, 42);

    printf("Nope");
    __breakpoint();
    printf("\b\b\b\b    ");

    // Break, and halt execution
    __breakpoint();
    for(;;);
    return 0;
}
