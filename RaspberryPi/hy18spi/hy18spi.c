// hy18spi.c
//
// Example program for HY-1.8 SPI LCD using bcm2835 library.
// Shows how to interface with SPI and draws couple of patterns.
//
// After installing bcm2835, you can build it with this command 
// gcc -o hy18spi hy18spi.c -l bcm2835
// and then test
// sudo ./hy18spi
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
   Most of this code has been borrowed from ST7735 frame 
   buffer driver in drivers/video/st7735 and some form 
   bcm2835 library spi example.
   This example is licensed under GPL V2 same as bcm2835 library.
   
*/

/*
HY-1.8 SPI connections

HY-1.8 J1        Raspberry Pi P1      bcm2835 lib pin
1 - GND          - P1.6 GND
2 - VCC          - P1.2 5V Power
3 - NC
4 - NC
5 - NC
6 - RESET        - P1.22 GPIO25        RPI_V2_GPIO_P1_22
7 - A0           - P1.18 GPIO24        RPI_V2_GPIO_P1_18
8 - SDA          - P1.19 GPIO10 MOSI   RPI_V2_GPIO_P1_19
9 - SCK          - P1.23 GPIO11 SCLK   RPI_V2_GPIO_P1_23
10 - CS          - P1.24 GPIO8 CE0     RPI_V2_GPIO_P1_24
11 - SD_SCK
12 - SD_MISO
13 - SD_MOSI
14 - SD_CS
15 - LED+         - P1.4 5V Power
16 - LED-         - P1.9 GND

*/

#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include <st7735.h>

// forwad declarations
void hy18spi_init_gpio(void);
void hy18spi_end_gpio(void);
void hy18spi_reset();
void hy18spi_init(void);
void hy18spi_end(void);
uint8_t hy18spi_transfer_cmd(uint8_t value);
uint8_t hy18spi_transfer_data(uint8_t value);
void hy18spi_probe();
void hy18spi_set_rgb(uint16_t color);
void hy18spi_set_pixel(uint16_t x, uint16_t y, uint16_t color);

#define HY18SPI_WIDTH 128
#define HY18SPI_HEIGHT 160

// configuration - comment to disable
#define CONFIG_FB_ST7735_PANEL_TYPE_RED_TAB 1
#define CONFIG_FB_ST7735_RGB_ORDER_REVERSED 1

#if ( CONFIG_FB_ST7735_PANEL_TYPE_RED_TAB == 1 )
# define ST7735_COLSTART 0
# define ST7735_ROWSTART 0
#else
# define ST7735_COLSTART 2
# define ST7735_ROWSTART 1
#endif

/* Init script function */
struct st7735_function {
        uint16_t cmd;
        uint16_t data;
};

/* Init script commands */
enum st7735_cmd {
        ST7735_START,
        ST7735_END,
        ST7735_CMD,
        ST7735_DATA,
        ST7735_DELAY
};

static struct st7735_function st7735_cfg_script[] = {
        { ST7735_START, ST7735_START},
        { ST7735_CMD, ST7735_SWRESET},
        { ST7735_DELAY, 150},
        { ST7735_CMD, ST7735_SLPOUT},
        { ST7735_DELAY, 500},
        { ST7735_CMD, ST7735_FRMCTR1},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x2d},
        { ST7735_CMD, ST7735_FRMCTR2},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x2d},
        { ST7735_CMD, ST7735_FRMCTR3},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x2d},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x2d},
        { ST7735_CMD, ST7735_INVCTR},
        { ST7735_DATA, 0x07},
        { ST7735_CMD, ST7735_PWCTR1},
        { ST7735_DATA, 0xa2},
        { ST7735_DATA, 0x02},
        { ST7735_DATA, 0x84},
        { ST7735_CMD, ST7735_PWCTR2},
        { ST7735_DATA, 0xc5},
        { ST7735_CMD, ST7735_PWCTR3},
        { ST7735_DATA, 0x0a},
        { ST7735_DATA, 0x00},
        { ST7735_CMD, ST7735_PWCTR4},
        { ST7735_DATA, 0x8a},
        { ST7735_DATA, 0x2a},
        { ST7735_CMD, ST7735_PWCTR5},
        { ST7735_DATA, 0x8a},
        { ST7735_DATA, 0xee},
        { ST7735_CMD, ST7735_VMCTR1},
        { ST7735_DATA, 0x0e},
        { ST7735_CMD, ST7735_INVOFF},
        { ST7735_CMD, ST7735_MADCTL},
