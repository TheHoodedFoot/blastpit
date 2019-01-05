#include "nulooq.h"

#if defined(SPACENAV)
#include <X11/Xlib.h>
#include <spnav.h>
#endif
#include <stdio.h>
#include <stdlib.h>

Nulooq::Nulooq(QObject *parent) : QObject(parent) {}
void
Nulooq::Loop()
{
#if defined(SPACENAV)
	Display *dpy;
	Window xWin;
	unsigned long pixels;

	if (!(dpy = XOpenDisplay(0))) {
		throw NULOOQ_NO_X;
	}
	pixels = BlackPixel(dpy, DefaultScreen(dpy));
	xWin = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0,
				   pixels, pixels);

	if (spnav_x11_open(dpy, xWin) == -1) {
		fprintf(stderr,
			"NuLOOQ error - The spacenav daemon cannot be "
			"found.");
		/* throw NULOOQ_NO_DAEMON; */
		return;
	}

	spnav_event nlq;

	while (spnav_wait_event(&nlq)) {
		if (nlq.type == SPNAV_EVENT_MOTION) {
			emit motionEvent(nlq.motion.x, nlq.motion.y,
					 nlq.motion.z, nlq.motion.rx,
					 nlq.motion.ry, nlq.motion.rz);
		} else {
			emit buttonEvent(nlq.button.bnum, nlq.button.press);
		}
	}

	spnav_close();
#endif
}

/* x and z are the x and y coordinates of the navring (-512 to +512)
 * y is the position of the tooltuner dial, leftmost quadrant is 0
 * 	going anticlockwise to -4096
 * ry is the rotation of the grey ring (-256 to +256)
 */

/*
 * LEDs can be controlled easily using libusb. If you adapt the libusb test
 * program to obtain a handle to the NuLOOQ (VENDOR_ID 0x046d, PRODUCT_ID
 * 0xc640), then the following code can control the LEDs:
 *
 * char led_bits[] = { 0x04, 0x1F };
 * retval = usb_control_msg(mydevice_handle, USB_TYPE_CLASS, 0x09, 0x204, 0,
 * led_bits, 0x2, 1000);
 *
 * The second byte in the led_bits array (0x1F) controls which LEDs are lit or
 * not. The five least significant bits control the five LEDs, ranging from 0
 * (no LEDs) to 0x1F (all LEDs.)
 *
 * The click can be turned on or off in a similar fashion:
 *
 * static char click_off[] = { 0x05, 0x01, 0x04, 0x01 };
 * static char click_on[] = { 0x05, 0xA0, 0x04, 0x01 };
 *
 * This turns the click on:
 *
 * retval = usb_control_msg(mydevice_handle, USB_TYPE_CLASS, 0x09, 0x305, 0,
 * click_on, 0x4, 1000);
 *
 * and this turns it off:
 *
 * retval = usb_control_msg(mydevice_handle, USB_TYPE_CLASS, 0x09, 0x305, 0,
 * click_off, 0x4, 1000);
 */

/* Now very likely libusb_control_transfer(....) */
