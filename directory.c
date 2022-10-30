/**************************************************************
 * Class:  CSC-415-01 Spring 2022
 * Names: Shahriz Malek, Duccio Rocca, Abhiram Rishi Prattipati, Christopher Solorzano
 * Student IDs: 920378989, 922254031, 921346982, 920528216
 * GitHub Name: RINO-GAELICO
 * Group Name: Sunset
 * Project: Basic File System
 *
 * File: directory.c
 *
 * Description: The file that manages the creation and initialization of directories
 *
 **************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fsLow.h"
#include "fs_struct.h"
#include "fat.h"
#include "directory.h"
#include "mfs.h"

fdDir * fdPtr;

const char* DELIM = " /";

struct DIR_ENTRY * directory;

DIR_ENTRY *initDirectoryRoot(struct DIR_ENTRY *directory, int elementsCount, int locationDir){

    uint32_t location = locationDir;
	u_int16_t lower_word = (u_int16_t) (location & 0xFFFFUL);
	u_int16_t upper_word = (u_int16_t) ((location >> 16) & 0xFFFFUL);

    
   
    time_t rawtime;
    struct tm *tmInfo;
    tmInfo = malloc(sizeof(time_t));
    time ( &rawtime );
    tmInfo = localtime ( &rawtime );

    u_int16_t fatDate= tmInfo->tm_year-80 << 9 | tmInfo->tm_mon+1 << 5 | tmInfo->tm_mday >> 1 ;
    
    u_int16_t fatTime = tmInfo->tm_hour << 11 | tmInfo->tm_min << 5 | tmInfo->tm_sec >> 1 ;

    u_int8_t seconds;
    seconds = time(NULL);


    int fileSize = elementsCount*(sizeof(DIR_ENTRY));


    for(int i=0 ; i<elementsCount ; i++){
        
        if(i==0){

            
            strcpy(directory[i].DIR_Name,".");
            directory[i].DIR_Attr = IS_DIR;
            directory[i].DIR_WrtTime = fatTime; // last time modified
            directory[i].DIR_WrtDate = fatDate; // date of last time modified
            directory[i].DIR_FileSize = fileSize; 
            directory[i].DIR_CrtDate = fatDate; // Date file was created. OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTime = fatTime; // Time file was created OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTimeMs = seconds; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
            memset(directory[i].DIR_FstClusLO, lower_word, 2);
            memset(directory[i].DIR_FstClusHI, upper_word, 2);
            directory[i].DIR_LstAccDate = fatDate; // Last access date OPTIONAL AS PER SPECS
            
        } else if(i==1){

           

            
            strcpy(directory[i].DIR_Name,"..");
            directory[i].DIR_Attr = IS_DIR;
            directory[i].DIR_WrtTime = fatTime; // last time modified
            directory[i].DIR_WrtDate = fatDate; // date of last time modified
            directory[i].DIR_FileSize = fileSize; 
            directory[i].DIR_CrtDate = fatDate; // Date file was created. OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTime = fatTime; // Time file was created OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTimeMs = seconds; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
            memset(directory[i].DIR_FstClusLO, lower_word, 2);
            memset(directory[i].DIR_FstClusHI, upper_word, 2);
            directory[i].DIR_LstAccDate = fatDate; // Last access date OPTIONAL AS PER SPECS

        } else{
            memset(directory[i].DIR_Name, 0x00, 11);
            directory[i].DIR_Attr = 0x00;
            directory[i].DIR_WrtTime = 0x0000; // last time modified
            directory[i].DIR_WrtDate = 0x0000; // date of last time modified
            directory[i].DIR_FileSize = 0x00000000; 
            directory[i].DIR_CrtTime = 0x0000; // Time file was created OPTIONAL AS PER SPECS
            directory[i].DIR_CrtDate = 0x0000; // Date file was created. OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTimeMs = 0x00; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
            directory[i].DIR_FstClusHI[0] = 0xff;//High word this entry’s 1st cluster #
            directory[i].DIR_FstClusHI[1] = 0xff;
            directory[i].DIR_FstClusLO[0] = 0xff; // Low word of 1st cluster #
            directory[i].DIR_FstClusLO[1] = 0xff;
            directory[i].DIR_LstAccDate = 0x0000; // Last access date OPTIONAL AS PER SPECS
        }

        
        
        directory[i].DIR_NTRes = 0x00; //reserved: set to 0 when a file is created
        
        

        
    }
    
	

    return directory;
}

DIR_ENTRY *initDirectory(struct DIR_ENTRY *directory, DIR_ENTRY parentDE, int elementsCount, int locationDir){

    uint32_t location = locationDir;
	u_int16_t lower_word = (u_int16_t) (location & 0xFFFFUL);
	u_int16_t upper_word = (u_int16_t) ((location >> 16) & 0xFFFFUL);

    
   
    time_t rawtime;
    struct tm *tmInfo;
    tmInfo = malloc(sizeof(time_t));
    time ( &rawtime );
    tmInfo = localtime ( &rawtime );

    u_int16_t fatDate= tmInfo->tm_year-80 << 9 | tmInfo->tm_mon+1 << 5 | tmInfo->tm_mday >> 1 ;
    
    u_int16_t fatTime = tmInfo->tm_hour << 11 | tmInfo->tm_min << 5 | tmInfo->tm_sec >> 1 ;

    u_int8_t seconds;
    seconds = time(NULL);

   


    int fileSize = elementsCount*(sizeof(DIR_ENTRY));


    for(int i=0 ; i<elementsCount ; i++){
        
        if(i==0){

            
            strcpy(directory[i].DIR_Name,".");
            directory[i].DIR_Attr = IS_DIR;
            directory[i].DIR_WrtTime = fatTime; // last time modified
            directory[i].DIR_WrtDate = fatDate; // date of last time modified
            directory[i].DIR_FileSize = fileSize; 
            directory[i].DIR_CrtDate = fatDate; // Date file was created. OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTime = fatTime; // Time file was created OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTimeMs = seconds; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
            memset(directory[i].DIR_FstClusLO, lower_word, 2);
            memset(directory[i].DIR_FstClusHI, upper_word, 2);
            directory[i].DIR_LstAccDate = fatDate; // Last access date OPTIONAL AS PER SPECS
            
        } else if(i==1){

           

            
            strcpy(directory[i].DIR_Name,"..");
            directory[i].DIR_Attr = IS_DIR;
            directory[i].DIR_WrtTime = parentDE.DIR_WrtDate; // last time modified
            directory[i].DIR_WrtDate = parentDE.DIR_WrtDate; // date of last time modified
            directory[i].DIR_FileSize = parentDE.DIR_FileSize; 
            directory[i].DIR_CrtDate = parentDE.DIR_CrtDate; // Date file was created. OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTime = parentDE.DIR_CrtTime; // Time file was created OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTimeMs = parentDE.DIR_CrtTimeMs; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
            u_int16_t lower_word = (u_int16_t) *(parentDE.DIR_FstClusLO);
	        u_int16_t upper_word = (u_int16_t) *(parentDE.DIR_FstClusHI);
            memset(directory[i].DIR_FstClusLO, lower_word, 2);
            memset(directory[i].DIR_FstClusHI, upper_word, 2);
            directory[i].DIR_LstAccDate = parentDE.DIR_LstAccDate; // Last access date OPTIONAL AS PER SPECS

        } else{
            memset(directory[i].DIR_Name, 0x00, 11);
            directory[i].DIR_Attr = 0x00;
            directory[i].DIR_WrtTime = 0x0000; // last time modified
            directory[i].DIR_WrtDate = 0x0000; // date of last time modified
            directory[i].DIR_FileSize = 0x00000000; 
            directory[i].DIR_CrtTime = 0x0000; // Time file was created OPTIONAL AS PER SPECS
            directory[i].DIR_CrtDate = 0x0000; // Date file was created. OPTIONAL AS PER SPECS
            directory[i].DIR_CrtTimeMs = 0x00; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
            directory[i].DIR_FstClusHI[0] = 0xff;//High word this entry’s 1st cluster #
            directory[i].DIR_FstClusHI[1] = 0xff;
            directory[i].DIR_FstClusLO[0] = 0xff; // Low word of 1st cluster #
            directory[i].DIR_FstClusLO[1] = 0xff;
            directory[i].DIR_LstAccDate = 0x0000; // Last access date OPTIONAL AS PER SPECS
        }

        
        
        directory[i].DIR_NTRes = 0x00; //reserved: set to 0 when a file is created
        
            
    }
    
	

    return directory;
};

DE_holder *parsePath (char * path, DE_holder* DEholder){


    if(path[0] != '/'){
        printf("NOT A VALID PATH\n");
       
        return NULL;
    }
   
    //load root into memory
    DIR_ENTRY * DE_buffer;
    DIR_ENTRY *dirBuffer;
    DIR_ENTRY *parentBuffer;
    //0 false ; 1 true
    int flagNotFound =0;
    DE_buffer = malloc(sizeof(DIR_ENTRY));

    parentBuffer = malloc(sizeof(DIR_ENTRY));

    dirBuffer = malloc(rootSize*MINBLOCKSIZE);

    dirBuffer = loadDirectory(rootSize*MINBLOCKSIZE,rootCluster, dirBuffer);
  
    char* previousComponent;

    previousComponent = malloc(256);
    strcpy(previousComponent,"/");
    
    if(strlen(path)<2){

        // printf("ONLY ROOT!!!\n");
        *parentBuffer = dirBuffer[0];
        *DE_buffer = dirBuffer[0];
        DEholder->indexDE = 0;
        DEholder->DE_element = *DE_buffer;
        DEholder->isDir = 1;
        // DEholder->currentComponent = malloc(256);
        // DEholder->lastComponent =malloc(256);
        strcpy(DEholder->lastComponent, previousComponent);
        strcpy(DEholder->currentComponent, previousComponent);
        DEholder->parentDE = *parentBuffer;
        free(parentBuffer);
        free(dirBuffer);
        free(DE_buffer);
        free(previousComponent);
        return DEholder;

    }else{
        *DE_buffer = dirBuffer[0];
    }

    

    //chunking and checking for next component if it is inside the directory
    
    char* token;
    // token = malloc(256);

    token = strtok(path, DELIM);

    

    while(token!=NULL){

        
        // checking the size of the array by checking the fileSize of the dot DE and dividing by the size of a DE
        int maxEntries = dirBuffer[0].DIR_FileSize/sizeof(DIR_ENTRY);
       

        // traverse the array of DIREntries to see id we find the token
        for(int i=0; i<maxEntries; i++){
 
            int compare = strcasecmp(dirBuffer[i].DIR_Name, token);
           
                
            if(compare==0){ 
                // we found the entry so we store it in the DE_BUffer
                
                    
                    *DE_buffer = dirBuffer[i];
                    DEholder->indexDE = i;
                    DEholder->DE_element = *DE_buffer;
                    *parentBuffer = dirBuffer[0];
                    DEholder->parentDE = *parentBuffer;
                    // DEholder->currentComponent = malloc(256);
                    // DEholder->lastComponent =malloc(256);
                    strcpy(DEholder->currentComponent,token);
                    strcpy(DEholder->lastComponent, previousComponent);
                    int tempIsDIR = checkDirBit(DE_buffer->DIR_Attr);
                    // printf("**** THIS SHOULD BE == 1 -------> %d\n",tempIsDIR);
                    DEholder->isDir = tempIsDIR;
                   
                   
                    
                    
                
                
                
                break;
            }
            // :( we didn't find any DE with that name so it is not a valid path
            if(i==(maxEntries-1)){
               
                
                flagNotFound = 1;
                *DE_buffer = dirBuffer[0];
                *parentBuffer = dirBuffer[0];
                DEholder->DE_element = *DE_buffer;
                DEholder->indexDE = -1;
                DEholder->parentDE = *parentBuffer;
                DEholder->isDir = -1;
                
                strcpy(DEholder->lastComponent, previousComponent);
                strcpy(DEholder->currentComponent,token);


                // if this is the last element the path is valid but last element does not exist
                // otherwise invalid path

                          
    
            }
        }
        strcpy(previousComponent,token);

        // next Component loaded into token
        token = strtok(NULL, DELIM);

        // if path is over we get out of while loop

        // if there is no more in the path and the last element was not found
        // path is valid but last element does not exist
        if(token==NULL && flagNotFound==1){
            free(parentBuffer);
            free(dirBuffer);
            free(DE_buffer);
            free(previousComponent);
            
            return DEholder;
        }

        // if there is more in the path and last element does not exist
        // then not a valid path
        if(token!=NULL && flagNotFound==1){
            printf("INVALID PATH\n");
            free(parentBuffer);
            free(dirBuffer);
            free(DE_buffer);
            free(previousComponent);
            // free(DEholder->lastComponent);
            // free(DEholder->currentComponent);
            free(DEholder);
            // free(token);
            return NULL;
        }

        // otherwise we prepare for next level

        // element was Found and there is no more
        if(token==NULL && flagNotFound==0){

            free(parentBuffer);
            free(dirBuffer);
            free(DE_buffer);
            free(previousComponent);
            
            
            
            return DEholder;
        }


        // element was found and there is more 
        // we have to make sure last element is a dir
        // otherwise No valid Path

        
        // mallloc enough to hold the new Component
        int dirSize = DE_buffer->DIR_FileSize;
        free(dirBuffer);
        dirBuffer = malloc(dirSize);
        int dirLocation = *(DE_buffer->DIR_FstClusHI) << 16 | *(DE_buffer->DIR_FstClusLO);

        // there is more so token!=NULL 
        // let's check if the last element we found is a dir 
        // otherwise return NULL

        if(!(checkDirBit(DE_buffer->DIR_Attr))){

            printf("NOT VALID PATH, NOT A DIRECTORY: %s\n",DE_buffer->DIR_Name);
            free(parentBuffer);
            free(dirBuffer);
            free(DE_buffer);
            free(previousComponent);
            free(DEholder);
            
            return NULL;
        }

        // we found the last element; there is more in the path to parse and 
        // last element is a dir :)

        // load the whole dir inside the dirBuffer
        free(dirBuffer);
        dirBuffer = malloc(dirSize);
        dirBuffer = loadDirectory(dirSize, dirLocation, dirBuffer);


    }

    free(parentBuffer);
    free(dirBuffer);
    free(DE_buffer);
    free(previousComponent);
    // free(token);

    return DEholder;

};


DIR_ENTRY * loadDirectory (int fileSize, int firstLocation, DIR_ENTRY * dirBuffer){

    //set the pointer to the first position
    int pointerBlock = firstLocation;
    int bufferPointer = 0;

    
    DIR_ENTRY * resetPointer;
    resetPointer = dirBuffer;

   

    //looping till we reach the EOF and reading one block at the time

    while(pointerBlock != EOF_FLAG){
        
        //read one block at the time
        LBAread(dirBuffer+bufferPointer,1,dataStart+pointerBlock);
        
        // the pointer allows us to move through the chain
        pointerBlock = nextBlock(pointerBlock);

        //advance the buffer one block
        bufferPointer= bufferPointer+(MINBLOCKSIZE/sizeof(DIR_ENTRY));
        
    
    }

    // we reached the EOF and the dirBuffer is holding the whole Directory
    dirBuffer = resetPointer;

    return dirBuffer;
    
    
};



// 1 = failed , 0 = success
int fs_mkdir(const char *pathname, mode_t attribute) {

    

    char * currentPath;
    char* pathnameVar;
    pathnameVar = malloc(strlen(pathname)+2);
    currentPath = malloc(MAX_DEPTH_PATH);

    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
    DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);
    
    if(pathname[0]!='/'){ // relative path

  
        strcpy(currentPath,currentDir);
        strcat(currentPath,"/");
        strcat(currentPath,pathname);
        DEholder = parsePath(currentPath, DEholder);

    }else{ // absolute path
        
        strcpy(pathnameVar,pathname);
        DEholder = parsePath(pathnameVar, DEholder);
    }

    

    // STEP ONE: validation to check if DIR_ENTRY is in the parent 
    //it should be a valid path but the last element should not be in the parent dir
    if(DEholder!=NULL && DEholder->indexDE==(-1)){

        

        //STEP TWO: return DE or a pointer to the last element
        int parentLocation = *(DEholder->parentDE.DIR_FstClusHI) << 16 | *(DEholder->parentDE.DIR_FstClusLO);
        int parentSize = DEholder->parentDE.DIR_FileSize;



        DIR_ENTRY * dirBufferParent;
        dirBufferParent = malloc(parentSize);
        

        
        dirBufferParent = loadDirectory(parentSize, parentLocation, dirBufferParent);
        int positionIndex=-1;


        

        //STEP THREE: find unused DE in parent
        for(int i=0 ; i< (dirBufferParent[0].DIR_FileSize/sizeof(DIR_ENTRY)) ; i++){

            

            if(dirBufferParent[i].DIR_Name[0] == 0x00){

                positionIndex = i;
                break;
            }
            
        }

        

        
        //STEP FOUR: Allocate space for new dir
        int startLocationNewDir = alloc(4);


        //STEP FIVE: init the DE for new DIR
        DIR_ENTRY * newDirPtr;
        newDirPtr = malloc(sizeof(DIR_ENTRY)*64);
        newDirPtr = initDirectory(newDirPtr, DEholder->parentDE, 64, startLocationNewDir);

        

        //STEP SIX: Fill in parent array at the positionIndex
        memcpy(dirBufferParent[positionIndex].DIR_Name,DEholder->currentComponent,11);
        dirBufferParent[positionIndex].DIR_Attr = IS_DIR;
        dirBufferParent[positionIndex].DIR_WrtTime = newDirPtr[0].DIR_WrtTime; // last time modified
        dirBufferParent[positionIndex].DIR_WrtDate = newDirPtr[0].DIR_WrtDate; // date of last time modified
        dirBufferParent[positionIndex].DIR_FileSize = newDirPtr[0].DIR_FileSize; 
        dirBufferParent[positionIndex].DIR_CrtDate = newDirPtr[0].DIR_CrtDate; // Date file was created. OPTIONAL AS PER SPECS
        dirBufferParent[positionIndex].DIR_CrtTime = newDirPtr[0].DIR_CrtTime; // Time file was created OPTIONAL AS PER SPECS
        dirBufferParent[positionIndex].DIR_CrtTimeMs = newDirPtr[0].DIR_CrtTimeMs; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
        dirBufferParent[positionIndex].DIR_LstAccDate = newDirPtr[0].DIR_LstAccDate; // Last access date OPTIONAL AS PER SPECS
        

        u_int16_t lower_word = (u_int16_t) *(newDirPtr[0].DIR_FstClusLO);
	    u_int16_t upper_word = (u_int16_t) *(newDirPtr[0].DIR_FstClusHI);
        memset(dirBufferParent[positionIndex].DIR_FstClusLO, lower_word, 2);
        memset(dirBufferParent[positionIndex].DIR_FstClusHI, upper_word, 2);

        

        // 2 reset pointer so thatt we can successfully make a free call to the original pointer
        DIR_ENTRY * resetPtr = newDirPtr;
        DIR_ENTRY * resetPtr2 = dirBufferParent;

        //STEP SEVEN: write new dir to disk 
        
        while(parentLocation!=EOF_FLAG){

            LBAwrite(resetPtr2,1,parentLocation+dataStart);
            parentLocation = nextBlock(parentLocation);
            
            resetPtr2 = resetPtr2 +(MINBLOCKSIZE/sizeof(DIR_ENTRY));

        }

        
        //STEP 8: parent dir to disk

        
        while(startLocationNewDir!=EOF_FLAG){

            LBAwrite(resetPtr ,1, startLocationNewDir+dataStart);
            startLocationNewDir = nextBlock(startLocationNewDir);
            
            resetPtr  = resetPtr  + (MINBLOCKSIZE/sizeof(DIR_ENTRY));
        }
        
        free(pathnameVar);
        free(currentPath);

        free(DEholder->currentComponent);
		free(DEholder->lastComponent);
		free(DEholder);

        free(newDirPtr);
        free(dirBufferParent);
        
        
        return 0;
            
    }


    if(DEholder!=NULL){

		free(DEholder->currentComponent);
		free(DEholder->lastComponent);
		
	}
    free(DEholder);
    free(pathnameVar);
    free(currentPath);

    return 1;

    
};

// 1 = failed , 0 = success
int fs_rmdir(const char *pathname){

    char * currentPath;
    char* pathnameVar;
    pathnameVar = malloc(strlen(pathname));
    currentPath = malloc(MAX_DEPTH_PATH);

    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
    DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);

    
    
    if(pathname[0]!='/'){ // relative path

  
        strcpy(currentPath,currentDir);
        strcat(currentPath,"/");
        strcat(currentPath,pathname);
        DEholder = parsePath(currentPath, DEholder);

    }else{ // absolute path
        
        strcpy(pathnameVar,pathname);
        DEholder = parsePath(pathnameVar, DEholder);
    }
   
    

    if(DEholder!=NULL && DEholder->indexDE>0 && DEholder->isDir==1){

        DIR_ENTRY * remDirPtr;
        remDirPtr = malloc(DEholder->DE_element.DIR_FileSize);
        int locationRemDir = *(DEholder->DE_element.DIR_FstClusHI) << 16 | *(DEholder->DE_element.DIR_FstClusLO);
        if(locationRemDir==rootStart){
            printf("CANNOT REMOVE ROOT\n");
            return (-1);
        }
        remDirPtr = loadDirectory(DEholder->DE_element.DIR_FileSize, locationRemDir, remDirPtr);

        //traverse the dir to check is empty

        for(int i=2; i< (remDirPtr[0].DIR_FileSize/sizeof(DIR_ENTRY)) ; i++){
            if(remDirPtr[i].DIR_Name[0] != 0x00 ){ // chek if even one of theDE is not UNUSED

                free(currentPath);
                free(pathnameVar);
                free(DEholder->currentComponent);
                free(DEholder->lastComponent);
                free(DEholder);
                free(remDirPtr);
                
                return 1; // failed the dir is not empty
            }
        }
        int startLocationRemDir = *(remDirPtr[0].DIR_FstClusHI) << 16 | *(remDirPtr[0].DIR_FstClusLO);
        release(startLocationRemDir);

        int parentLocation = *(DEholder->parentDE.DIR_FstClusHI) << 16 | *(DEholder->parentDE.DIR_FstClusLO);
        int parentSize = DEholder->parentDE.DIR_FileSize;
        DIR_ENTRY * dirBuffer;
        dirBuffer = malloc(parentSize);
        dirBuffer = loadDirectory(parentSize, parentLocation, dirBuffer);
        int indexRmDir = DEholder->indexDE;


        memset(dirBuffer[indexRmDir].DIR_Name, 0x00, 11);
        dirBuffer[indexRmDir].DIR_Attr = 0x00;
        dirBuffer[indexRmDir].DIR_WrtTime = 0x0000; // last time modified
        dirBuffer[indexRmDir].DIR_WrtDate = 0x0000; // date of last time modified
        dirBuffer[indexRmDir].DIR_FileSize = 0x00000000; 
        dirBuffer[indexRmDir].DIR_CrtTime = 0x0000; // Time file was created OPTIONAL AS PER SPECS
        dirBuffer[indexRmDir].DIR_CrtDate = 0x0000; // Date file was created. OPTIONAL AS PER SPECS
        dirBuffer[indexRmDir].DIR_CrtTimeMs = 0x00; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
        dirBuffer[indexRmDir].DIR_FstClusHI[0] = 0xff;//High word this entry’s 1st cluster #
        dirBuffer[indexRmDir].DIR_FstClusHI[1] = 0xff;
        dirBuffer[indexRmDir].DIR_FstClusLO[0] = 0xff; // Low word of 1st cluster #
        dirBuffer[indexRmDir].DIR_FstClusLO[1] = 0xff;
        dirBuffer[indexRmDir].DIR_LstAccDate = 0x0000; // Last access date OPTIONAL AS PER SPECS


        DIR_ENTRY * resetPointer = dirBuffer;

        // writing on disk the parent directory
        while(parentLocation!=EOF_FLAG){

            LBAwrite(resetPointer,1,parentLocation+dataStart);
            parentLocation = nextBlock(parentLocation);
            resetPointer = resetPointer +(MINBLOCKSIZE/sizeof(DIR_ENTRY));

        }
        free(dirBuffer);
        free(currentPath);
        free(pathnameVar);
        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
        free(DEholder);
        free(remDirPtr);
        

        return 0;

        
        
        


    }
    if(DEholder!=NULL){

        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
        
    }
    free(DEholder);
    free(currentPath);
    free(pathnameVar);
    return 1;
    

}
 



char * fs_getcwd(char *buf, size_t size){

   
    strcpy(buf, currentDir);

    return buf;

}

int fs_setcwd(char *buf){

    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
    DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);

    

    if(buf[0]=='/'){ //absolute path
        
        DEholder = parsePath(buf, DEholder);

       


        if(DEholder!= NULL && DEholder->indexDE>=0 && DEholder->isDir==1){

            strcpy(currentDir,buf);
           
            free(DEholder->currentComponent);
            free(DEholder->lastComponent);
            free(DEholder);
	    
            return 0;
        }
        else{
            if(DEholder!=NULL){

                free(DEholder->currentComponent);
                free(DEholder->lastComponent);
                
            }
            free(DEholder);
            return 1;
        }
    }else{ // relative path

        
        
        if(strcmp(buf,".")==0){

            free(DEholder->currentComponent);
            free(DEholder->lastComponent);
            free(DEholder);
            
            return 0;
        }
        if(strcmp(buf,"..")==0 && strcmp(currentDir,"/")==0){

            free(DEholder->currentComponent);
            free(DEholder->lastComponent);
            free(DEholder);
            
            return 0;
        }

        if( strcmp(buf,"..")==0 ){

            int counter = strlen(currentDir);
            char * subString;
            subString = malloc(MAX_DEPTH_PATH);

            while(currentDir[counter]!='/'){
                counter--;
            }
            if(counter==0){
                counter++;
            }
            
           
            strncpy(subString,&currentDir[0],counter);
            subString[counter]='\0';
            strcpy(currentDir,subString);

            free(DEholder->currentComponent);
            free(DEholder->lastComponent);
            free(DEholder);
            free(subString);
            
            return 0;
        }


        char * currentPath;
        currentPath = malloc(MAX_DEPTH_PATH);
        strcpy(currentPath,currentDir);
        // if the path is not equal to / let's add / to it
        if(strcmp(currentPath,"/")!=0){
            
            strcat(currentPath,"/");
        }
        
        strcat(currentPath,buf);
        

        char* resetPath;
        resetPath = malloc(MAX_DEPTH_PATH);
        strcpy(resetPath, currentPath);
        
        DEholder = parsePath(resetPath, DEholder);

        if(DEholder!= NULL && DEholder->indexDE>=0 && DEholder->isDir==1){

            
            strcpy(currentDir,currentPath);
            
            free(DEholder->currentComponent);
            free(DEholder->lastComponent);
            free(DEholder);
            free(currentPath);
            free(resetPath);
            return 0;
            
        }else{
            free(currentPath);
            if(DEholder!=NULL){

            free(DEholder->currentComponent);
            free(DEholder->lastComponent);
            }
            free(DEholder);
            free(resetPath);
            return 1;
        }
        

    }
}

int fs_isFile(char * path) {

    char * currentPath;
    char* pathnameVar;
    pathnameVar = malloc(strlen(path)+2);
    currentPath = malloc(MAX_DEPTH_PATH);

    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
    DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);
    
    if(path[0]!='/'){ // relative path

  
        strcpy(currentPath,currentDir);
        strcat(currentPath,"/");
        strcat(currentPath,path);
        DEholder = parsePath(currentPath, DEholder);

    }else{ // absolute path
        
        strcpy(pathnameVar,path);
        DEholder = parsePath(pathnameVar, DEholder);
    }



    
   
    if(DEholder!=NULL && DEholder->indexDE>=0 && DEholder->isDir==0){

        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
        free(DEholder);
        free(pathnameVar);
        free(currentPath);
        return 1;

    }else{
        if(DEholder!=NULL){

            free(DEholder->currentComponent);
            free(DEholder->lastComponent);
            
            }
            free(DEholder);
            free(pathnameVar);
            free(currentPath);
        return 0;
    }

    
};

int fs_isDir(char * path){

    char * currentPath;
    char* pathnameVar;
    pathnameVar = malloc(strlen(path)+2);
    currentPath = malloc(MAX_DEPTH_PATH);

    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
    DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);
    
    if(path[0]!='/'){ // relative path

  
        strcpy(currentPath,currentDir);
        strcat(currentPath,"/");
        strcat(currentPath,path);
        DEholder = parsePath(currentPath, DEholder);

    }else{ // absolute path
        
        strcpy(pathnameVar,path);
        DEholder = parsePath(pathnameVar, DEholder);
    }



    
   
    if(DEholder!=NULL && DEholder->indexDE>=0 && DEholder->isDir==1){

        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
        free(DEholder);
        free(pathnameVar);
        free(currentPath);
        return 1;
    }else{
        if(DEholder!=NULL){

            free(DEholder->currentComponent);
            free(DEholder->lastComponent);
            
            }
            free(DEholder);
            free(pathnameVar);
            free(currentPath);
        return 0;
    }


}

int checkDirBit(char attributeValue){


    if ( (attributeValue & (1 << (5 - 1))) > 0){ //true

		return 1;
	}
    else{ //false
            
		return 0;
	}

}




int fs_delete(char* filename){	//removes a file

    char * currentPath;
    char* pathnameVar;
    pathnameVar = malloc(strlen(filename));
    currentPath = malloc(MAX_DEPTH_PATH);

    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
    DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);
    
    if(filename[0]!='/'){ // relative path

  
        strcpy(currentPath,currentDir);
        strcat(currentPath,"/");
        strcat(currentPath,filename);
        DEholder = parsePath(currentPath, DEholder);

    }else{ // absolute path
        
        strcpy(pathnameVar,filename);
        DEholder = parsePath(pathnameVar, DEholder);
    }
   
   

    if(DEholder!=NULL && DEholder->indexDE>0 && DEholder->isDir==0){


        int locationRemFile = *(DEholder->DE_element.DIR_FstClusHI) << 16 | *(DEholder->DE_element.DIR_FstClusLO);
    
        release(locationRemFile);

        // loading the parent directory

        int parentLocation = *(DEholder->parentDE.DIR_FstClusHI) << 16 | *(DEholder->parentDE.DIR_FstClusLO);
        int parentSize = DEholder->parentDE.DIR_FileSize;

        DIR_ENTRY * dirBuffer;
        dirBuffer = malloc(parentSize);
        dirBuffer = loadDirectory(parentSize, parentLocation, dirBuffer);
        int indexRmFile = DEholder->indexDE;

        // wiping out the parent directory's DEe correspoonding to the file to be removed

        memset(dirBuffer[indexRmFile].DIR_Name, 0x00, 11);
        dirBuffer[indexRmFile].DIR_Attr = 0x00;
        dirBuffer[indexRmFile].DIR_WrtTime = 0x0000; // last time modified
        dirBuffer[indexRmFile].DIR_WrtDate = 0x0000; // date of last time modified
        dirBuffer[indexRmFile].DIR_FileSize = 0x00000000; 
        dirBuffer[indexRmFile].DIR_CrtTime = 0x0000; // Time file was created OPTIONAL AS PER SPECS
        dirBuffer[indexRmFile].DIR_CrtDate = 0x0000; // Date file was created. OPTIONAL AS PER SPECS
        dirBuffer[indexRmFile].DIR_CrtTimeMs = 0x00; // Millisecond stamp at file creation OPTIONAL AS PER SPECS
        dirBuffer[indexRmFile].DIR_FstClusHI[0] = 0xff;//High word this entry’s 1st cluster #
        dirBuffer[indexRmFile].DIR_FstClusHI[1] = 0xff;
        dirBuffer[indexRmFile].DIR_FstClusLO[0] = 0xff; // Low word of 1st cluster #
        dirBuffer[indexRmFile].DIR_FstClusLO[1] = 0xff;
        dirBuffer[indexRmFile].DIR_LstAccDate = 0x0000; // Last access date OPTIONAL AS PER SPECS


        DIR_ENTRY * resetPointer = dirBuffer;

        // writing on disk the parent directory
        while(parentLocation!=EOF_FLAG){

            LBAwrite(resetPointer,1,parentLocation+dataStart);
            parentLocation = nextBlock(parentLocation);
            resetPointer = resetPointer +(MINBLOCKSIZE/sizeof(DIR_ENTRY));

        }
        free(dirBuffer);
        free(currentPath);
        free(pathnameVar);
        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
        free(DEholder);
       
        

        return 0; 


    }
    if(DEholder!=NULL){

        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
        free(DEholder);
    }
    free(currentPath);
    free(pathnameVar);
    return 1;

    }



fdDir * fs_opendir(const char *name){

   

    char * currentPath;
    char* pathnameVar;
    pathnameVar = malloc(strlen(name)+2);
    currentPath = malloc(MAX_DEPTH_PATH);

    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
    DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);
    
    if(name[0]!='/'){ // relative path

  
        strcpy(currentPath,currentDir);
        strcat(currentPath,"/");
        strcat(currentPath,name);
        DEholder = parsePath(currentPath, DEholder);

    }else{ // absolute path
        
        strcpy(pathnameVar,name);
        DEholder = parsePath(pathnameVar, DEholder);
    }

    
   
    if(DEholder!=NULL && DEholder->indexDE>=0 && DEholder->isDir==1){

        
        
        fdPtr = malloc(sizeof(fdDir));

         

        
        fdPtr->dirPtr = malloc(DEholder->DE_element.DIR_FileSize);
        int locationOpenDir = *(DEholder->DE_element.DIR_FstClusHI) << 16 | *(DEholder->DE_element.DIR_FstClusLO);
        fdPtr->dirPtr  = loadDirectory(DEholder->DE_element.DIR_FileSize, locationOpenDir, fdPtr->dirPtr);

        
        
        fdPtr->totEntries = fdPtr->dirPtr[0].DIR_FileSize/sizeof(DIR_ENTRY);
        fdPtr->dirEntryPosition = 0;
        fdPtr->directoryStartLocation = locationOpenDir;
        fdPtr->d_reclen = sizeof(fdDir);
        fdPtr->ptrDireItem = malloc(sizeof(struct fs_diriteminfo));


        free(currentPath);
        free(pathnameVar);
        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
        free(DEholder);
        

        return fdPtr;
        
    }

    if(DEholder!=NULL){

        free(DEholder->currentComponent);
        free(DEholder->lastComponent);
       
    }
    free(DEholder);
    free(currentPath);
    free(pathnameVar);
    fdPtr = NULL;
    return fdPtr;


}



int fs_closedir(fdDir *dirp){

   

    free(dirp->ptrDireItem);
    free(dirp->dirPtr);
    free(dirp);
    return 0;

}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){

    
    
    if(dirp->dirEntryPosition >= dirp->totEntries){
        return NULL;
    }

    while(dirp->dirPtr[dirp->dirEntryPosition].DIR_Name[0] == 0x00){

        dirp->dirEntryPosition+=1;
        if(dirp->dirEntryPosition >= dirp->totEntries){
        return NULL;
        }

    }

    strcpy(dirp->ptrDireItem->d_name, dirp->dirPtr[dirp->dirEntryPosition].DIR_Name);

    
    int checkDirFile = checkDirBit(dirp->dirPtr[dirp->dirEntryPosition].DIR_Attr);

    if(checkDirFile==1){
        dirp->ptrDireItem->fileType = FT_DIRECTORY;
    }else{
        dirp->ptrDireItem->fileType = FT_REGFILE;
    }

    dirp->dirEntryPosition+=1;

    return dirp->ptrDireItem;
    
}



int fs_stat(const char *path, struct fs_stat *buf){



    char * currentPath;
    char* pathnameVar;
    pathnameVar = malloc(strlen(path)+2);
    currentPath = malloc(MAX_DEPTH_PATH);

    
    DE_holder *DEholder;
    DEholder = malloc(sizeof(DE_holder));
	DEholder->lastComponent = malloc(256);
    DEholder->currentComponent = malloc(256);
    
    if(path[0]!='/'){ // relative path

  
        strcpy(currentPath,currentDir);
        strcat(currentPath,"/");
        strcat(currentPath,path);
        
        DEholder = parsePath(currentPath, DEholder);

    }else{ // absolute path
        
        strcpy(pathnameVar,path);
        DEholder = parsePath(pathnameVar, DEholder);
    }

    
    
    

    if(DEholder!=NULL && DEholder->indexDE>=0){
        buf->st_accesstime = DEholder->DE_element.DIR_LstAccDate;
        buf->st_blksize = MINBLOCKSIZE;
        buf->st_blocks= DEholder->DE_element.DIR_FileSize/MINBLOCKSIZE;
        buf->st_createtime= DEholder->DE_element.DIR_CrtTime;
        buf->st_modtime= DEholder->DE_element.DIR_WrtTime;
        buf->st_size= DEholder->DE_element.DIR_FileSize;
        return 0;
    }

    return 1;

}



    





