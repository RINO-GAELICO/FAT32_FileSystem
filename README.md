San Francisco State University
CSC 415-01 Operating System Principles - Professor Robert Bierman

Duccio Rocca (team leader)
Abhiram Rishi Prattipati
Christopher Solorzano
Shahriz Malek

GitHub: RINO-GAELICO
https://github.com/CSC415-2022-Spring/csc415-filesystem-RINO-GAELICO
April 27, 2022
 
Assignment - File System Design

The Volume Control Block on FAT32 file system is called Boot record. It is located in the reserved region and contains the data structure BPB (BIOS Parameter Block). The first sector of the volume is sometimes called the “boot sector”, or the “0th sector”. The BPB describes the physical layout of the data storage volume - it contains volume details such as the total number of blocks, number of free blocks, block size, free block pointers, pointer to the root directory, and type of volume. The Volume Control Block is essential in the File-System implementation.

VCB in C structure definition with details of each field:
struct BS_BPB {
char BS_jmpBoot[3]; // see docs for possible values
char BS_OEMName[8];//system formatting the volume (LINUX?)
short BPB_BytsPerSec;// It is 512 as per instructions
char BPB_SecPerClus;//BPB_BytsPerSec*BPB_SecPerClus not>32K
char BPB_RsvdSecCnt; //in FAT32 is typically 32.
char BPB_NumFATs; // # of FATs, probably = 2
short BPB_RootEntCnt; // for FAT32 this must be set to 0
short BPB_TotSec16; // for FAT32 this must be set to 0
char BPB_Media; // 0xF8 for fixed → low byte of FAT[0]
short BPB_FATSz16; // for FAT32 this must be set to 0
short BPB_SecPerTrk;// Sectors per track for interrupt 0x13
short BPB_NumHeads; // Number of heads for interrupt 0x13
int BPB_HiddSec; // = 0 on media that are not partitioned
int BPB_TotSec32; // total count of sectors on the volume
int BPB_FATSz32 ; //32bit count of blocks taken by ONE FAT
short BPB_ExtFlags ; // see tech sheet
short BPB_FSVer ; // version number of the FAT32 volume
int BPB_RootClus ;// 1st cluster of the root dr, usually 2
short BPB_FSInfo ; // Block number of FSINFO, usually 1
short BPB_BkBootSec ;//block # in reserved area of copy of //the boot record, usually 6
char BPB_Reserved[12];// Reserved for future expansion. File System 1
// When formatting FAT32 volume, we should set all of the bytes of this field to 0.
      char BS_DrvNum ;//This field is operating system specific
      char BS_Reserved1 ;//at formatting set this byte to 0
      char BS_BootSig ;//signature byte
      int BS_VolID ;//Volume serial number
      char BS_VolLab[11]; //Volume label. Match the Vlab in root
      char BS_FilSysType[8];// based on the FAT type: “FAT32   “
}


In a FAT system, the free space is tracked through the file-allocation table (FAT).This
table uses a variation of a linked allocation to keep track of the position of the files and also of the free space within the disk. To implement the FAT within the disk a region of storage has to be reserved to contain the table. The table has an entry of 32-bit (word) for each block and it is indexed by block number. Each block is pointing to the next to form a sort of linked list. The chain terminates with a special value that indicates the EOF.
Allocating a new block to a file is done by finding the first free block in the table and replacing the previous EOF value with the address of the new block.

In a FAT system, a directory is just a “file” composed of an array of 32-byte structures. These structures are called “directory entries”. There is only one special directory and it is the root directory. In a FAT32 system, the root directory can be of variable size and is represented by a cluster chain, just like any other directory. There is a specific field in the BPB called BPB_RootClus, that stores the location of the first cluster of the root directory. The root directory does not have a filename and there is no info on file date and file times. Also, the root directory does not contain the classical dot “.” and dot dot “..”, which are the first and second entries in every other directory.
In a FAT32 system, there are short directory and long directory entries. The latter are just regular (short) directory entries in which the attribute field has a value of ATTR_LONG_NAME, File System 2 
This indicates that the “file” is part of the long name entry for some other file. The long directory entries contain the long name of a file. The name contained in a short entry is called the alias name.

A long directory entry works as an extension to the short directory entries. A long entry cannot exist by itself. Long entries that are found without being coupled with a valid short entry, are called orphans.

Also, the long directory entries are located in close physical proximity to the short directory entries they are associated with. They are immediately contiguous to the short directory entries that are associated with. The reason why there are these two types is that long directory entries are invisible on old versions of MS-DOS/Windows.

We decided to describe, here below, both short and long directory entry structures. The short/regular directory entry is simply denominated as DIR_ENTRY, while the long one is denominated as LN_DIR_ENTRY.

struct DIR_ENTRY {
char DIR_Name[11]; // filename
char DIR_Attr; // see tech sheet for values
char DIR_NTRes; //reserved: set to 0 when a file is created short DIR_CrtTimeMs; // Millisecond stamp at file creation short DIR_CrtTime; // Time file was created
     short DIR_CrtDate; // Date file was created.
    short DIR_LstAccDate; // Last access date
    char DIR_FstClusHI[2];//High word this entry’s 1st cluster #
    short DIR_WrtTime; // last time modified
    short DIR_WrtDate; // date of last time modified
    char DIR_FstClusLO[2]; // Low word of 1st cluster #
    int DIR_FileSize; //holding this file’s size in bytes
}DIR_ENTRY;

