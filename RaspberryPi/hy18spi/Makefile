# This Makefile expects bcm2835 library to be build in ../bcm2835

all: hy18spi.c  st7735.h
	gcc -o hy18spi hy18spi.c  -I. -I../bcm2835/src  -L../bcm2835/src -l bcm2835
