# This Makefile expects bcm2835 library to be build in ../bcm2835

all: hd44780.c  rpi-hd44780/rpi_hd44780.c rpi-hd44780/rpi_hd44780.h
	gcc -o hd44780 hd44780.c rpi-hd44780/rpi_hd44780.c -I. -Irpi-hd44780 -I../bcm2835/src  -L../bcm2835/src -l bcm2835
