/* 
Wonderful GPIO snippet from joan

see original forum thread here:
"Read all the GPIO ports in one action (high speed)"
http://www.raspberrypi.org/phpBB3/viewtopic.php?f=33&t=43895

Code has been polished a bit by elektrknight so it compiles 
and runs as expected. 

Dear joan, 
Yes we do need these headers and C is case sensitive :-) 
But thank you for this little gem!

How to build it? Hm, this should do it: 

gcc read_all.c 

*/

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#define GPIO_BASE  0x20200000
#define GPIO_LEN  0xB4

#define GPSET0     7
#define GPSET1     8
#define GPCLR0    10
#define GPCLR1    11
#define GPLEV0    13
#define GPLEV1    14

static volatile uint32_t  * gpioReg = MAP_FAILED;

uint32_t gpioRead_Bits_0_31(void)
{
   return (*(gpioReg + GPLEV0));
}

void gpioWrite_Bits_0_31_Clear(uint32_t levels)
{
   *(gpioReg + GPCLR0) = levels;
}

void gpioWrite_Bits_0_31_Set(uint32_t levels)
{
   *(gpioReg + GPSET0) = levels;
}

static uint32_t * initMapMem(int fd, uint32_t addr, uint32_t len)
{
    return (uint32_t *) mmap(0, len,
       PROT_READ|PROT_WRITE|PROT_EXEC,
       MAP_SHARED|MAP_LOCKED,
       fd, addr);
}

main()
{
   int fdMem;

   fdMem = open("/dev/mem", O_RDWR | O_SYNC) ;
   gpioReg = initMapMem(fdMem, GPIO_BASE, GPIO_LEN);
   close(fdMem);
   printf("bits 0-31 are %08X\n", gpioRead_Bits_0_31());
}

