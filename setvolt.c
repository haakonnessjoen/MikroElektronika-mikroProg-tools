/*
    setvolt - Power your board from MikroElektronika mikroProg(tm) programmer

    Copyright (C) 2011, Håkon Nessjøen <haakon.nessjoen@gmail.com>
    All trade and/or services marks mentioned are the property of their respective owners.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

static struct libusb_device_handle *devh = NULL;

static int find_mikroprog_device(void) {
	devh = libusb_open_device_with_vid_pid(NULL, 0x3e1a, 0x0200);
	return devh != NULL ? 0 : -EIO;
}

void cleanup() {
	libusb_release_interface(devh, 0);
	libusb_close(devh);
	libusb_exit(NULL);
}

int main(int argc, char **argv) {
	int ret;
	float volts;
	int written = 0;
	unsigned char req_stop[] = { 0x01, 0x20, 0x2 };
	unsigned char req_start[] = { 0x01, 0x20, 0x1 };
	unsigned char req_volt[] = { 0x01, 0x10, 0x0 };
	unsigned char buffer[1];

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <0|1.8-5>\n", argv[0]);
		return 1;
	}

	volts = atof(argv[1]);

	if (volts != 0 && (volts < 1.8f || volts > 5)) {
		fprintf(stderr, "Invalid voltage level. Must be between 1.8v and 5v\n");
		return 1;
	}

	ret = libusb_init(NULL);
	if (ret < 0) {
		fprintf(stderr, "failed to initialise libusb\n");
		exit(1);
	}

	ret = find_mikroprog_device();
	if (ret < 0) {
		fprintf(stderr, "Could not find/open Mikroelektronika mikroProg device\n");
		goto out;
	}

	ret = libusb_claim_interface(devh, 0);
	if (ret < 0) {
		fprintf(stderr, "usb_claim_interface error %d\nDo you have sufficient rights?\n", ret);
		goto out;
	}

	if (volts == 0) {
		libusb_bulk_transfer(devh, (2 | LIBUSB_ENDPOINT_OUT), req_stop, sizeof(req_stop), &written, 1000);
		if (written < sizeof(req_stop)) {
			fprintf(stderr, "Error writing data to usb device\n");
			goto out;
		}
		libusb_bulk_transfer(devh, (2 | LIBUSB_ENDPOINT_IN), buffer, sizeof(buffer), &written, 1000);
		if (written != 1 || buffer[0] != 0) {
			fprintf(stderr, "Error deactivating power: Did not receive 'ok' signal from mikroProg\n");
			goto out;
		}
		printf("Power off\n");
		goto out;
	}

	req_volt[2] = 58 + (unsigned char)((volts - 1.8f) * 52.9f); // 5v
	libusb_bulk_transfer(devh, (2 | LIBUSB_ENDPOINT_OUT), req_volt, sizeof(req_volt), &written, 1000);
	if (written < sizeof(req_volt)) {
		fprintf(stderr, "Error writing data to usb device\n");
		goto out;
	}
	libusb_bulk_transfer(devh, (2 | LIBUSB_ENDPOINT_IN), buffer, sizeof(buffer), &written, 1000);
	if (written != 1 || buffer[0] != 0) {
		fprintf(stderr, "Error setting voltage: Did not receive 'ok' signal from mikroProg\n");
		goto out;
	}

	libusb_bulk_transfer(devh, (2 | LIBUSB_ENDPOINT_OUT), req_start, sizeof(req_start), &written, 1000);
	if (written < sizeof(req_start)) {
		fprintf(stderr, "Error writing data to usb device\n");
		goto out;
	}
	libusb_bulk_transfer(devh, (2 | LIBUSB_ENDPOINT_IN), buffer, sizeof(buffer), &written, 1000);
	if (written != 1 || buffer[0] != 0) {
		fprintf(stderr, "Error activiting power: Did not receive 'ok' signal from mikroProg\n");
		goto out;
	}
	printf("Power was set to %.1f Volt\n", volts);

	
	libusb_release_interface(devh, 0);
out:
	libusb_close(devh);
	libusb_exit(NULL);
	return ret >= 0 ? ret : -ret;

}

