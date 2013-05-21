// hd44780.c
//
// Example program for HD44780 based LCD using bcm2835 library.
// Shows how to interface through GPIO.
//
// After installing bcm2835, you can build it with this command 
// gcc -o hd44780 hd44780.c rpi-hd44780/rpi_hd44780.c -l bcm2835
// and then test
// sudo ./hd44780
//
// Or you can you can modify Makefile 
//
// Detailed Raspberry Pi GPIO info can be found here:
// http://www.combinatorialdesign.com/boards/Raspberry_Pi/
//
// Author: Pawel Wodnicki
// Copyright (C) 2013 Pawel Wodnicki
// $Id: $

/*
   Some of this code has been borrowed bcm2835 library blink` example.
   This example is licensed under GPL V2 same as bcm2835 library.
   
*/

/*
Pinount for 16P or 2x8P and 14P or 2x7P connectors - 4 bit mode only
Two configurations are possible depending on the module VCC
Minimum RO 5V   module - Read Only acess for 5V modules as the RPi GPIO is not 5V tolerant
Maximum RW 3.3V module - Red/Write acess for 3.3V modules only, NOTE all 5V VCC connections must be changed to 3.3V

This pinount does not use any of the UART/I2C nor SPI pins, only GPIO4 
has an alternate function GPIOCLOCK0, the optional GPIO18 has an alternate function PWM, 
GPIO7 could be used when SPI's CE1 is not used 

GPIO Pins are the same for both Rev 2 and Rev 1

P1 conenction diagram can be found here:
http://combinatorialdesign.com/boards/Raspberry_Pi/P1

HD44780    Minimum RO 5V  Maximum RW 3V3                       5V Module  3.3V Module
-----------------------------------------------------------------------------------
1  - VSS   GND                                                 F - P1.6   F - P1.6
2  - VDD   5V VCC         3V3 VCC                              B - P1.2   A - P1.1 
3  - VO    POT Wiper      POT Wiper
4  - R/S   GPIO17         GPIO17          RPI_V2_GPIO_P1_11    K - P1.11  K - P1.11
5  - R/W   5V VCC         GPIO18          RPI_V2_GPIO_P1_12    D - P1.4   L - P1.12
6  - EN    GPIO4          GPIO4           RPI_V2_GPIO_P1_07    G - P1.7   G - P1.7
7  - D0
8  - D1
9  - D2
10 - D3
11 - D4    GPIO22         GPIO22          RPI_V2_GPIO_P1_15    O - P1.15  O - P1.15
12 - D5    GPIO23         GPIO23          RPI_V2_GPIO_P1_16    P - P1.16  P - P1.16
13 - D6    GPIO24         GPIO24          RPI_V2_GPIO_P1_18    R - P1.18  R - P1.18
14 - D7    GPIO25         GPIO25          RPI_V2_GPIO_P1_22    V - P1.22  V - P1.22
-----------------------------------------------------------------------------------
15 - A+    5V VCC         3V3 VCC                              D - P1.4   Q - P1.17 
16 - A-    GND                                                 Y - P1.25  Y - P1.25
-----------------------------------------------------------------------------------
*/
#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include "rpi_hd44780.h"

// forwad declarations
void hd44780_init_gpio(void);
void hd44780_end_gpio(void);
void hd44780_reset();
void hd44780_init(void);
void hd44780_end(void);
void hd44780_transfer_cmd(uint8_t value);
void hd44780_transfer_data(uint8_t value);
void hd44780_probe();


// configuration - comment to disable

int main(int argc, char **argv)
{
   unsigned int pin;
   int x,y;
	
// If you call this, it will not actually access the GPIO
// Use for testing
//        bcm2835_set_debug(1);

    // intialize bcm2835 library
//    if (!bcm2835_init())
//        return 1;


    // initialize gpio
    hd44780_init_gpio();

// pin testing - uncomment one line here and from bcm2835_gpio_fsel to do{...}while(1); loop
// 6 OK pin = RPI_V2_GPIO_P1_22; // RESET - GPIO25
// 7 OK pin = RPI_V2_GPIO_P1_18; // A0 - GPIO24
// 8 OK pin = RPI_V2_GPIO_P1_19; // SDA - GPIO10 MOSI
// 9 OK pin = RPI_V2_GPIO_P1_23; // SCK - GPIO11 SCLK
// 10 OK pin = RPI_V2_GPIO_P1_24; // CS - GPIO8 CE0
//    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
//    do {
//       bcm2835_gpio_clr(pin);
//       delay(5);
//       bcm2835_gpio_set(pin);
//       delay(5);
//    } while(1);

    // initialize display
    hd44780_init();

    do {
       lcd_clear_screen();
       delay(500);
       lcd_set_cursor(LINE1_START);
       lcd_send_string("10 Wires");
       lcd_set_cursor(LINE2_START);
       lcd_send_string("GPIO KIT");
       delay(1000);
    } while(1);

    // deintialize bcm2835 library
    bcm2835_close();

    return 0;
}

// hardware reset
void hd44780_reset()
{
  delay(120);
}

void hd44780_init_gpio()
{
  lcd_set_gpio_pins();
}

// gpio deinit
void hd44780_end_gpio()
{
}

void hd44780_transfer_cmd(uint8_t value)
{
   lcd_send_byte(value, COMMAND);
}

void hd44780_transfer_data(uint8_t value)
{
   lcd_send_byte(value, DATA);
}

// hd44780 init
void hd44780_init(void)
{
  hd44780_reset();
  lcd_init();
  lcd_clear_screen();
  lcd_set_cursor(LINE1_START);
}

// hd44780 deinit
void hd44780_end(void)
{
  hd44780_end_gpio();
}
