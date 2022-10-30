/**************************************************************
* Class:  CSC-415-01  Spring 2022
* Names: Shahriz Malek, Duccio Rocca, Abhiram Rishi Prattipati, Christopher Solorzano
* Student IDs: 920378989, 922254031, 921346982, 920528216 
* GitHub Name: RINO-GAELICO
* Group Name: Sunset
* Project: Basic File System
*
* File: fs_struct.h
*
* Description: Interface of basic I/O functions
*
**************************************************************/
#ifndef _FS_STRUCT_H
#define _FS_STRUCT_H
#pragma pack(1)
#define MAX_DEPTH_PATH 10240

extern uint32_t firstFreeCluster;
extern u_int8_t *buffer;
extern u_int8_t *FSInfoBuffer;
extern uint32_t *fatBuffer;
extern uint32_t firstFreeCluster;
extern uint32_t fatStart;
extern uint32_t fatSize;
extern uint32_t dataStart;
extern uint32_t rootStart;
extern uint32_t rootCluster;
extern uint32_t rootSize;
extern char * currentDir;

typedef struct {

	int FSI_LeadSig;
	u_int8_t FSI_Reserved1[480];
	int FSI_StrucSig;
	int FSI_Free_Count;
	int FSI_Nxt_Free;
	int FSI_Reserved2[3];
	int FSI_TrailSig;



}FS_INFO;


typedef struct {
	u_int8_t BS_jmpBoot[3]; // see docs for possible values
    u_int8_t BS_OEMName[8]; //system formatting the volume (LINUX?)
    u_int16_t BPB_BytsPerSec;// It is 512 as per instructions	
    u_int8_t BPB_SecPerClus;//BPB_BytsPerSec*BPB_SecPerClus not>32K
	u_int16_t BPB_RsvdSecCnt; //in FAT32 is typically 32.	
    u_int8_t BPB_NumFATs; // # of FATs, probably = 2
	u_int16_t BPB_RootEntCnt; // for FAT32 this must be set to 0
	u_int16_t BPB_TotSec16; // for FAT32 this must be set to 0
	u_int8_t BPB_Media; // 0xF8 for fixed → low byte of FAT[0]
	u_int16_t BPB_FATSz16; // for FAT32 this must be set to 0
    u_int16_t BPB_SecPerTrk;// Sectors per track for interrupt 0x13
	u_int16_t BPB_NumHeads; // Number of heads for interrupt 0x13
	uint32_t BPB_HiddSec; // = 0 on media that are not partitioned
	uint32_t BPB_TotSec32; // total count of sectors on the volume
	uint32_t BPB_FATSz32 ; //32bit count of blocks taken by ONE FAT
	u_int16_t BPB_ExtFlags ; // see tech sheet
	u_int16_t BPB_FSVer ; // version number of the FAT32 volume
	uint32_t BPB_RootClus ;// 1st cluster of the root dr, usually 2
	u_int16_t BPB_FSInfo ; // Block number of FSINFO, usually 1
    u_int16_t BPB_BkBootSec ;//block # in reserved area of copy of 	//the boot record, usually 6
	u_int8_t BPB_Reserved[12];// Reserved for future expansion. //When formatting FAT32 volume we should set all of the bytes of //this field to 0.
	u_int8_t BS_DrvNum ;//This field is operating system specific
	u_int8_t BS_Reserved1 ;//at formatting set this byte to 0
	u_int8_t BS_BootSig ;//signature byte
	uint32_t BS_VolID ;//Volume serial number
	u_int8_t BS_VolLab[11]; //Volume label. Match the Vlab in root
	u_int8_t BS_FilSysType[8];// based on the FAT type: “FAT32   “
}BS_BPB;

typedef struct DIR_ENTRY {
    char DIR_Name[11]; // filename
    char DIR_Attr; // see tech sheet for values
    char DIR_NTRes; //reserved: set to 0 when a file is created
    char DIR_CrtTimeMs; // Millisecond stamp at file creation
    short DIR_CrtTime; // Time file was created
    short DIR_CrtDate; // Date file was created.
    short DIR_LstAccDate; // Last access date
    char DIR_FstClusHI[2]; //High word this entry’s 1st cluster #
    short DIR_WrtTime; // last time modified
    short DIR_WrtDate; // date of last time modified
    char DIR_FstClusLO[2]; // Low word of 1st cluster #
    int DIR_FileSize; //holding this file’s size in bytes
}DIR_ENTRY;

typedef struct LN_DIR_ENTRY {
	char LDIR_Ord; // sequence # in the set of LN_DIR_ENTRYs
    short LDIR_Name1[5]; // utf 16-characters 1-5 of long-name
	char LDIR_Attr ;     // must be ATTR_LONG_NAME
	char LDIR_Type; //0x00 indicate sub-component of long name
    char LDIR_Chksum;// 11 characters of the alias in DIR_ENTRY // are used in the checksum calculation, 
	// see docs for algorithm
	short LDIR_Name2[6]; // 6 more utf 16-characters of name
    short LDIR_FstClusLO;  // Must be 0x0000
	short LDIR_Name3[2]; // Characters 12-13 in utf-16
} LN_DIR_ENTRY;

extern BS_BPB *bpbPtr;
extern DIR_ENTRY *rootDirectory;
extern FS_INFO *fsInfoptr;


#endif