/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: Shahriz Malek, Duccio Rocca, Abhiram Rishi Prattipati, Christopher Solorzano
* Student IDs: 920378989, 922254031, 921346982, 920528216
* GitHub Name: RINO-GAELICO
* Group Name: Sunset
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "fsLow.h"
#include "directory.h"
#include "fat.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512



typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int filePosition; // where we are into the file bytes
	int buflen;		//holds how many valid bytes are in the buffer
	DIR_ENTRY * DEptr;	// pointer to file's DE
	DIR_ENTRY * parentDir;
	int size;		// size of the file	
	int isBufferDirty; // is buffer written to disk
	int relativeFileBlockPs; // which block of the file we are at
	int indexDE;	// the index of the DE innto the parent directory
	int newFile;	// flag that tells us if this file needs a new allocation
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int putBlockFile(b_io_fd fd); //write on disk one block
int getBlockFile(b_io_fd fd); // read from disk one block
int commitDE(b_io_fd fd); // commit DE 


int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;

			
	if (startup == 0) b_init();  //Initialize our system				
							
	returnFd = b_getFCB(); // get our own file descriptor

	if(returnFd ==(-1)) // check for error - all used FCB's
	{
		return (-1);
	}


	fcbArray[returnFd].buf = malloc(sizeof(char)*MINBLOCKSIZE);
	if(fcbArray[returnFd].buf==NULL){
		printf("ERROR MALLOC\n");
		return   (-1);
	}

	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].DEptr = malloc(sizeof(DIR_ENTRY));
	

	char * currentPath;
    char* pathnameVar;
    pathnameVar = malloc(strlen(filename)+2);
    currentPath = malloc(MAX_DEPTH_PATH);

    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
    DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);

	// some variables usefule for time and date variables

	time_t rawtime;
	struct tm *tmInfo;
	tmInfo = malloc(sizeof(time_t));
	time ( &rawtime );
	tmInfo = localtime ( &rawtime );
	u_int16_t fatDate= tmInfo->tm_year-80 << 9 | tmInfo->tm_mon+1 << 5 | tmInfo->tm_mday >> 1 ;
	u_int16_t fatTime = tmInfo->tm_hour << 11 | tmInfo->tm_min << 5 | tmInfo->tm_sec >> 1 ;
	u_int8_t seconds;
	seconds = time(NULL);
    
    if(filename[0]!='/'){ // relative path

        strcpy(currentPath,currentDir);
        strcat(currentPath,"/");
        strcat(currentPath,filename);
        DEholder = parsePath(currentPath, DEholder);

    }else{ // absolute path
        
        strcpy(pathnameVar,filename);
        DEholder = parsePath(pathnameVar, DEholder);
    }

	

    
    // check  if it is valid, if it exists and if it is a file
    if(DEholder!=NULL && DEholder->indexDE>=0 && DEholder->isDir==0){ // valid path and exists and is not a dir

		
		int parentSize = DEholder->parentDE.DIR_FileSize;
		fcbArray[returnFd].parentDir = malloc(parentSize);

		
		
        int parentLocation = *(DEholder->parentDE.DIR_FstClusHI) << 16 | *(DEholder->parentDE.DIR_FstClusLO);
        
        fcbArray[returnFd].parentDir = loadDirectory(parentSize, parentLocation, fcbArray[returnFd].parentDir);
		fcbArray[returnFd].indexDE = DEholder->indexDE;

		*(fcbArray[returnFd].DEptr) = (DEholder->DE_element);

		if(flags & O_TRUNC && (flags & O_RDONLY) != O_RDONLY){ // truncate whatever is in there

			fcbArray[returnFd].DEptr->DIR_FileSize = 0;
			fcbArray[returnFd].size = 0;
			fcbArray[returnFd].filePosition = 0;
			fcbArray[returnFd].relativeFileBlockPs = fcbArray[returnFd].filePosition/MINBLOCKSIZE;
			fcbArray[returnFd].isBufferDirty = 0;

			

		}else if((flags & O_APPEND) == O_APPEND){ // not truncate so append an exisiting file

			fcbArray[returnFd].filePosition = fcbArray[returnFd].DEptr->DIR_FileSize;
			fcbArray[returnFd].size = fcbArray[returnFd].DEptr->DIR_FileSize; 
			fcbArray[returnFd].relativeFileBlockPs = fcbArray[returnFd].filePosition/MINBLOCKSIZE;
			fcbArray[returnFd].isBufferDirty = 0;

			

		}else { // not truncate not append so just open at the beginning


			
			fcbArray[returnFd].size = fcbArray[returnFd].DEptr->DIR_FileSize;
			fcbArray[returnFd].filePosition = 0;
			fcbArray[returnFd].relativeFileBlockPs = fcbArray[returnFd].filePosition/MINBLOCKSIZE;
			if((flags & O_RDONLY) == O_RDONLY){ // read only so it doesn't need to be comitted

				fcbArray[returnFd].isBufferDirty = 1;

			}else{ // not read only so we might need to commit the buffer

				fcbArray[returnFd].isBufferDirty = 0;
			}

			

		}

		fcbArray[returnFd].newFile = 1;

		free(pathnameVar);
		free(currentPath);

		free(DEholder->currentComponent);
		free(DEholder->lastComponent);
		free(DEholder);

		return returnFd;

		
	}

	if(DEholder!=NULL && DEholder->indexDE<0){ // valid path but file does not exist

		

		int parentSize = DEholder->parentDE.DIR_FileSize;
		
		fcbArray[returnFd].parentDir = malloc(parentSize);
		//STEP TWO: return DE or a pointer to the last element
        int parentLocation = *(DEholder->parentDE.DIR_FstClusHI) << 16 | *(DEholder->parentDE.DIR_FstClusLO);
        
        fcbArray[returnFd].parentDir = loadDirectory(parentSize, parentLocation, fcbArray[returnFd].parentDir);
		
		int positionIndex=-1;

        // find unused DE in parent
        for(int i=0 ; i< (fcbArray[returnFd].parentDir[0].DIR_FileSize/sizeof(DIR_ENTRY)) ; i++){

       
            if(fcbArray[returnFd].parentDir[i].DIR_Name[0] == 0x00){

                positionIndex = i;
                break;
            }
            
        }




		if(positionIndex>(-1)){
			fcbArray[returnFd].indexDE = positionIndex;
		}else{
			printf("LOOKS  PARENT DIRECTORY IS FULL\n");
			return (-1);
		}
		
		// && (flags & O_RDONLY) != O_RDONLY

		if((flags & (O_CREAT)) == O_CREAT ){ // brand new file

			

			fcbArray[returnFd].DEptr = malloc(sizeof(DIR_ENTRY));
			if(fcbArray[returnFd].DEptr==NULL){
				printf("ERROR MALLOC\n");
				return (-1);
			}
			fcbArray[returnFd].DEptr->DIR_FileSize = 0;
			fcbArray[returnFd].size = fcbArray[returnFd].DEptr->DIR_FileSize;
			fcbArray[returnFd].filePosition = 0;
			fcbArray[returnFd].relativeFileBlockPs = fcbArray[returnFd].filePosition/MINBLOCKSIZE;
			fcbArray[returnFd].isBufferDirty = 0;

			strcpy(fcbArray[returnFd].DEptr->DIR_Name,DEholder->currentComponent);
			

			fcbArray[returnFd].DEptr->DIR_Attr = 0x00;
            fcbArray[returnFd].DEptr->DIR_WrtTime = fatTime; // last time modified
            fcbArray[returnFd].DEptr->DIR_WrtDate = fatDate; // date of last time modified
            
            fcbArray[returnFd].DEptr->DIR_CrtDate = fatDate; // Date file was created. OPTIONAL AS PER SPECS
            fcbArray[returnFd].DEptr->DIR_CrtTime = fatTime; // Time file was created OPTIONAL AS PER SPECS
            fcbArray[returnFd].DEptr->DIR_CrtTimeMs = seconds; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
            
            fcbArray[returnFd].DEptr->DIR_LstAccDate = fatDate; // Last access date OPTIONAL AS PER SPECS

			fcbArray[returnFd].newFile = 0;


			
			free(pathnameVar);
			free(currentPath);

		

			free(DEholder->currentComponent);
			free(DEholder->lastComponent);
			free(DEholder);
			

			return returnFd;

		}else{

			

			free(DEholder->currentComponent);
			free(DEholder->lastComponent);
			
			
			free(DEholder);
			free(currentPath);
			free(pathnameVar);
			
			return (-1);
		}

		
	}



	if(DEholder!=NULL){

        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
        
    }
	free(DEholder);
    free(currentPath);
    free(pathnameVar);
    
    return (-1);


	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	int distanceEndBuffer = abs( MINBLOCKSIZE - fcbArray[fd].index );

	if(offset >= distanceEndBuffer){

		if(fcbArray[fd].isBufferDirty==0){ // we need to flush the buffer

			putBlockFile(fd);
			
			fcbArray[fd].isBufferDirty = 1;
			fcbArray[fd].index = 0;
			fcbArray[fd].filePosition = whence+offset;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
			fcbArray[fd].buflen = 0;

			// now we are ready to read the block into the buffer
			getBlockFile(fd);

			return 0;

		}
		
	}else{ // we stay in the same buffer

		fcbArray[fd].index += offset;
		fcbArray[fd].filePosition = whence+offset;

		return 0;
		
	}
	
	
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 		//invalid file descriptor
		}
		
	int bytesWritten = 0;

	if(count<0){
		printf("NOTHING TO WRITE\n");
		return (-1);
	}

	
	if(fcbArray[fd].newFile==0){ //  we need new allocation for a brand new file

		int fileAllocation;

		

		if(count>MINBLOCKSIZE){ // we need to write more than one block

			fileAllocation = alloc((count/MINBLOCKSIZE)+1);

			uint32_t location = fileAllocation;
			u_int16_t lower_word = (u_int16_t) (location & 0xFFFFUL);
			u_int16_t upper_word = (u_int16_t) ((location >> 16) & 0xFFFFUL);

			memset(fcbArray[fd].DEptr->DIR_FstClusLO, lower_word, 2);
			memset(fcbArray[fd].DEptr->DIR_FstClusHI, upper_word, 2);


			while((count/MINBLOCKSIZE) > 0){ // we write one block at the time

				// we load the buffer and update the relative fields
				memcpy(fcbArray[fd].buf,buffer, MINBLOCKSIZE);
				bytesWritten += MINBLOCKSIZE;
				
				fcbArray[fd].filePosition += MINBLOCKSIZE;
				
				fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
				fcbArray[fd].size += MINBLOCKSIZE;

				// write the buffer on disk
				putBlockFile(fd);
				
				count -= MINBLOCKSIZE;

			}



			// we need to load the remaining bytes which is < 512 and write it on disk
			
			// we load the buffer and update the relative fields
			memcpy(fcbArray[fd].buf,buffer, count);
			bytesWritten += count;
				
			fcbArray[fd].filePosition += count;
			
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
			fcbArray[fd].size += count;

			fcbArray[fd].buflen += count;
			fcbArray[fd].index += count;

			// write the buffer on disk
			putBlockFile(fd);



			fcbArray[fd].isBufferDirty = 1;
			fcbArray[fd].newFile = 1;

			//done

		

		}else{ // we just need one block

			

			fileAllocation = alloc(1);

			uint32_t location = fileAllocation;
			
			u_int16_t lower_word = (u_int16_t) (location & 0xFFFFUL);
			u_int16_t upper_word = (u_int16_t) ((location >> 16) & 0xFFFFUL);

			memset(fcbArray[fd].DEptr->DIR_FstClusLO, lower_word, 2);
			memset(fcbArray[fd].DEptr->DIR_FstClusHI, upper_word, 2);

			int locationFileTest = *(fcbArray[fd].DEptr->DIR_FstClusHI) << 16 | *(fcbArray[fd].DEptr->DIR_FstClusLO);
			
			
			// we load the buffer and update the relative fields
			// this is a brand new file so we start from zero
			
			
			memcpy(fcbArray[fd].buf, buffer, count);
		

			bytesWritten += count;
			fcbArray[fd].buflen = count;
			fcbArray[fd].filePosition += count;
			fcbArray[fd].index = count;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
			fcbArray[fd].size += count;

			// write the buffer on disk
			putBlockFile(fd);
			fcbArray[fd].isBufferDirty = 1;
			fcbArray[fd].newFile = 1;

			//done



		}

		return bytesWritten;

	

	}else{ // already allocated somewhere we just neeed to allocate more if we need more space

		int blocksNow = (fcbArray[fd].size / MINBLOCKSIZE)+1;
		int blocksAfter = ((fcbArray[fd].size + count) / MINBLOCKSIZE)+1;
		int locationFile = *(fcbArray[fd].DEptr->DIR_FstClusHI) << 16 | *(fcbArray[fd].DEptr->DIR_FstClusLO);;
		
		

		

		if(blocksAfter > blocksNow){ // we need to reallocate then

			// we release and reallocate now
			release(locationFile);
			locationFile = alloc(((fcbArray[fd].size+count)/MINBLOCKSIZE)+1);
			

		}

		uint32_t location = locationFile;
		u_int16_t lower_word = (u_int16_t) (location & 0xFFFFUL);
		u_int16_t upper_word = (u_int16_t) ((location >> 16) & 0xFFFFUL);

		memset(fcbArray[fd].DEptr->DIR_FstClusLO, lower_word, 2);
		memset(fcbArray[fd].DEptr->DIR_FstClusHI, upper_word, 2);

	

		

		int spaceLeftBuffer = MINBLOCKSIZE - fcbArray[fd].buflen;

		
		

		if( spaceLeftBuffer > 0 && count <= spaceLeftBuffer){ // we just write on top of the buffer and we are done

		// the count here is smaller than the remaining space in the buffer

		

			char * tempPtr;
			
			tempPtr = fcbArray[fd].buf;

			tempPtr = tempPtr + fcbArray[fd].buflen;


			memcpy(tempPtr, buffer, count);
			
			
			bytesWritten += count;
			fcbArray[fd].buflen += count;
			fcbArray[fd].filePosition += count;
			fcbArray[fd].index += count;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
			fcbArray[fd].size += count;

			putBlockFile(fd);
			fcbArray[fd].isBufferDirty = 1;

			



		}else if(spaceLeftBuffer > 0 && count > spaceLeftBuffer){ // we write on top of the buffer and we keep going until we are done

			char * tempPtr = fcbArray[fd].buf;
			char* resetTheir = buffer;

			

			tempPtr += fcbArray[fd].buflen;

			

			memcpy(tempPtr, buffer, spaceLeftBuffer);
			resetTheir += spaceLeftBuffer;

			
			
			bytesWritten += spaceLeftBuffer;
			fcbArray[fd].buflen += spaceLeftBuffer;
			fcbArray[fd].filePosition += spaceLeftBuffer;
			fcbArray[fd].index += spaceLeftBuffer;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
			fcbArray[fd].size += spaceLeftBuffer;
			count -= spaceLeftBuffer;

			

			putBlockFile(fd);
			fcbArray[fd].isBufferDirty = 1;

			while((count/MINBLOCKSIZE)>0){

				fcbArray[fd].buflen = 0;
				fcbArray[fd].index = 0;

				

				memcpy(fcbArray[fd].buf,resetTheir, MINBLOCKSIZE);
				resetTheir+=MINBLOCKSIZE;
				
				bytesWritten += MINBLOCKSIZE;
				
				fcbArray[fd].filePosition += MINBLOCKSIZE;
				
				fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
				fcbArray[fd].size += MINBLOCKSIZE;
				count -= MINBLOCKSIZE;

				putBlockFile(fd);
				fcbArray[fd].isBufferDirty = 1;
				
				


			}

			// now what is left to be written is less than 512 bytes
			if(count>0){

				
				memcpy(fcbArray[fd].buf,resetTheir, count);
				resetTheir+=count;
				bytesWritten += count;
				fcbArray[fd].buflen = count;
				fcbArray[fd].filePosition += count;
				fcbArray[fd].index = count;
				fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
				fcbArray[fd].size += count;


				putBlockFile(fd);
			}

			

			




		}else if(count < MINBLOCKSIZE){ // there is no more space in the buffer the count is small enough to do it in one session

			// let's flush the buffer and reload it once
			

			
			memcpy(fcbArray[fd].buf,buffer, count);
			
			bytesWritten += count;
			fcbArray[fd].buflen = count;
			fcbArray[fd].filePosition += count;
			fcbArray[fd].index = count;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
			fcbArray[fd].size += count;

			putBlockFile(fd);
			fcbArray[fd].isBufferDirty = 1;
			


		}else if(count > MINBLOCKSIZE){ // there is no more space in buffer but count is big som we need more sessions

			char* resetTheir = buffer;
			

			while((count/MINBLOCKSIZE)>0){

				

				memcpy(fcbArray[fd].buf,resetTheir, MINBLOCKSIZE);
				resetTheir+=MINBLOCKSIZE;
			
				bytesWritten += MINBLOCKSIZE;
				
				fcbArray[fd].filePosition += MINBLOCKSIZE;
				
				fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
				fcbArray[fd].size += MINBLOCKSIZE;
				count -= MINBLOCKSIZE;

				putBlockFile(fd);
				fcbArray[fd].isBufferDirty = 1;

				

				
			}

			// now what is left to be written is less than 512 bytes

			if(count>0){
				memcpy(fcbArray[fd].buf,resetTheir, count);
				resetTheir+=count;
				bytesWritten += count;
				fcbArray[fd].buflen = count;
				fcbArray[fd].filePosition += count;
				fcbArray[fd].index = count;
				fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition)-1)/MINBLOCKSIZE;
				fcbArray[fd].size += count;
				
			}
			
			

			putBlockFile(fd);


			
		}

		

		return bytesWritten;


	}


	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	int bytesReadTotal = 0;
	int bytesToGive = 0;
	char * resetBuffer = buffer;
	char * resetOur = (fcbArray[fd].buf)+fcbArray[fd].index;
	// char * resetOur = (fcbArray[fd].buf);

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	// part one fill from existing buffer
	if(fcbArray[fd].buflen>0 && fcbArray[fd].index<fcbArray[fd].buflen){

		

	
		bytesToGive = fcbArray[fd].buflen-fcbArray[fd].index;

		if(count>=bytesToGive){ // we need to continue after this step

			memcpy(buffer,resetOur,bytesToGive);
			resetBuffer+=bytesToGive;
			resetOur+=bytesToGive;
			bytesReadTotal += bytesToGive;
			fcbArray[fd].index += bytesToGive;
			fcbArray[fd].filePosition += bytesToGive;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition))/MINBLOCKSIZE;
			count = count - bytesToGive;

			
			


		}else{ // count is less than the data available so let's transfer only count and we are done

			memcpy(buffer,resetOur,count);
			resetBuffer+=count;
			resetOur+=count;
			bytesReadTotal += count;
			fcbArray[fd].index += count;
			fcbArray[fd].filePosition += count;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition))/MINBLOCKSIZE;
			fcbArray[fd].isBufferDirty = 1;

			
			

			return count;

		}

	}

	
	
	if(count>0){

		// part 2 filled in multiples of block size
		bytesToGive = 0;
		fcbArray[fd].index = 0;
		int remainingInFile = fcbArray[fd].size - fcbArray[fd].filePosition;

		while((count/MINBLOCKSIZE)>0 && remainingInFile > 0){ // remaining of count is so big that we need more than one block

			remainingInFile = fcbArray[fd].size - fcbArray[fd].filePosition;
			
			if(remainingInFile > MINBLOCKSIZE){
				

				getBlockFile(fd);
				memcpy(resetBuffer,fcbArray[fd].buf,MINBLOCKSIZE);
				resetBuffer+=MINBLOCKSIZE;
				count -= MINBLOCKSIZE;
				bytesReadTotal += MINBLOCKSIZE;
				fcbArray[fd].filePosition += MINBLOCKSIZE;
				fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition))/MINBLOCKSIZE;

				

			}else{

				getBlockFile(fd);
				memcpy(resetBuffer,fcbArray[fd].buf,remainingInFile);
				resetBuffer += remainingInFile;
				resetOur = fcbArray[fd].buf+remainingInFile;
				count -= remainingInFile;
				bytesReadTotal += remainingInFile;
				fcbArray[fd].filePosition += remainingInFile;
				fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition))/MINBLOCKSIZE;

			}
			

			


		} 

		if(count==0){

			fcbArray[fd].isBufferDirty = 1;
			return bytesReadTotal;
		}
		// the remaining is less than 512 bytes

		// part 3 filled from refilled buffer

		getBlockFile(fd);

		// let's calculate how much good data we have in the buffer

		if((fcbArray[fd].size - fcbArray[fd].filePosition)>=MINBLOCKSIZE){ // in this case the rest of the file is bigger than buffer

			fcbArray[fd].buflen = MINBLOCKSIZE; // so the bulength is the whole 512 bytes

		}else{  // in this case the rest of the file is smaller than 512 
				// so the difference between size and where we are at in the file will be the available data in the buffer

			fcbArray[fd].buflen = fcbArray[fd].size - fcbArray[fd].filePosition;
		}


		if(count<=fcbArray[fd].buflen){ // count is less or equal to what we have in the buffer

			memcpy(resetBuffer,fcbArray[fd].buf,count);
			
			fcbArray[fd].index+= count;
			fcbArray[fd].filePosition += count;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition))/MINBLOCKSIZE;
			fcbArray[fd].isBufferDirty = 1;
			bytesReadTotal += count;

			

			// at this point bytes left to read should be zero
			
			return bytesReadTotal;




			
		}else{ // what we have is less than counter

			memcpy(resetBuffer,fcbArray[fd].buf,fcbArray[fd].buflen);
			
			fcbArray[fd].index+= fcbArray[fd].buflen;
			fcbArray[fd].filePosition += fcbArray[fd].buflen;
			fcbArray[fd].relativeFileBlockPs = ((fcbArray[fd].filePosition))/MINBLOCKSIZE;
			fcbArray[fd].isBufferDirty = 1;
			bytesReadTotal += fcbArray[fd].buflen;

			// at this point bytes left to read should be zero

			

			return bytesReadTotal;


		}

	}

	
	}
	
