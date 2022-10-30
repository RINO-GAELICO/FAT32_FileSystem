/**************************************************************
 * Class:  CSC-415-01 Spring 2022
 * Names: Shahriz Malek, Duccio Rocca, Abhiram Rishi Prattipati, Christopher Solorzano
 * Student IDs: 920378989, 922254031, 921346982, 920528216
 * GitHub Name: RINO-GAELICO
 * Group Name: Sunset
 * Project: Basic File System
 *
 * File: directory.h
 *
 * Description: Headers for directory.c
 *
 **************************************************************/

#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "fsLow.h"
#include "fat.h"
#include "fs_struct.h"
#include "mfs.h"

#define DIR_ENTRY_UNUSED 0xE5
#define IS_DIR 0x10
#define ATTR_READ_ONLY 0x01 
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_ARCHIVE 0x20
#define IS_DIR_BIT 5


typedef struct 
{
    int indexDE;
    char * lastComponent;
    char * currentComponent;
    int isDir; // 1 YES , 0 NO;
    DIR_ENTRY DE_element;
    DIR_ENTRY parentDE;

}DE_holder;

extern DE_holder *deHolderRet;
extern fdDir * fdPtr;

int checkDirBit(char attributeValue);

DE_holder *parsePath (char * path, DE_holder *DEholder);

DIR_ENTRY* initDirectory(struct DIR_ENTRY directory[], DIR_ENTRY parentDE, int elementsCount, int locationDir);

DIR_ENTRY* initDirectoryRoot(struct DIR_ENTRY directory[], int elementsCount, int locationDir);

DIR_ENTRY* loadDirectory (int fileSize, int firstLocation, DIR_ENTRY * dirBuffer);

#endif