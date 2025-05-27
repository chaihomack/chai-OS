
#include "disk_driver.h"
#include "../../mylibs/my_stdlib.h"
#include "stdint.h"

#define DATA            0x1F0
#define SECTOR_COUNT    0x1F2
#define LBA_LOW         0x1F3
#define LBA_MID         0x1F4
#define LBA_HIGH        0x1F5
#define DRIVE           0x1F6
#define COMMAND         0x1F7
#define STATUS          0x1F7

#define BSY   0x80
#define DRQ   0x08
#define ERR   0x01

#define IDENTIFY_COMMAND 0xEC

BYTE inb(WORD port)
{
    BYTE result;
    __asm__ volatile (
        "inb %1, %0" 
        : "=a" (result)    
        : "Nd" (port)      
    );
    return result;
}


void outb(WORD port, BYTE data)
{
    __asm__ volatile (
        "outb %0, %1"
        :                  
        : "a" (data),      
          "Nd" (port)      
    );
}

WORD inw(WORD port)
{
    unsigned short result;
    __asm__ volatile (
        "inw %1, %0"
        : "=a" (result)    
        : "Nd" (port)      
    );
    return result;
}

void outw(WORD port, WORD data)
{
    __asm__ volatile (
        "outw %0, %1"
        :
        : "a" (data),      
          "Nd" (port)      
    );
}


BYTE ATA_disk_status()
{
	return inb(STATUS);
}

BYTE ATA_disk_init()
{
	outb(0x3F6, 0x04);  
    outb(0x3F6, 0x00);                  //resetting
    while (inb(STATUS) & BSY);          // waiting for BSY=0
    outb(DRIVE, 0xE0 | 0xA0);           // LBA + Master

    outb(COMMAND, 0xEC);                // Identify
    while (!(inb(STATUS) & 0x08));      // waiting DRQ=1

    for (int i = 0; i < 256; i++) {
        WORD tmp = inw(DATA);
        (void)tmp;                  //reading output... but ignoring
    }

	return 0;
}

int ATA_disk_read(BYTE *buff, DWORD sector, DWORD count) 
{
    outb(DRIVE, 0xE0 | ((sector >> 24) & 0x0F)); // master and LBA
    
    outb(SECTOR_COUNT, count); 
    
    outb(LBA_LOW, sector & 0xFF);         //24 bits for adressing
    outb(LBA_MID, (sector >> 8) & 0xFF);   
    outb(LBA_HIGH, (sector >> 16) & 0xFF); 
    
    outb(COMMAND, 0x20); //read command
    
    while (1) 
	{
        BYTE status = inb(STATUS);

        if (status & 0x01) return 1;      // ERR=1
        if (!(status & 0x80) && (status & 0x08)) break; // BSY=0 and DRQ=1
    }
	
    for (int i = 0; i < 256 * count; i++) 
	{
        *((WORD*)buff) = inw(DATA);
        buff += 2;  
    }
    
    return 0; 
}

int ATA_disk_write(const BYTE *buff, DWORD sector, DWORD count) 
{    
    if (!buff || count == 0) return -1;

    outb(DRIVE, 0xE0 | ((sector >> 24) & 0x0F));  // LBA + Master

    outb(SECTOR_COUNT, count);        // number of sectors
    outb(LBA_LOW, sector);       // LBA Low
    outb(LBA_MID, sector >> 8);  // LBA Mid
    outb(LBA_HIGH, sector >> 16); // LBA High

    outb(COMMAND, 0x30);  //write command

    while ((inb(STATUS) & (BSY | DRQ)) != DRQ); // waiting for BSY = 0 and DRQ = 1

    for (int i = 0; i < 256 * count; i++) {
        outw(DATA, *((uint16_t*)buff));   // writing
        buff += 2;
    }

    if(inb(STATUS) & ERR)
        return -1;
    return 0;
}

int ATA_disk_flush() 
{    
    outb(DRIVE, 0xE0 | 0xA0);  // LBA + Master
    
    outb(COMMAND, 0xE7);  // flush command
    
    DWORD timeout = 1000000;  //timeout
    while (--timeout) {
        BYTE status = inb(STATUS);
        
        if (status & ERR) {
            return -1;  
        }
        
        if ((status & (STATUS | DRQ)) == 0) {
            return 0;  
        }
        
        for (volatile int i = 0; i < 1000; i++);
    }
    
    return -2;  // timeout
} //idk

WORD ATA_get_word_from_DISK_IDENTIFY(unsigned int word_number)
{
    outb(COMMAND, IDENTIFY_COMMAND);
    
    while ((inb(STATUS) & (BSY | DRQ)) != DRQ); //waiting for BSY = 0 and DRQ = 1

    WORD buffer[256];

    for (int i = 0; i < 256; i++)
    {
        buffer[i] = inw(DATA);
    }

    return buffer[word_number];
}

unsigned int ATA_get_sector_count()
{
    WORD lo = ATA_get_word_from_DISK_IDENTIFY(60);
    WORD hi = ATA_get_word_from_DISK_IDENTIFY(61);

    return ((unsigned int)hi << 16) | lo;
}