// Interface to Close the file	
void b_close (b_io_fd fd)
	{

		if(fcbArray[fd].isBufferDirty==0){ // we need to flush the buffer

			putBlockFile(fd);

		}

		commitDE(fd);

		free(fcbArray[fd].buf);
		free(fcbArray[fd].DEptr);
		free(fcbArray[fd].parentDir);
	}


int putBlockFile(b_io_fd fd){

	int startLocation = *(fcbArray[fd].DEptr->DIR_FstClusHI) << 16 | *(fcbArray[fd].DEptr->DIR_FstClusLO);

	if(fcbArray[fd].relativeFileBlockPs==0){
		LBAwrite(fcbArray[fd].buf,1,dataStart+startLocation);
	}

	int pointerBlock = startLocation;
	
	for(int i=0;i<fcbArray[fd].relativeFileBlockPs+1;i++){

		pointerBlock = startLocation;
		startLocation = nextBlock(startLocation);
	}


	
	LBAwrite(fcbArray[fd].buf,1,dataStart+pointerBlock);
	

	return 0;

}

int getBlockFile(b_io_fd fd){

	int   startLocation = *(fcbArray[fd].DEptr->DIR_FstClusHI) << 16 | *(fcbArray[fd].DEptr->DIR_FstClusLO);
	
	if(fcbArray[fd].relativeFileBlockPs==0){

		

		LBAread(fcbArray[fd].buf,1,dataStart+startLocation);
	}

	int pointerBlock = startLocation;
	
	for(int i=0;i<(fcbArray[fd].relativeFileBlockPs)+1;i++){

		pointerBlock = startLocation;
		startLocation = nextBlock(startLocation);
	}

	
	LBAread(fcbArray[fd].buf,1,dataStart+pointerBlock);

	return 0;

}

