/*
 * Copyright (c) 2012 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __TARGET_GPIOCONFIG_H
#define __TARGET_GPIOCONFIG_H

#include <platform/gpio.h>

#define GPIO_LED0 GPIO(GPIO_PORT_A, 5)

#define GPIO_DATA_VALID GPIO(GPIO_PORT_A, 0)

#define GPIO_BUTTON_B GPIO(GPIO_PORT_B, 2)
#define GPIO_BUTTON_Y GPIO(GPIO_PORT_B, 3)
#define GPIO_BUTTON_SELECT GPIO(GPIO_PORT_B, 4)
#define GPIO_BUTTON_START GPIO(GPIO_PORT_B, 5)
#define GPIO_BUTTON_UP GPIO(GPIO_PORT_B, 6)
#define GPIO_BUTTON_DOWN GPIO(GPIO_PORT_B, 7)
#define GPIO_BUTTON_LEFT GPIO(GPIO_PORT_B, 8)
#define GPIO_BUTTON_RIGHT GPIO(GPIO_PORT_B, 9)
#define GPIO_BUTTON_A GPIO(GPIO_PORT_A, 4)
#define GPIO_BUTTON_X GPIO(GPIO_PORT_A, 5)
#define GPIO_BUTTON_L GPIO(GPIO_PORT_A, 8)
#define GPIO_BUTTON_R GPIO(GPIO_PORT_A, 9)
#endif
