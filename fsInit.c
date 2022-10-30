/**************************************************************
 * Class:  CSC-415-01 Spring 2022
 * Names: Shahriz Malek, Duccio Rocca, Abhiram Rishi Prattipati, Christopher Solorzano
 * Student IDs: 920378989, 922254031, 921346982, 920528216  
 * GitHub Name: RINO-GAELICO
 * Group Name: Sunset
 * Project: Basic File System
 *
 * File: fsInit.c
 *
 * Description: Main driver for file system assignment.
 *
 * This file is where you will start and initialize your system
 *
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fsLow.h"
#include "fat.h"
#include "fs_struct.h"
#include "directory.h"
#include "mfs.h"
#include "VCB.h"
#pragma pack(1)

u_int8_t *buffer;
u_int8_t *FSInfoBuffer;
BS_BPB * bpbPtr;
FS_INFO *fsInfoptr;
DIR_ENTRY *rootDirectory;
uint32_t firstFreeCluster;
uint32_t fatStart;
uint32_t fatSize;
uint32_t dataStart;
uint32_t rootStart;
uint32_t rootCluster;
uint32_t rootSize;
uint32_t *fatBuffer;
DE_holder* deHolderRet;
char* currentDir;

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %lu blocks with a block size of %lu \n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	uint32_t numberOfBlocks32 = numberOfBlocks & 0xFFFFFFFF;
	uint32_t blockSize32 = blockSize & 0xFFFFFFFF;

	uint64_t DiskSize = numberOfBlocks * blockSize;
	
	//preparing the buffer for VCB
	buffer = malloc(blockSize);

	// prepering the buffer for FSINFO
	FSInfoBuffer = malloc(blockSize);
	memset(FSInfoBuffer, 0x00, 0x200);


	// getting block 0 and 1 from disk
	LBAread(FSInfoBuffer, 1 , 1);
	fsInfoptr = (FS_INFO *) FSInfoBuffer;
	LBAread(buffer, 1, 0);

	// to determine if we need to format the volume or not
	// we check the magic number
	if ((buffer[0x1fe]) != (0x55) || (buffer[0x1ff]) != (0xAA))
	{
	 	// not formatted yet

	 	// casting the buffer to read some information from the block 0
		bpbPtr = (BS_BPB *) buffer;

		bpbPtr = initVCB(bpbPtr, numberOfBlocks32);
		
		// ending signatures
		/*
		* as per instructions from documentation: 
		* "If we consider the contents of the sector as a byte array, 
		*  it must be true that sector[510] equals 0x55, and sector[511] equals 0xAA."
		*/
		buffer[0x1fe] = 0x55;
		buffer[0x1ff] = 0xAA;
		LBAwrite(buffer, 1, 0);
		// write backup copy of VCB
		LBAwrite(buffer, 1, 6);

		fatStart = bpbPtr->BPB_RsvdSecCnt;
		fatSize = bpbPtr->BPB_FATSz32;
		dataStart = fatStart + fatSize * (bpbPtr->BPB_NumFATs);
		rootStart = dataStart + (bpbPtr->BPB_SecPerClus * (bpbPtr->BPB_RootClus-1));
		rootCluster = bpbPtr->BPB_RootClus;

		printf("FATSIZE IN BLOCKS: %d\n",fatSize);
		printf("FAT START: %d\n", fatStart);
		printf("CLUSTER IN BLOCKS: %d\n",bpbPtr->BPB_SecPerClus);
		printf("DATA START: %d\n", dataStart);
		printf("Number of Blocks in Volume: %d\n",numberOfBlocks32);
		printf("BLOCK SIZE, %d\n", blockSize32);

		// Initializing the FAT map

		// malloc for fat
		fatBuffer = malloc(blockSize * fatSize);
		memset(fatBuffer, 0x00, blockSize * fatSize);


	
		fatBuffer = fatInit(fatBuffer, numberOfBlocks32, blockSize32);

		// end of initialization of FAT map

		printf("Last free cluster: %d\n", lastFreeCluster);

		
		// where the fat starts? at the end of the reserved sectors

		// init root directory
		
		// 64 DEs would occupy exactly 4 blocks 
		// each DE is 32 bytes -> 32*64 = 2048 / blocksize = 4

		rootSize = 4;

		rootDirectory = malloc(sizeof(DIR_ENTRY)*64);

		rootStart = alloc(rootSize);

		rootDirectory = initDirectoryRoot(rootDirectory, 64, rootStart);

		int rootLocation = *(rootDirectory[0].DIR_FstClusHI) << 16 | *(rootDirectory[0].DIR_FstClusLO) ;

		// actually writing root on disk
		LBAwrite(rootDirectory,rootSize, dataStart+rootStart);

		printf("Root start: %d\n", rootStart);

		// FSInfo sector 
		// in block 1 of Disk as definied in bpbPtr->BPB_FSInfo

		fsInfoptr->FSI_LeadSig = 0x41615252; // This is lead signature 
										   // here in the middle we leave 480 bytes of zero as reserved area
		fsInfoptr->FSI_StrucSig = 0x61417272; // Another signature 
		fsInfoptr->FSI_Free_Count = lastFreeCluster;// last known free cluster count on the volume. 
												   // If the value is 0xFFFFFFFF, then the free count is unknown and must be computed
		fsInfoptr->FSI_Nxt_Free =  firstFreeCluster; // It indicates the cluster number at which the driver 
												   // should start looking for free clusters,  If the value is 0xFFFFFFFF, then there is no hint 
												   // and the driver should start looking at cluster 2.
		fsInfoptr->FSI_TrailSig = 0xaa550000; // this has the 0xAA55 signature in sector offsets 510 and 511, 
												   // just like the first boot sector does 
		

		//writing on disk
		LBAwrite(FSInfoBuffer,1,1);
		
		
	}
	else
	{ 
	// already formatted
	

	// cast the VCB buffer to the right struct
	bpbPtr = (BS_BPB *) buffer;

	// filling some global variables
	fatStart = bpbPtr->BPB_RsvdSecCnt;
	fatSize = bpbPtr->BPB_FATSz32;
	dataStart = fatStart + fatSize * (bpbPtr->BPB_NumFATs);
	rootStart = dataStart + (bpbPtr->BPB_SecPerClus * (bpbPtr->BPB_RootClus-1));
	rootCluster = bpbPtr->BPB_RootClus;
	firstFreeCluster = fsInfoptr->FSI_Nxt_Free;


	//load the fat in memory
	// malloc for fat
	fatBuffer = malloc(blockSize * fatSize);
	LBAread(fatBuffer, fatSize, fatStart);

	rootSize = returnBlockSize(rootStart);
	

	}
	// once fat is loaded let's calculate free space
	calculateTotFreeSpace(firstFreeCluster);

	// set currentDir to root
	currentDir = malloc(MAX_DEPTH_PATH);
	strcpy( currentDir, "/");


	printf("FATSIZE IN BLOCKS: %d\n",fatSize);
	printf("FAT START: %d\n", fatStart);
	printf("CLUSTER IN BLOCKS: %d\n",bpbPtr->BPB_SecPerClus);
	printf("DATA START: %d\n", dataStart);
	printf("Number of Blocks in Volume: %d\n",numberOfBlocks32);
	printf("BLOCK SIZE, %d\n", blockSize32);



	return 0;
}

void exitFileSystem()
{

	free(rootDirectory);
	free(currentDir);

	if(deHolderRet!=NULL){

		free(deHolderRet->currentComponent);
		free(deHolderRet->lastComponent);
		free(deHolderRet);
	}
	
	free(fatBuffer);
	free(buffer);
	free(FSInfoBuffer);
	printf("System exiting\n");
}