struct LN_DIR_ENTRY {
char LDIR_Ord; // sequence # in the set of LN_DIR_ENTRYs short LDIR_Name1[5]; // utf 16-characters 1-5 of long-name char LDIR_Attr ; // must be ATTR_LONG_NAME
char LDIR_Type; //0x00 indicate sub-component of long name File System 3
char LDIR_Chksum;// 11 characters of the alias in DIR_ENTRY // are used
in the checksum calculation,
// see docs for algorithm
short LDIR_Name2[6]; // 6 more utf 16-characters of name
short LDIR_FstClusLO;  // Must be 0x0000
short LDIR_Name3[2]; // Characters 12-13 in utf-16
} LN_DIR_ENTRY;


This metadata is described in detail in the Directory Entry structure. It includes filename, file size, file dates and times (create, last access, modified), ACL, and location.
File System 4



File System Directory

After parse path was implemented, the other command functions were created such as loadDirectory, fs_mkdir, fs_rmvdir, fs_getcwd, fs_setcwd, fs_isFile, fs_isDir, fs_delete, fs_openDir, fs_diriteminfo, fs_stat. Parse path was an integral method that needed to be created in order for all the other functions to be created. loadDirectory was also a method that was needed for the other methods. Fs_mkdir needed the loadDirectory to be able to load the parent directory and to iterate through it to be able to find an unused Directory Entry to create a Directory Entry. Once the unused Directory Entry was found in the parent, space was allocated for the new directory and the parent array is filled in with the meta data in place for the new directory entry such as the last time modified, fate of last time modified, date file was created, etc. Then the new directory entry is written to disk using LBAwrite and a while loop. Then the parent directory is written to disk. Once the parent directory and the new directory entry is written to disk, then all the memory that was malloced then gets free for proper memory management. All the other methods use parse path heavily to be able to traverse through the Directories.

Key directory functions:

int fs_mkdir(const char *pathname, mode_t mode, struct DIR_ENTRY *directory){ (returns 1 = failed, 0 = success)

int fs_rmdir(const char *pathname)
(returns 1 = failed, 0 = success)

int fs_isFile(char * path, struct DIR_ENTRY *directory, int elementsCount)
(return 1 if file, 0 otherwise)

int fs_isDir(char * path) (return 1 if directory, 0 otherwise)

int fs_delete(char* filename) removes a file

Directory iteration functions

fdDir * fs_opendir(const char *name) 

int fs_closedir(fdDir *dirp)

Misc directory functions

char * fs_getcwd(char *buf, size_t size) Get the current directory and return the buffer 


int fs_setcwd(char *buf) 

int fs_isFile(char * path) Creating a directory entry holder

int fs_isDir(char * path)

struct fs_diriteminfo *fs_readdir(fdDir *dirp) Takes in a file descriptor for the directory entry

int fs_stat(const char *path, struct fs_stat *buf) Takes in a path through the parameters and then parses it based off of if it is a relative or absolute path


Milestone 3 Final - File Control Block
b_io_fd b_open (char * filename, int flags) 
In b_open function, the file system writes to a file if it’s in the disk. If it doesn’t exist, then the method will create flags related to files such as truncate, create, append, etc. These set the file position to the end of the file before starting to write on the end of the file.

int b_seek (b_io_fd fd, off_t offset, int whence) 
Checks to see if the file descriptor is valid or invalid
Checks to see if the offset is reater than or equal to the distance to the end of the buffer
Then checks to see if the buffer is dirty, if it is then it will flush out the buffer
If it is not dirty then we stay in the same buffer and add the offset to the index and set the
file descriptor file position to the whence plus the offset }

int b_write (b_io_fd fd, char * buffer, int count){
Checks to see if the file descriptor is valid or invalid
Checks to see if we need a new allocation for a brand new file
If the count is greater than the minimum block size then it will write to more than one
block
A block is written one at a time, the the relative fields get updated such as the relative file
block position, file position, and the size of the buffer then gets written on disk and the count gets decreased. Then the remaining bytes get loaded and written on disk. If we just need one block then one block gets allocated using are fille allocation free
space routine. If the file is already allocated but they need more space then we give it to them. We continue to write on top of the buffer until finished


int b_read (b_io_fd fd, char * buffer, int count){
The bytes to read and the bytes to give is set to 0
Checks to see if the file descriptor is valid or invalid
If statement to help filling from the existing buffer
If the count is less than the data that is available then we transferred only the count
If the count is greater than 0 we fill in multiples of block size
If we need more blocks if still over 512 then we give it to them until the remaining is less
than 512 bytes }

void b_close (b_io_fd fd){
Commit the file to disk if it is open for write, for read you dont
the access date, the file size, the mod date changes so need to update the directory
entry to the file
make sure the changes get committed to the directory entry
commitDE(ptrtoDir, indexDE);
free the file descriptor
free up the buffer
}

 
 Work Cited
“Microsoft Extensible Firmware Initiative FAT32 File System Specification.” Microsoft
Hardware White Paper, Version 1.03, Microsoft Corporation, 6 Dec. 2000, www.cs.fsu. edu/~cop4610t/assignments/project3/spec/fatspec.pdf
File System 46
