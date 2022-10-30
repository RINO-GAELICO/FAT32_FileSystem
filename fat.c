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
 * Description: The file that manages the File Allocation Table
 *
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fs_struct.h"
#include "fat.h"
#pragma pack(1)

uint32_t lastFreeCluster;
uint32_t totalFreeSpace;
int fatIndex;
int blockSizeFAT;

uint32_t * fatInit(uint32_t * fatBuffer, int numberOfBlocks, int blockSize)
{


	// fatSize = fatSize;
	// fatStart = fatStart;
	blockSizeFAT = blockSize;

	// if we represent instead the FAT as array
	// first 2 entries conventially left empty
	fatBuffer[0] = 0x0ffffff8;
	fatBuffer[1] = 0x0ffffff8;

	firstFreeCluster = 2;
	fatIndex = firstFreeCluster;

	// initilize free space
	// numberOfBlocks-338 because we don't represent the following blocks
	// VCB (1block) + reserved space (31 blocks) + 2 FAT maps (153*2) = 338
	for (int i = firstFreeCluster; i < (numberOfBlocks - 338); i++)
	{

		if (i == ((numberOfBlocks - 338))-1)
		{
	
			fatBuffer[i] = EOF_FLAG;
			lastFreeCluster = i;
		}
		else
		{
			fatIndex++;
			fatBuffer[i] = fatIndex;
		}
	}

	totalFreeSpace = numberOfBlocks - 338 - firstFreeCluster;


	// writing FatMap on Disk
	LBAwrite(fatBuffer, fatSize, fatStart);

	// writing back up FatMap
	LBAwrite(fatBuffer, fatSize, fatStart + fatSize);


	return fatBuffer;
}

// alloc returns the first entry of the chain allocation
int alloc(int fileLength)
{
	// check if there is enough free space
	if(totalFreeSpace<fileLength){
		printf("ALLOCATION UNSUCCESSFUL, NOT ENOUGH FREE SPACE\n");
		return -1;
	}
	

	// assigning the first FAT entry of free space to startAllocation
	int startAllocation = firstFreeCluster;

	// using a pointer to traverse the chain of entries
	int pointerFAT = firstFreeCluster;
	for (int i = 0; i < fileLength; i++)
	{
		if (i == fileLength - 1)
		{
			// the new first free sapce entry is the value pointed by the last entry of our traverse
			firstFreeCluster = fatBuffer[pointerFAT];
			// placing a EOF_FLAG flag, we reached the end of the chain
			fatBuffer[pointerFAT] = EOF_FLAG;
		}
		else
		{
			pointerFAT = fatBuffer[pointerFAT];
		}
	}

	totalFreeSpace -= fileLength;

	// writing FatMap on Disk after modification
	LBAwrite(fatBuffer, fatSize, fatStart);

	// writing back up FatMap
	LBAwrite(fatBuffer, fatSize, fatStart + fatSize);

	//writng on sdsik the pointer to first free space
	fsInfoptr->FSI_Nxt_Free = firstFreeCluster;
	LBAwrite(FSInfoBuffer,1,1);

	// returning the value corresponding to the starting point of chain
	return startAllocation;

}

int release(int startingBlock){
	
	int length=0;
	// fat already in memory 

	//in case only one block length
	if(fatBuffer[startingBlock]==EOF_FLAG){

		fatBuffer[startingBlock] = firstFreeCluster;
		length=1;
		
		
	}else{//more than one block

		int pointerBlock = startingBlock;
		int currentBlock;


		while(pointerBlock!=EOF_FLAG)
		{
			currentBlock = pointerBlock;
			pointerBlock = nextBlock(pointerBlock);
			length++;
		}
		fatBuffer[currentBlock] = firstFreeCluster;
	}


	firstFreeCluster = startingBlock;
	// writing FatMap on Disk after modification
	LBAwrite(fatBuffer, fatSize, fatStart);

	// writing back up FatMap
	LBAwrite(fatBuffer, fatSize, fatStart + fatSize);

	//writng on sdsik the pointer to first free space
	fsInfoptr->FSI_Nxt_Free = firstFreeCluster;
	LBAwrite(FSInfoBuffer,1,1);

	totalFreeSpace += length;

	return 0;

}

int nextBlock(int currentBlock){

	return fatBuffer[currentBlock];
}

int returnBlockSize (int startingBlocK){

	int totalBlock = 1;
	int pointerBlock = fatBuffer[startingBlocK];


	while(pointerBlock!=EOF){
		totalBlock+=1;
		pointerBlock = nextBlock(pointerBlock);

	}
	return totalBlock;
}

int calculateTotFreeSpace(int start){

	int counter = 0;

	while(fatBuffer[start]!=EOF_FLAG){

		start = nextBlock(start);
		counter++;

	}
	totalFreeSpace = counter;
	return counter;
}

