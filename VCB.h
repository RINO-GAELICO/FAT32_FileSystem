#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "fsLow.h"
#include "fs_struct.h"

BS_BPB * initVCB(BS_BPB * bpbPtr,int numberOfBlocks);