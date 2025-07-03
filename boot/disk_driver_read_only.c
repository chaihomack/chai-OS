#include <stdint.h>
#include "../mylibs/my_stdtypes.h"

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

#define BSY   0x80
#define DRQ   0x08
#define ERR   0x01

#define IDENTIFY_COMMAND    0xEC
#define READ_COMMAND        0x20

extern BYTE inb(WORD port);
extern void outb(WORD port, BYTE data);
extern WORD inw(WORD port);
extern void outw(WORD port, WORD data);

BYTE ATA_disk_init()
{
	outb(0x3F6, 0x04);  
    outb(0x3F6, 0x00);      //resetting
                      
    int timeout = 1000000;
    while ((inb(STATUS) & BSY) && --timeout);          // waiting BSY=0
    if(inb(STATUS) & ERR) return 1;
    if (timeout == 0) return -1;        //time out
    
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
    
    int timeout = 1000000;
    while (((inb(STATUS) & (BSY | DRQ)) != DRQ) && --timeout);   // wait for BSY=0 and DRQ=1
    if(inb(STATUS) & ERR) return 1;
    if (timeout == 0) return -1;        //time out
    
    WORD* wbuff = (WORD*)buff;
    for (int i = 0; i < 256 * count; i++)
        wbuff[i] = inw(DATA);
    
    return 0; 
}
