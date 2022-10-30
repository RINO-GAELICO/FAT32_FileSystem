/**************************************************************
 * Class:  CSC-415-01 Spring 2022
 * Names: Shahriz Malek, Duccio Rocca, Abhiram Rishi Prattipati, Christopher Solorzano
 * Student IDs: 920378989, 922254031, 921346982, 920528216
 * GitHub Name: RINO-GAELICO
 * Group Name: Sunset
 * Project: Basic File System
 *
 * File: fat.h
 *
 * Description: Headers for fat.c
 *
 **************************************************************/
extern uint32_t *fatBuffer;
extern uint32_t lastFreeCluster;
extern uint32_t totalFreeSpace;
#define EOF_FLAG 0xffffffff

uint32_t * fatInit (uint32_t * fatBuffer, int numberOfBlocks, int blockSize);

int alloc(int fileLength);

int release(int startingBlock);

int nextBlock(int currentBlock);

int returnBlockSize (int startingBlocK);

int calculateTotFreeSpace(int start);