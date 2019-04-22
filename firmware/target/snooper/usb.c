/*
 * Copyright (c) 2013-2015 Travis Geiselbrecht
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
#include <err.h>
#include <debug.h>
#include <stdio.h>
#include <trace.h>
#include <target.h>
#include <compiler.h>
#include <dev/gpio.h>
#include <dev/usb.h>
#include <dev/usbc.h>
#include <hw/usb.h>
#include <lk/init.h>
#include <platform/gpio.h>
#include <target/gpioconfig.h>

#define LOCAL_TRACE 0

#define W(w) (w & 0xff), (w >> 8)
#define W3(w) (w & 0xff), ((w >> 8) & 0xff), ((w >> 16) & 0xff)

/* top level device descriptor */
static const uint8_t dev_descr[] = {
    0x12,      /* descriptor length */
    DEVICE,    /* Device Descriptor type */
    W(0x0200), /* USB Version */
    239,       /* class */
    2,         /* subclass */
    1,         /* protocol */
    64,        /* max packet size, ept0 */
    W(0x9999), /* vendor */
    W(0x9999), /* product */
    W(0x9999), /* release */
    0x2,       /* manufacturer string */
    0x1,       /* product string */
    0x0,       /* serialno string */
    0x1,       /* num configs */
};

/* high/low speed device qualifier */
static const uint8_t devqual_descr[] = {
    0x0a,             /* len */
    DEVICE_QUALIFIER, /* Device Qualifier type */
    W(0x0200),        /* USB version */
    0x00,             /* class */
    0x00,             /* subclass */
    0x00,             /* protocol */
    64,               /* max packet size, ept0 */
    0x01,             /* num configs */
    0x00              /* reserved */
};

static const uint8_t cfg_descr[] = {
    0x09,          /* Length of Cfg Descr */
    CONFIGURATION, /* Type of Cfg Descr */
    W(0x09),       /* Total Length (incl ifc, ept) */
    0x00,          /* # Interfaces */
    0x01,          /* Cfg Value */
    0x00,          /* Cfg String */
    0xc0,          /* Attributes -- self powered */
    250,           /* Power Consumption - 500mA */
};

static const uchar langid[] = {0x04, 0x03, 0x09, 0x04};

static const uint8_t if_descriptor_lowspeed[] = {
    0x09,      /* length */
    INTERFACE, /* type */
    0x01,      /* interface num */
    0x00,      /* alternates */
    0x01,      /* endpoint count */
    0x03,      /* interface class */
    0x00,      /* interface subclass */
    0x00,      /* interface protocol */
    0x00,      /* string index */

    0x09,       // bLength
    0x21,       // bDescriptorType (HID)
    0x10, 0x01, // bcdHID 1.10
    0x00,       // bCountryCode
    0x01,       // bNumDescriptors
    0x22,       // bDescriptorType[0] (HID)
    50, 0x00,   // wDescriptorLength[0] 50

    /* endpoint 1 IN */
    0x07,     /* length */
    ENDPOINT, /* type */
    0x81,     /* address: 1 IN */
    0x03,     /* type: bulk */
    W(64),    /* max packet size: 64 */
    8,        /* interval */
};

static const uint8_t hid_descriptor[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04,       // Usage (Joystick)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x01,       //   Report ID (1)
    0xA1, 0x02,       //   Collection (Logical)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x02,       //     Report Count (2)
    0x15, 0x00,       //     Logical Minimum (0)
    0x26, 0xFF, 0x00, //     Logical Maximum (255)
    0x35, 0x00,       //     Physical Minimum (0)
    0x46, 0xFF, 0x00, //     Physical Maximum (255)
    0x09, 0x30,       //     Usage (X)
    0x09, 0x31,       //     Usage (Y)
    0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x65, 0x00,       //     Unit (None)
    0x75, 0x01,       //     Report Size (1)
    0x95, 0x10,       //     Report Count (16)
    0x25, 0x01,       //     Logical Maximum (1)
    0x45, 0x01,       //     Physical Maximum (1)
    0x05, 0x09,       //     Usage Page (Button)
    0x19, 0x01,       //     Usage Minimum (0x01)
    0x29, 0x10,       //     Usage Maximum (0x10)
    0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,             //   End Collection
    0xC0,             // End Collection
};