#if ( CONFIG_FB_ST7735_RGB_ORDER_REVERSED == 1 )
        { ST7735_DATA, 0xc0},
#else
        { ST7735_DATA, 0xc8},
#endif
        { ST7735_CMD, ST7735_COLMOD},
        { ST7735_DATA, 0x05},
#if 0 /* set_addr_win will set these, so no need to set them at init */
        { ST7735_CMD, ST7735_CASET},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x00 + ST7735_COLSTART},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, HY18SPI_WIDTH - 1 + ST7735_COLSTART},
        { ST7735_CMD, ST7735_RASET},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x00 + ST7735_ROWSTART},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, HY18SPI_HEIGHT - 1 + ST7735_ROWSTART},
#endif
        { ST7735_CMD, ST7735_GMCTRP1},
        { ST7735_DATA, 0x02},
        { ST7735_DATA, 0x1c},
        { ST7735_DATA, 0x07},
        { ST7735_DATA, 0x12},
        { ST7735_DATA, 0x37},
        { ST7735_DATA, 0x32},
        { ST7735_DATA, 0x29},
        { ST7735_DATA, 0x2d},
        { ST7735_DATA, 0x29},
        { ST7735_DATA, 0x25},
        { ST7735_DATA, 0x2b},
        { ST7735_DATA, 0x39},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x03},
        { ST7735_DATA, 0x10},
        { ST7735_CMD, ST7735_GMCTRN1},
        { ST7735_DATA, 0x03},
        { ST7735_DATA, 0x1d},
        { ST7735_DATA, 0x07},
        { ST7735_DATA, 0x06},
        { ST7735_DATA, 0x2e},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x29},
        { ST7735_DATA, 0x2d},
        { ST7735_DATA, 0x2e},
        { ST7735_DATA, 0x2e},
        { ST7735_DATA, 0x37},
        { ST7735_DATA, 0x3f},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x02},
        { ST7735_DATA, 0x10},
///#if 0 /* init_display will turn on the display after clearing it */
        { ST7735_CMD, ST7735_DISPON},
        { ST7735_DELAY, 100},
///#endif
        { ST7735_CMD, ST7735_NORON},
        { ST7735_DELAY, 10},
        { ST7735_END, ST7735_END},
};

int main(int argc, char **argv)
{
   unsigned int pin;
   int x,y;
	
// If you call this, it will not actually access the GPIO
// Use for testing
//        bcm2835_set_debug(1);

    // intialize bcm2835 library
    if (!bcm2835_init())
        return 1;

    // initialize spi
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);                   // MODE 3
///    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
/// NOT OK    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4); //< 4 = 16ns = 62.5MHz
/// OK
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8); //< 8 = 32ns = 31.25MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16); //< 16 = 64ns = 15.625MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32); //< 32 = 128ns = 7.8125MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); //< 64 = 256ns = 3.90625MHz - 4000000 spi clock
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

    // initialize gpio
    hy18spi_init_gpio();

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
    hy18spi_init();

    // set black background
    hy18spi_set_rgb(0x0000);

    // wait a sec
    delay(1000);
   
    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x+y);
      }
    }
    // wait a 10 sec
    delay(1000);
 
    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x*y);
      }
    }
    // wait a 10 sec
    delay(1000);

    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x|y);
      }
    }
    // wait a 10 sec
    delay(1000);

    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x&y);
      }
    }
    // wait a 10 sec
    delay(1000);

    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x^y);
      }
    }
    // wait a 10 sec
    delay(1000);

    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, (x+y)*(x+y));
      }
    }
    // wait a 10 sec
    delay(1000);

    // set a fancy background
    hy18spi_set_rgb(0xf0f0);
    // wait a sec
    delay(1000);

    // deinitialize display
    hy18spi_end();

    // deintialize spi
    bcm2835_spi_end();

    // deintialize bcm2835 library
    bcm2835_close();

    return 0;
}

