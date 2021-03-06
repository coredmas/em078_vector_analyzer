/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#pragma O0

#include "../include/diskio.h"		/* FatFs lower layer API */
#include <stdio.h>

#include <alt_sdmmc.h>
#include <alt_globaltmr.h>
#include <hps.h>
#include <socal.h>
#include <alt_clock_manager.h>

#define MBR_SIGNATURE   0xAA55
#define MBR_SIG_ADDR    0x1FE
#define MBR_P1_ADDR     0x1BE
#define MBR_P2_ADDR     0x1CE
#define MBR_P3_ADDR     0x1DE
#define MBR_P4_ADDR     0x1EE
#define MBR_PENTRY_SZ   0x10


// matching MBR (master boot record) partition struct
typedef struct mbr_partition_entry {
    uint8_t     bootable;   // Boot Indicator (0x80 = bootable)
    uint8_t     schs_b1;    // starting chs value byte1
    uint8_t     schs_b2;    // starting chs value byte2
    uint8_t     schs_b3;    // starting chs value byte3
    uint8_t     p_type;     // Partition-Type
    uint8_t     echs_b1;    // ending chs value byte1
    uint8_t     echs_b2;    // ending chs value byte2
    uint8_t     echs_b3;    // ending chs value byte3
    uint16_t    lba_hw1;    // logical block address of first sector in partition
    uint16_t    lba_hw2;    // split into half-words since partition addresses are hot aligned on word boundary
    uint16_t    size_hw1;   // number of sectors in partition
    uint16_t    size_hw2;
} mbr_partition_entry_t;

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */



	 // Card information
 ALT_SDMMC_CARD_INFO_t Card_Info;



volatile		 int fat_sd_base;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	//DSTATUS stat;
	if (pdrv != 0) return STA_NOINIT;


	// should check STA_NODISK
	return 0;
}