int commitDE(b_io_fd fd){

	// updating the DE that we have as field into the struct FCB

	// maybe the size has changed
	fcbArray[fd].DEptr->DIR_FileSize = fcbArray[fd].size;

	// some time and date variables
	time_t rawtime;
	struct tm *tmInfo;
	tmInfo = malloc(sizeof(time_t));
	time ( &rawtime );
	tmInfo = localtime ( &rawtime );
	u_int16_t fatDate= tmInfo->tm_year-80 << 9 | tmInfo->tm_mon+1 << 5 | tmInfo->tm_mday >> 1 ;
	u_int16_t fatTime = tmInfo->tm_hour << 11 | tmInfo->tm_min << 5 | tmInfo->tm_sec >> 1 ;
	u_int8_t seconds;
	seconds = time(NULL);

	fcbArray[fd].DEptr->DIR_WrtDate = fatDate;
	fcbArray[fd].DEptr->DIR_WrtTime = fatTime;

	// then copying the DE into the parent directory
	fcbArray[fd].parentDir[fcbArray[fd].indexDE] = *(fcbArray[fd].DEptr);

	// writing the parent into disk
	int parentLocation = *(fcbArray[fd].parentDir[0].DIR_FstClusHI) << 16 | *(fcbArray[fd].parentDir[0].DIR_FstClusLO);
	DIR_ENTRY * resetPtr = fcbArray[fd].parentDir;

        
	while(parentLocation!=EOF_FLAG){

		LBAwrite(resetPtr,1,parentLocation+dataStart);
		parentLocation = nextBlock(parentLocation);
		
		resetPtr = resetPtr +(MINBLOCKSIZE/sizeof(DIR_ENTRY));

	}

}