// hardware reset
void hy18spi_reset()
{
  bcm2835_gpio_clr(RPI_V2_GPIO_P1_22); //GPIO25 - RESET
  delay(10);
  bcm2835_gpio_set(RPI_V2_GPIO_P1_22); //GPIO25 - RESET
  delay(120);
}

void hy18spi_init_gpio()
{
   // SPI pins have been initialized including GPIO8 - CS
   // A0 and RESET pins set to output
   bcm2835_gpio_fsel(RPI_V2_GPIO_P1_18, BCM2835_GPIO_FSEL_OUTP); // A0 - GPIO24
   bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_OUTP); // RESET - GPIO25
   // A0 set high
   bcm2835_gpio_set(RPI_V2_GPIO_P1_18); //GPIO24 - A0
   // RESET set high
   bcm2835_gpio_set(RPI_V2_GPIO_P1_22); //GPIO25 - RESET
   // CS set high
   bcm2835_gpio_set(RPI_V2_GPIO_P1_24); //GPIO8 - CS

}

// gpio deinit
void hy18spi_end_gpio()
{
   // A0 and RESET pins set to input
   bcm2835_gpio_fsel(RPI_V2_GPIO_P1_18, BCM2835_GPIO_FSEL_INPT); // A0 - GPIO24
   bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_INPT); // RESET - GPIO25
}

uint8_t hy18spi_transfer_cmd(uint8_t value)
{
   bcm2835_gpio_clr(RPI_V2_GPIO_P1_18); //GPIO24 - A0
   return bcm2835_spi_transfer(value);
}

uint8_t hy18spi_transfer_data(uint8_t value)
{
   bcm2835_gpio_set(RPI_V2_GPIO_P1_18); //GPIO24 - A0
   return bcm2835_spi_transfer(value);
}

void hy18spi_set_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  hy18spi_transfer_cmd(ST7735_CASET);         // column addr set
  hy18spi_transfer_data(0x00);
  hy18spi_transfer_data(x0+ST7735_COLSTART);  // XSTART
  hy18spi_transfer_data(0x00);
  hy18spi_transfer_data(x1+ST7735_COLSTART);  // XEND

  hy18spi_transfer_cmd(ST7735_RASET);         // row addr set
  hy18spi_transfer_data(0x00);
  hy18spi_transfer_data(y0+ST7735_ROWSTART);  // YSTART
  hy18spi_transfer_data(0x00);
  hy18spi_transfer_data(y1+ST7735_ROWSTART);  // YEND
}

void hy18spi_set_rgb(uint16_t color)
{
  uint8_t x, y;
  hy18spi_set_addr_window(0, 0, HY18SPI_WIDTH - 1, HY18SPI_HEIGHT - 1);
  hy18spi_transfer_cmd(ST7735_RAMWR);  // write to RAM
  for (x=0; x < HY18SPI_WIDTH; x++)
  {
    for (y=0; y < HY18SPI_HEIGHT; y++)
    {
      hy18spi_transfer_data(color >> 8);
      hy18spi_transfer_data(color);
    }
  }
  hy18spi_transfer_cmd(ST7735_NOP);
}


void hy18spi_set_pixel(uint16_t x, uint16_t y, uint16_t color)
{
  hy18spi_set_addr_window(x,y,x+1,y+1);
  hy18spi_transfer_cmd(ST7735_RAMWR);  // write to RAM
  hy18spi_transfer_data(color >> 8);
  hy18spi_transfer_data(color);
}

static void hy18spi_run_cfg_script()
{
        int i = 0;
        int end_script = 0;

        do {
                switch (st7735_cfg_script[i].cmd)
                {
                case ST7735_START:
                        break;
                case ST7735_CMD:
                        hy18spi_transfer_cmd(st7735_cfg_script[i].data & 0xff);
                        break;
                case ST7735_DATA:
                        hy18spi_transfer_data(st7735_cfg_script[i].data & 0xff);
                        break;
                case ST7735_DELAY:
                        delay(st7735_cfg_script[i].data);
                        break;
                case ST7735_END:
                        end_script = 1;
                }
                i++;
        } while (!end_script);
}
// hy-1.8 init
void hy18spi_init(void)
{
  hy18spi_reset();
  hy18spi_run_cfg_script();
}

// hy-1.8 deinit
void hy18spi_end(void)
{
  hy18spi_end_gpio();
}


