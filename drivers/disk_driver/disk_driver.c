
#include "disk_driver.h"
#include "../../mylibs/my_stdlib.h"
#include "stdint.h"
#include "../asm/io_asm.h"

#define DATA            0x1F0
#define SECTOR_COUNT    0x1F2
#define LBA_LOW         0x1F3
#define LBA_MID         0x1F4
#define LBA_HIGH        0x1F5
#define DRIVE           0x1F6
#define COMMAND         0x1F7
#define STATUS          0x1F7

#define DRIVE_MASTER_CHS 0xA0
#define DRIVE_SLAVE_CHS  0xB0
#define DRIVE_MASTER_LBA 0xE0
#define DRIVE_SLAVE_LBA  0xF0

#define BSY   0x80  // busy
#define DRQ   0x08  // data request
#define ERR   0x01

#define IDENTIFY_COMMAND    0xEC
#define READ_COMMAND        0x20
#define WRITE_COMMAND       0x30
#define FLUSH_COMMAND       0xE7

#define CHECK_ERR                       (inb(STATUS) & ERR)
#define CHECK_DRQ                       (inb(STATUS) & DRQ)
#define CHECK_BSY                       (inb(STATUS) & BSY)

#define WAIT_BSY_OFF(timeout)           while (CHECK_BSY && --timeout)
#define WAIT_DRQ_ON(timeout)            while (!CHECK_DRQ && --timeout)
#define WAIT_BSY_OFF_DRQ_ON(timeout)    while (((inb(STATUS) & (BSY | DRQ)) != DRQ) && --timeout)

int wait_BSY_off()
{
    int timeout = 1000000;
    WAIT_BSY_OFF(timeout);
    if (timeout == 0) return -1;        //time out
    if (CHECK_ERR) return 1;
    return 0;
}

int wait_DRQ_on()
{
    int timeout = 1000000;
    WAIT_DRQ_ON(timeout);
    if (timeout == 0) return -1;        //time out
    if(CHECK_ERR) return 1;
    return 0;
}

int wait_BSY_off_DRQ_on()
{
    int timeout = 1000000;
    WAIT_BSY_OFF_DRQ_ON(timeout);
    if(CHECK_ERR) return 1;
    if (timeout == 0) return -1;        //time out
    return 0;
}

BYTE ATA_disk_status()
{
	return inb(STATUS);
}

BYTE ATA_disk_init()
{
	outb(0x3F6, 0x04);  
    outb(0x3F6, 0x00);                  //resetting
    wait_BSY_off();
    outb(DRIVE, DRIVE_MASTER_LBA | DRIVE_MASTER_CHS);           // LBA + Master

    outb(COMMAND, IDENTIFY_COMMAND);

    for (int i = 0; i < 512; i++) {
        BYTE tmp = inb(DATA);
        (void)tmp;                 //reading output... but ignoring
    }

	return 0;
}

int ATA_disk_read(BYTE *buff, uint32_t sector, uint32_t count) 
{
    outb(DRIVE, DRIVE_MASTER_LBA | ((sector >> 24) & 0x0F)); // master and LBA
    
    outb(SECTOR_COUNT, count); 
    
    outb(LBA_LOW, sector & 0xFF);         //24 bits for adressing
    outb(LBA_MID, (sector >> 8) & 0xFF);   
    outb(LBA_HIGH, (sector >> 16) & 0xFF); 
    
    outb(COMMAND, READ_COMMAND); 
    
    int rc = wait_BSY_off_DRQ_on();
    if (rc) return rc;
    
    WORD *wbuff = (WORD*) buff;
    for (int i = 0; i < 256 * count; i++) 
        wbuff[i] = inw(DATA);  
        
    return 0; 
}

int ATA_disk_write(const BYTE *buff, uint32_t sector, uint32_t count) 
{
    if (!buff || count == 0) return -1;

    outb(DRIVE, DRIVE_MASTER_LBA | ((sector >> 24) & 0x0F));  // LBA + Master

    outb(SECTOR_COUNT, count);
    outb(LBA_LOW, sector);
    outb(LBA_MID, sector >> 8);
    outb(LBA_HIGH, sector >> 16);

    outb(COMMAND, WRITE_COMMAND);  // write command

    const WORD *wbuff = (const WORD*) buff;
    for (uint32_t i = 0; i < 256 * count; i++)
        outw(DATA, wbuff[i]);

    int rc = wait_BSY_off();
    if (rc) return rc;

    return 0;
}


int ATA_disk_flush() 
{    
    outb(DRIVE, DRIVE_MASTER_LBA | DRIVE_MASTER_CHS);  // LBA + Master
    
    outb(COMMAND, FLUSH_COMMAND);  // flush command
    
    DWORD timeout = 1000000;  //timeout
    while (--timeout) {
        BYTE status = inb(STATUS);
        
        if (CHECK_ERR) {
            return -1;  
        }
        
        if (CHECK_DRQ == 0) {
            return 0;  
        }
        
        for (volatile int i = 0; i < 1000; i++);
    }
    
    return -2;  // timeout
} //idk

WORD ATA_get_word_from_DISK_IDENTIFY(uint32_t word_number)
{
    outb(COMMAND, IDENTIFY_COMMAND);
    
    int rc = wait_BSY_off_DRQ_on();
    if (rc) return rc;

    WORD buffer[256];

    for (int i = 0; i < 256; i++)
    {
        buffer[i] = inw(DATA);
    }

    return buffer[word_number];
}

uint32_t ATA_get_sector_count()
{
    WORD lo = ATA_get_word_from_DISK_IDENTIFY(60);
    WORD hi = ATA_get_word_from_DISK_IDENTIFY(61);

    return ((unsigned int)hi << 16) | lo;
}
