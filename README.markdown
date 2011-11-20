mikroProg tools for Linux
=========================

The aim for this project is to maybe some day be able to burn PIC processors
with the mikroProg device from MikroElektronica, using Linux.

Currently, the only tool present is "setvolt", which will enable the Vcc power on the mikroProg.

You can set any voltage between 1.8v and 5v.

This tool requires libusb-1.0-dev package installed.

Usage:
------

    ./setvolt 5
