/*
 * libusb example program to list devices on the bus
 * Copyright © 2007 Daniel Drake <dsd@gentoo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Compile with:
 * 	gcc $(pkgconf --cflags libusb-1.0) <SOURCE> $(pkgconf --libs libusb-1.0) -o <EXECUTABLE>
 *
 */

#include <stdio.h>
#include <unistd.h>

#include "libusb.h"

/* x and z are the x and y coordinates of the navring (-512 to +512)
 * y is the position of the tooltuner dial, leftmost quadrant is 0
 * 	going anticlockwise to -4096
 * ry is the rotation of the grey ring (-256 to +256)
 */

enum nlq_leds {
	LED_TOP = 0x01, // click triggerpoints
	LED_BOTTOM = 0x02,
	LED_LEFT = 0x04,
	LED_RIGHT = 0x08,
	LED_CENTRE = 0x10 // touch triggerpoint
};

void
setclick(libusb_device_handle *handle, int state)
{
	char click_bits[] = {0x05, 0x01, 0x04, 0x01};
	click_bits[1] = state ? 0xa0 : 0x01;
	libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS, 0x09, 0x305,
				0, click_bits, 0x4, 1000);
}

void
setleds(libusb_device_handle *handle, int top, int bottom, int left, int right,
	int centre)
{
	char led_bits[] = {0x04, 0x1f};
	led_bits[1] = (top & LED_TOP) | (bottom & LED_BOTTOM) |
		      (left & LED_LEFT) | (right & LED_RIGHT) |
		      (centre & LED_CENTRE);
	libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS, 0x09, 0x204,
				0, led_bits, 0x2, 1000);
}

int
main(void)
{
	int r;

	r = libusb_init(NULL);
	if (r < 0) return r;

	libusb_device_handle *nulooq =
		libusb_open_device_with_vid_pid(NULL, 0x046d, 0xc640);
	if (nulooq) {
		setclick(nulooq, 1);

		for (int i = 0; i < 0x20; i++) {
			setleds(nulooq, i & 0x01, i & 0x02, i & 0x04, i & 0x08,
				i & 0x10);
			usleep(200000);
		}

		setclick(nulooq, 0);

	} else {
		printf("Failed to attach to NuLOOQ. Check permissions.\n");
	}

	libusb_exit(NULL);
	return 0;
}