usb_config config = {
    .lowspeed = {
        .device = USB_DESC_STATIC(dev_descr),
        .device_qual = USB_DESC_STATIC(devqual_descr),
        .config = USB_DESC_STATIC(cfg_descr),
    },
    .highspeed = {
        .device = USB_DESC_STATIC(dev_descr),
        .device_qual = USB_DESC_STATIC(devqual_descr),
        .config = USB_DESC_STATIC(cfg_descr),
    },

    .langid = USB_DESC_STATIC(langid),
};

static status_t ep_cb_rx(ep_t endpoint, usbc_transfer_t *t);
static status_t ep_cb_tx(ep_t endpoint, usbc_transfer_t *t);

static uint8_t report[5];

static void queue_tx(void)
{
    static usbc_transfer_t transfer;

    report[0] = 1;
    transfer.callback = &ep_cb_tx;
    transfer.result = 0;
    transfer.buf = report;
    transfer.buflen = sizeof(report);
    transfer.bufpos = 0;
    transfer.extra = 0;

    usbc_queue_tx(1, &transfer);
}

static status_t ep_cb_rx(ep_t endpoint, usbc_transfer_t *t)
{
    return NO_ERROR;
}

static status_t ep_cb_tx(ep_t endpoint, usbc_transfer_t *t)
{
#if LOCAL_TRACE
    LTRACEF("ep %u transfer %p\n", endpoint, t);
    usbc_dump_transfer(t);
#endif

    if (t->result >= 0)
        queue_tx();

    return NO_ERROR;
}

static status_t usb_cb(void *cookie, usb_callback_op_t op, const union usb_callback_args *args)
{
    // LTRACEF("cookie %p, op %u, args %p\n", cookie, op, args);
    //printf("cookie %p, op %u, args %p\n", cookie, op, args);

    if (op == USB_CB_ONLINE)
    {
        usbc_setup_endpoint(1, USB_IN, 8, USB_INTR);
        queue_tx();
    }
    else if (op == USB_CB_SETUP_MSG)
    {
        const struct usb_setup *setup = args->setup;
        if (setup->request_type == 0x21 && setup->request == 0xa)
        {
            usbc_ep0_ack();
        }
        else if (setup->request_type == 0x81 && setup->request == 0x6 && setup->value == 0x2200 && setup->index == 0x0)
        {
            usbc_ep0_send(hid_descriptor, sizeof(hid_descriptor), setup->length);
        }
        else
        {
            printf("SETUP: req_type=%#x req=%#x value=%#x index=%#x len=%#x\n",
                   setup->request_type, setup->request, setup->value, setup->index, setup->length);
        }
    }
    return NO_ERROR;
}

void target_usb_setup(void)
{
    usb_setup(&config);
    printf("appending interfaces\n");

    usb_append_interface_lowspeed(if_descriptor_lowspeed, sizeof(if_descriptor_lowspeed));
    usb_append_interface_highspeed(if_descriptor_lowspeed, sizeof(if_descriptor_lowspeed));
    usb_register_callback(&usb_cb, NULL);

    usb_add_string("LK", 1);
    usb_add_string("LK Industries", 2);

    report[1] = 0x80;
    report[2] = 0x80;
    usb_start();
}

void handle_button(unsigned gpio, uint8_t *data, int offset)
{
    int val = gpio_get(gpio);
    const uint8_t mask = 1 << offset;
    if (val)
    {
        *data &= ~mask;
    }
    else
    {
        *data |= mask;
    }
}

bool stm32_exti0_irq(void)
{
    handle_button(GPIO_BUTTON_B, &report[3], 0);
    handle_button(GPIO_BUTTON_Y, &report[3], 1);
    handle_button(GPIO_BUTTON_SELECT, &report[3], 2);
    handle_button(GPIO_BUTTON_START, &report[3], 3);
    handle_button(GPIO_BUTTON_A, &report[3], 4);
    handle_button(GPIO_BUTTON_X, &report[3], 5);
    handle_button(GPIO_BUTTON_L, &report[3], 6);
    handle_button(GPIO_BUTTON_R, &report[3], 7);

    if (!gpio_get(GPIO_BUTTON_LEFT))
    {
        report[1] = 0x0;
    }
    else if (!gpio_get(GPIO_BUTTON_RIGHT))
    {
        report[1] = 0xff;
    }
    else
    {
        report[1] = 0x7f;
    }

    if (!gpio_get(GPIO_BUTTON_UP))
    {
        report[2] = 0x0;
    }
    else if (!gpio_get(GPIO_BUTTON_DOWN))
    {
        report[2] = 0xff;
    }
    else
    {
        report[2] = 0x7f;
    }

    return false;
}