/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{

	ALT_STATUS_CODE status;


	//int stat;
	int size;
	 int fat;
	 int sd_base;
	 // SD/MMC Device size - hardcode max supported size for now
		 uint64_t Sdmmc_Device_Size;

	 // SD/MMC Block size
// uint32_t Sdmmc_Block_Size;

		 ALT_SDMMC_CARD_MISC_t card_misc_cfg;

BYTE buff[512];
int i;
status = alt_clk_is_enabled(ALT_CLK_L4_SP);
	if (status == ALT_E_FALSE) status = alt_clk_clock_enable(ALT_CLK_L4_SP);

// Initialize global timer
   if(!alt_globaltmr_int_is_enabled())
       {
           status = alt_globaltmr_init();
       }

   // Setting up SD/MMC
       alt_sdmmc_init();
       alt_sdmmc_card_pwr_on();

   	   alt_sdmmc_card_identify(&Card_Info);

		    			       // case ALT_SDMMC_CARD_TYPE_MMC:
		    			       //     print("INFO: MMC Card detected.\n");

		    			        alt_sdmmc_card_bus_width_set(&Card_Info, ALT_SDMMC_BUS_WIDTH_4);
		    			    //    alt_sdmmc_fifo_param_set((ALT_SDMMC_FIFO_NUM_ENTRIES >> 3) - 1,  (ALT_SDMMC_FIFO_NUM_ENTRIES >> 3), ALT_SDMMC_MULT_TRANS_TXMSIZE1);
		    			        alt_sdmmc_card_misc_get(&card_misc_cfg);

		    			  //      Sdmmc_Block_Size = card_misc_cfg.block_size;
		    			        Sdmmc_Device_Size = ((uint64_t)Card_Info.blk_number_high << 32) + Card_Info.blk_number_low;
		    			        Sdmmc_Device_Size *= Card_Info.max_r_blkln;
		    			      //  stat = alt_sdmmc_dma_enable();

		    			    	//printf("SD/MMC block size %d; ",Sdmmc_Block_Size);
		    			    //	printf("device size %ll;\n\r",Sdmmc_Device_Size);



		    			    	alt_sdmmc_read(&Card_Info, buff, (void*)(0), 512);
		    			    	if (alt_read_hword(&buff[MBR_SIG_ADDR]) == MBR_SIGNATURE)
		    			    	        printf("MBR Table Found\n\r");
		    			    	fat = -1;
		    			    	 for (i = 0; i < 4; i++) {
		    			    	            mbr_partition_entry_t *mbr_entry = (mbr_partition_entry_t *) &buff[(MBR_P1_ADDR + (MBR_PENTRY_SZ * i))];
		    			    	           // if (0xA2 == mbr_entry->p_type) printf("0xA2 - "); else
		    			    	           printf ("Partition type 0x%2x - ", mbr_entry->p_type);

		    			    	                sd_base = (((mbr_entry->lba_hw2) << 16) | mbr_entry->lba_hw1); //get block address
		    			    	                if ((11 == mbr_entry->p_type) && (fat==-1))
		    			    	                {
		    			    	                	fat = i;
		    			    	                	fat_sd_base=sd_base;
		    			    	                }
		    			    	                size = (((mbr_entry->size_hw2) << 16) | mbr_entry->size_hw1);

		    			    	                printf("Partition %d; size %d, sd_base %d\n\r", i, size, sd_base );

		    			    	 }
		    			    	 printf("Using FAT partition %d", fat);

//		    			    	 printf("first 10 sectors of FAT partition\r\n");
//
//
//		    			    	for (j=0;j<10;j++) {
//		    			    		printf("sector %d\r\n",j);
//		    			    		stat = alt_sdmmc_read(&Card_Info, buff, (void*)(512*(j+fat_sd_base)), 512);
//
//		    			    		for (i=0;i<512;i++){
//
//		    			    			printf("%c",buff[i]);
//		    			    	if (i%32 ==31 ) printf("\r\n");
//
//		    			    		}
//		    			    	}
		    	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	//BYTE *buff,		/* Data buffer to store read data */
	BYTE *buff,
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{


	//DRESULT res;

	//printf("read...\r\n");
	  //printf("start sector %d; number of sectors %d",sector,count);
	if (pdrv != 0)
		return RES_PARERR;
	else
	{
		alt_sdmmc_read(&Card_Info, buff, (void*)((fat_sd_base+sector)*512), (size_t)count*512);

   // printf("read end \r\n");
    return RES_OK;
	}

}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	//DRESULT res;

	//printf("write...\n\r");
	if (pdrv != 0)
		return RES_PARERR;
	else
	{
		alt_sdmmc_write(&Card_Info,  (void*)((fat_sd_base+sector)*512), (void*)buff, (size_t)count*512);


    return RES_OK;
	}

}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	//printf("IOCTL!!");

	if (pdrv!=0 ) return RES_PARERR; else
	{
	res = RES_ERROR;
	switch (cmd) {
	case CTRL_SYNC :

		// Process of the command for the RAM drive

		res = RES_OK;
	break;

	case GET_SECTOR_SIZE :

		// Process of the command for the MMC/SD card
		*(DWORD*)buff = 512;
		res =  RES_OK;
		break;

	case GET_BLOCK_SIZE :

		// Process of the command the USB drive
		*(DWORD*)buff = 512;
		res =  RES_OK;
	    break;

	case CTRL_TRIM :

	    // Process of the command the USB drive
		res =  RES_OK;
	    break;
	}
	}


	return res;
}

DWORD get_fattime (void)
{
	int year = 2063;
	int month = 04;
	int day = 05;
	int hour = 20;
	int minute = 20;
	int second = 10;


//	bit31:25
//	Year origin from the 1980 (0..127, e.g. 37 for 2017)

//	bit24:21
//	Month (1..12)

//	bit20:16
//	Day of the month(1..31)

//	bit15:11
//	Hour (0..23)

//	bit10:5
//	Minute (0..59)

//	bit4:0
//	Second / 2 (0..29, e.g. 25 for 50)

	return  ( ((year-1980) & 0x7F) << 25 ) | ( (month & 0xF) << 21) | ( (day & 0x1F) << 16) | ( (hour & 0x1F ) << 11) | ( (minute & 0x3F) <<5) | (second & 0x1F );

//return 0;

}
