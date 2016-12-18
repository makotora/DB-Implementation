#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "exhash.h"
#include "BF.h"
#include "EH_functions.h"
#include "superfasthash.h"

int EH_CreateIndex(char *fileName, char* attrName, char attrType, int attrLength, int depth) {
    /* Add your code here */
    int i;
    int fd;
    int blocksWithBuckets;
    int blockBuckets;
    int * bucketsArray;
    void * block;
    EH_info *info;


    /*Create first block with info*/
    BF_Init();
    if (BF_CreateFile(fileName) < 0)
    {
        BF_PrintError("Error creating file");
        return -1;    
    }
    if ((fd = BF_OpenFile(fileName)) < 0) 
    {
        BF_PrintError("Error opening file");
        return -1;
    }


    if (BF_AllocateBlock(fd) < 0) 
    {
        BF_PrintError("Error allocating block");
        return -1;
    }
    if (BF_ReadBlock(fd, 0, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }
    /*write data to info*/
    int EH_info_size = 6*sizeof(int) + sizeof(char) + attrLength*sizeof(char);
    info = malloc(sizeof(EH_info));

    info->fileDesc = fd;
    info->attrType = attrType;
    info->attrLength = attrLength;
    info->attrName = attrName;
    info->depth = depth;
    info->recordLength = sizeof(Record);
    info->blockInfoSize = sizeof(BLOCK_info);
    info->bucketBlockInfoSize = sizeof(bucketBlock_info);

    memcpy(block,&EH_info_size,sizeof(int));
    block += sizeof(int);

    memcpy(block,&(info->fileDesc),sizeof(int));
    block += sizeof(int);
    memcpy(block,&(info->attrType),sizeof(char));
    block += sizeof(char);
    memcpy(block,&(info->attrLength),sizeof(int));
    block += sizeof(int);
    memcpy(block,(info->attrName),attrLength*sizeof(char));
    block += attrLength*sizeof(char);
    memcpy(block,&(info->depth),sizeof(int));
    block += sizeof(int);
    memcpy(block,&(info->recordLength),sizeof(int));
    block += sizeof(int);
    memcpy(block,&(info->blockInfoSize),sizeof(int));
    block += sizeof(int);
    memcpy(block,&(info->bucketBlockInfoSize),sizeof(int));
    block += sizeof(int);

    int total_buckets = 1 << depth;/*buckets = 2^depth (double by left shift depth times)*/
    bucketBlock_info bucketsInfo;

    /*calculate how many blocks we will allocate for buckets*/
    int buckets = total_buckets;

    /*buckets that can fit in block 0*/
    int firstBlockBuckets = (BLOCK_SIZE - sizeof(int) - EH_info_size - info->bucketBlockInfoSize) / BUCKET_SIZE;
    int bucketsPerBlock = (BLOCK_SIZE - info->bucketBlockInfoSize) / BUCKET_SIZE;/*need space for bucketBlockInfo in each bucket block*/
    blocksWithBuckets = 1;/*one for the first block (at least one block is used)*/

    if (buckets > firstBlockBuckets)
    {
        buckets -= firstBlockBuckets;/*subtract the buckets we add on the first block*/

        
        blocksWithBuckets += buckets / bucketsPerBlock;/*calculate how many fit in other blocks*/

        if (buckets % bucketsPerBlock != 0)/*remains*/
            blocksWithBuckets++;
    }

    buckets = total_buckets;/*'reset' temp variable to total_buckets*/ 
    int recordsBlock = blocksWithBuckets;/*first block with records is right after the last block with buckets*/
    
    printf("Initial buckets:%d\n",buckets);
    /*save as many buckets as we can in block 0*/
    /*calculate how many buckets fit in block 0*/
    blockBuckets = firstBlockBuckets;

    if (buckets < blockBuckets)
    {/*if we can fit all of them in block 0*/
        blockBuckets = buckets;
        buckets = 0;
        bucketsInfo.nextBucketBlock = -1;/*no next block with buckets for now*/
    }
    else
    {/*if we cant just save as many as we can in block 0*/
        buckets = buckets - blockBuckets;
        bucketsInfo.nextBucketBlock = 1;/*block 1 will be allocated soon to fit the rest of the buckets*/
    }
    
    memcpy(block,&bucketsInfo,info->bucketBlockInfoSize);/*copy bucketsInfo in block 0*/
    block += info->bucketBlockInfoSize;

    bucketsArray = malloc(blockBuckets*sizeof(int));
    /*Initialise buckets with -1*/
    for (i=0; i<blockBuckets; i++)
    {
        bucketsArray[i] = recordsBlock;
        recordsBlock++;
    }
    memcpy(block,bucketsArray,blockBuckets*sizeof(int));
    free(bucketsArray);

    if (BF_WriteBlock(fd,0) < 0)
    {
        BF_PrintError("Error writing block back");
        return -1;
    }

    /*If we didnt fit all buckets in block 0*/
    /*Allocate blocks required for our indexes/hash_table*/
    /*Initialise all indexes with -1 to show that there are no records yet*/
    int blockNum = 0;

    while (buckets != 0) 
    {        
        blockNum ++;/*start from 1 because 0 is already used for info*/
        if (buckets < bucketsPerBlock)
        {/*if we can fit all of them in block blockNum*/            
            blockBuckets = buckets;
            buckets = 0;
            bucketsInfo.nextBucketBlock = -1;/*no next block with buckets*/
        }
        else
        {/*if we cant just save as many as we can*/            
            blockBuckets = bucketsPerBlock;
            buckets = buckets - blockBuckets;
            bucketsInfo.nextBucketBlock = blockNum + 1;
            /*block (blockNum + 1) will be allocated soon to fit the rest of the buckets*/
        }
                
        /*blockBuckets = how many buckets/indexes we will initialise in this block*/

        if (BF_AllocateBlock(fd) < 0)
        {
            BF_PrintError("Error allocating block");
            return -1;
        }

        if (BF_ReadBlock(fd, blockNum, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }

        memcpy(block,&bucketsInfo,info->bucketBlockInfoSize);/*copy bucketsInfo in this block*/
        block += info->bucketBlockInfoSize;

        
        bucketsArray = malloc(blockBuckets*sizeof(int));
        for (i=0;i<blockBuckets;i++)
        {
            bucketsArray[i] = recordsBlock;
            recordsBlock++;       
        }
        memcpy(block,bucketsArray,blockBuckets*sizeof(int));
        free(bucketsArray);

        if (BF_WriteBlock(fd,blockNum) < 0)
        {
            BF_PrintError("Error writing block back");
            return -1;
        }


    }

    BLOCK_info block_info;/*Initially,all blocks share the same block_info*/
    block_info.records = 0;
    block_info.localDepth = depth;

    /*create one block (with info) for each bucket*/
    for(i=0;i<total_buckets;i++)
    {
        blockNum++;/*We continue from the same blockNum we stopped when creating buckets*/

        if (BF_AllocateBlock(fd) < 0)
        {
            BF_PrintError("Error allocating block");
            return -1;
        }

        if (BF_ReadBlock(fd, blockNum, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }

        memcpy(block,&block_info,info->blockInfoSize);/*copy the block_info in this block*/
        
        if (BF_WriteBlock(fd,blockNum) < 0)
        {
            BF_PrintError("Error writing block back");
            return -1;
        }
    }


    if (BF_CloseFile(fd) < 0) 
    {
        BF_PrintError("Error closing file");
        return -1;
    }

    /*We saved info in file.Free the struct*/
    free(info);

    return 0;   
   
}


EH_info* EH_OpenIndex(char *fileName) {
    /* Add your code here */

    EH_info *info;
    void *block;
    int fd;
    if ((fd = BF_OpenFile(fileName)) < 0) 
    {
        BF_PrintError("Error opening file");
        return NULL;
    }
    printf("HI");

    if (BF_ReadBlock(fd, 0, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return NULL;
    }

    int EH_info_size; 
    memcpy(&EH_info_size,block,sizeof(int));
    block += sizeof(int);

    info = malloc(sizeof(EH_info));

    memcpy(&(info->fileDesc),block,sizeof(int));
    block += sizeof(int);
    memcpy(&(info->attrType),block,sizeof(char));
    block += sizeof(char);
    memcpy(&(info->attrLength),block,sizeof(int));
    block += sizeof(int);

    info->attrName = malloc( (info->attrLength+1)*sizeof(char));
    memcpy(info->attrName,block,(info->attrLength)*sizeof(char));
    info->attrName[info->attrLength] = '\0';
    block += (info->attrLength)*sizeof(char);

    memcpy(&(info->depth),block,sizeof(int));
    block += sizeof(int);
    memcpy(&(info->recordLength),block,sizeof(int));
    block += sizeof(int);
    memcpy(&(info->blockInfoSize),block,sizeof(int));
    block += sizeof(int);
    memcpy(&(info->bucketBlockInfoSize),block,sizeof(int));
    block += sizeof(int);

    printf("tessst %c %d %s %d",info->attrType,info->attrLength,info->attrName,info->depth);

    return info;
   
} 



int EH_CloseIndex(EH_info* header_info) {
    /* Add your code here */

    int fd = header_info->fileDesc;    
    if (BF_CloseFile(fd) < 0) 
    {
        BF_PrintError("Error closing file");
        return -1;
    }

    free(header_info->attrName);
    free(header_info);

    return 0;
  
}

int EH_InsertEntry(EH_info* header_info, Record record) {
    /* Add your code here */
    void *block;
    int fd = header_info->fileDesc;
    int depth = header_info->depth;
    int numBuckets = 1 << depth;
    int recordLength = header_info->recordLength;
    int blockInfoSize = header_info->blockInfoSize;


    printf("\n---In EH_InsertEntry function---\n"); 
    char *key = malloc( 25*sizeof(char));

    if (!strcmp(header_info->attrName,"id"))
    {
        my_itoa(record.id, &key);
    }
    else if (!strcmp(header_info->attrName,"name"))
    {
        strcpy(key,record.name);
    }
    else if (!strcmp(header_info->attrName,"surname"))
    {
        strcpy(key,record.surname);
    }
    else if (!strcmp(header_info->attrName,"city"))
    {
        strcpy(key,record.city);
    }
    
    int hash_key = hash(key, strlen(key)) % numBuckets; //the bucket in which this entry is gonna go
    int currentBlock = get_bucket_data(fd, hash_key);

    if (BF_ReadBlock(fd, currentBlock, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }

    BLOCK_info block_info;
    memcpy(&block_info, block, blockInfoSize);
    
    int max_records = (BLOCK_SIZE - blockInfoSize) / recordLength;

    //printBuckets(fd,header_info);
    printf("numBuckets = %d\nhashed to bucket %d\n(in block %d) ,records :%d\n",numBuckets,hash_key,currentBlock,block_info.records);
    if (block_info.records < max_records)
    {
        printf("Adding record to block\n\n");
        addRecordToBlock(fd, currentBlock, record);
    }
    else
    {
        printf("global d = %d\nlocal d = %d\n\n",depth,block_info.localDepth);
        if (block_info.localDepth == depth)/*if block's local depth is equal to global depth*/
            extend(header_info);

        split_bucket(header_info,hash_key, currentBlock, record);
    }

    free(key);
    printf("\n---Out of EH_InsertEntry---\n");

    return 0;
   
}


int EH_GetAllEntries(EH_info header_info, void *value) {
    /* Add your code here */


    return 0;
   
}


int HashStatistics(char* filename) {
    /* Add your code here */
    EH_info * info = EH_OpenIndex(filename);
    void * block;
    int i,j;
    int fd = info->fileDesc;
    int depth = info->depth;
    int totalBlocks = BF_GetBlockCounter(fd);

    printf("\n---HashStatistics---\n");
    printf("Hash file has %d blocks\n",totalBlocks);
    
    /*Copy all buckets (their blocks) in an array*/
    int blockBuckets;
    int bucketCounter;
    int total_buckets = 1 << depth;

    int *init_array_buckets = malloc(total_buckets*BUCKET_SIZE); 
    int *array_buckets = init_array_buckets;
    bucketBlock_info bucketsInfo;
    int currBlock = 0;

    if (BF_ReadBlock(fd, currBlock, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }

    int EH_info_size;
    memcpy(&EH_info_size,block,sizeof(int));
    block += sizeof(int);
    block += EH_info_size;
    memcpy(&bucketsInfo, block, info->bucketBlockInfoSize);
    block += info->bucketBlockInfoSize;
    /*calculate how many buckets fit in bucket 0*/
    blockBuckets = (BLOCK_SIZE - sizeof(int) - EH_info_size - info->bucketBlockInfoSize) / BUCKET_SIZE;
    int current_buckets = total_buckets;

    if (current_buckets <= blockBuckets)
    {/*if there all buckets are in block 0*/
        bucketCounter = current_buckets;
        current_buckets = 0;
    }
    else
    {
        bucketCounter = blockBuckets;
        current_buckets -= blockBuckets;
    }
    memcpy(array_buckets, block, bucketCounter*BUCKET_SIZE);
    array_buckets += bucketCounter;
        
    /*calculate how many buckets fit in a block (besides block 0)*/
    int bucketsPerBlock = (BLOCK_SIZE - info->bucketBlockInfoSize) / BUCKET_SIZE;
    //printf("next bucket block is %d\nnew buckeets =%d\n", bucketsInfo.nextBucketBlock, all_buckets );
    while(current_buckets != 0)
    {
        currBlock = bucketsInfo.nextBucketBlock;
        if (BF_ReadBlock(fd, bucketsInfo.nextBucketBlock, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }
        memcpy(&bucketsInfo, block, info->bucketBlockInfoSize);
        block += info->bucketBlockInfoSize;

        /*If this block has all the rest buckets*/
        if (current_buckets <= bucketsPerBlock)
        {
            bucketCounter = current_buckets;
            current_buckets = 0;
        }
        else
        {
            bucketCounter = bucketsPerBlock;/*get all of these block's buckets*/
            current_buckets -= bucketsPerBlock;
        }
    
        memcpy(array_buckets, block, bucketCounter*BUCKET_SIZE);

        array_buckets+= bucketCounter;
    }
    /*Done copying*/
    array_buckets = init_array_buckets;/*'reset' this pointer/point to the beginning of the array again*/

    BLOCK_info block_info;
    int * recordBlocks = malloc(total_buckets*sizeof(int));
    recordBlocks[0] = array_buckets[0];
    int recordBlocksNum = 1;

    if (BF_ReadBlock(fd, recordBlocks[0], &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }
    memcpy(&block_info,block,info->blockInfoSize);

    int totalRecords = block_info.records;
    int maxNumOfRecords = recordBlocks[0];
    int minNumOfRecords = recordBlocks[0];

    for (i=1;i<total_buckets;i++)/*we already added the first one*/
    {
        int recordsNum;
        int recordBlock = array_buckets[i];
        int isNewRecordBlock = 1;
        for (j=0;j<recordBlocksNum;j++)
        {
            if ( recordBlock == recordBlocks[j])
            {
                isNewRecordBlock = 0;
                break;
            }
        }

        if (isNewRecordBlock)
        {
            if (BF_ReadBlock(fd, recordBlock, &block) < 0) 
            {
                BF_PrintError("Error getting block");
                return -1;
            }
            memcpy(&block_info,block,info->blockInfoSize);
            recordsNum = block_info.records;
            recordBlocks[recordBlocksNum++] = recordBlock;

            totalRecords += recordsNum;

            if (recordsNum > maxNumOfRecords)
                maxNumOfRecords = recordsNum;

            if (recordsNum < minNumOfRecords)
                minNumOfRecords = recordsNum;
        }
    }
    float average = (float) totalRecords / (float) recordBlocksNum;
    printf("TotalRecords: %d TotalRecordBlocks %d\n",totalRecords,recordBlocksNum);
    printf("Minimum number of records: %d\n",minNumOfRecords);
    printf("Average number of records: %.5f\n",average);
    printf("Maximum number of records: %d\n",maxNumOfRecords);
    free(recordBlocks);
    EH_CloseIndex(info);

    return -1;
   
}
