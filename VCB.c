#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fs_struct.h"
#include "VCB.h"


BS_BPB * initVCB(BS_BPB *bpbPtr, int numberOfBlocks){

    // jump instructions
		// This field has two allowed forms:
		/* jmpBoot[0] = 0xEB, jmpBoot[1] = 0x??, jmpBoot[2] = 0x90
		and
		jmpBoot[0] = 0xE9, jmpBoot[1] = 0x??, jmpBoot[2] = 0x??
		*/
		bpbPtr->BS_jmpBoot[0] = 0xEB;
		bpbPtr->BS_jmpBoot[1] = 0x58;
		bpbPtr->BS_jmpBoot[2] = 0x90;

		// OEM name: some systems use this string to decide if a volume is reliable
		memcpy(bpbPtr->BS_OEMName, "MSWIN4.1", 8);

		// bpbPtr
		bpbPtr->BPB_BytsPerSec = 0x0200; 	   // 512
		bpbPtr->BPB_SecPerClus = 0x01;		   // = 1
		bpbPtr->BPB_RsvdSecCnt = 0x0020;       // = 32
		bpbPtr->BPB_NumFATs = 0x02;			   // = 2 (one is for backup)
		bpbPtr->BPB_RootEntCnt = 0x0000;	   // For FAT32 volumes, this field must be set to 0.
		bpbPtr->BPB_TotSec16 = 0x0000;		   // For FAT32 volumes, this field must be set to 0.
		bpbPtr->BPB_Media = 0xF8;			   // 0xF8 is the standard value for “fixed” (non-removable) media
		bpbPtr->BPB_FATSz16 = 0x0000;		   // On FAT32 volumes this field must be 0
		bpbPtr->BPB_SecPerTrk = 0x0020;		   // = 32; This field is only relevant for media that have a geometry and are visible on interrupt 0x13.
		bpbPtr->BPB_NumHeads = 0x0040;		   // = 64; Number of heads for interrupt 0x13
		bpbPtr->BPB_HiddSec = 0x00000000;			   // This field should always be zero on media that are not partitioned.
		bpbPtr->BPB_TotSec32 = numberOfBlocks; //  = volumeSize / blockSize

		// from this field forward: FAT32 specific
		/*
		*	the value of BPB_FATSz32 is determined followed the instructions provided in the documentation
		*	the value found is expressed in blocks
		*	we should expect a nuber large enough to represent the whole volume
		*	considering that one 32 bit entry would be able to represent the position of a cluster
		* 	of ~16k bytes. Therefore in one block (512 bytes) we would be able to fit ~8MB, if our math is correct.
		*/
		uint32_t TmpVal1 = numberOfBlocks - (bpbPtr->BPB_RsvdSecCnt);
		uint32_t TmpVal2 = (256 * bpbPtr->BPB_SecPerClus) + bpbPtr->BPB_NumFATs;
		
		TmpVal2 = TmpVal2 / 2;
		bpbPtr->BPB_FATSz32 = (((TmpVal1 + (TmpVal2 - 1)) / TmpVal2))-2;
		
		bpbPtr->BPB_ExtFlags = 0x0000;
		bpbPtr->BPB_FSVer = 0x0000;
		bpbPtr->BPB_RootClus = 0x00000002; // root location
		bpbPtr->BPB_FSInfo = 0x0001;
		bpbPtr->BPB_BkBootSec = 0x0006;
		memset( bpbPtr->BPB_Reserved, 0x00, 12 );
		bpbPtr->BS_DrvNum = 0x80;
		bpbPtr->BS_Reserved1 = 0x00;
		bpbPtr->BS_BootSig = 0x29;
		
		uint32_t seconds;
    	seconds = time(NULL);
		bpbPtr->BS_VolID = seconds;
		// printf("This is the second value %u",seconds);

		memset(bpbPtr->BS_VolLab, 0x20, 11);
		memcpy(bpbPtr->BS_FilSysType, "FAT32   ", 8);



    return bpbPtr;

}