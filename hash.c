#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "hash.h"
#include  "superfasthash.h"
#include "HT_functions.h"


int HT_CreateIndex( char *fileName, char attrType, char* attrName, int attrLength, int buckets) {
    /* Add your code here */

    int i;
    int fd;
    int bucketsPerBlock = BLOCK_SIZE / BUCKET_SIZE;
    int blockBuckets;
    int * bucketsArray;
    void * block;
    HT_info info;

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
    info.fileDesc = fd;
    info.attrType = attrType;
    info.attrName = attrName;
    info.attrLength = attrLength;
    info.numBuckets = buckets;
    info.recordLength = sizeof(Record);
    info.blockInfoSize = sizeof(BLOCK_info);

    memcpy(block,&info,sizeof(info));
    block += sizeof(info);/*Adjust pointer to point after the saved struct*/
    
    /*save as many buckets as we can in block 0*/
    blockBuckets = (BLOCK_SIZE - sizeof(info)) / BUCKET_SIZE;/*calculate how many buckets fit in block 0*/
    if (buckets < blockBuckets)
    {/*if we can fit all of them in block 0*/
        blockBuckets = buckets;
        buckets = 0;
    }
    else
    {/*if we cant just save as many as we can in block 0*/
        buckets = buckets - blockBuckets;
    }

    bucketsArray = malloc(blockBuckets*sizeof(int));
    /*Initialise buckets with -1*/
    for (i=0; i<blockBuckets; i++)
    {
        bucketsArray[i] = -1;
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
        blockBuckets = (buckets < bucketsPerBlock) ? buckets : bucketsPerBlock;
        /*calculate how many buckets/indexes we will initialise in this block*/
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

        bucketsArray = malloc(blockBuckets*sizeof(int));
        for (i=0;i<blockBuckets;i++)
        {
            bucketsArray[i] = -1;       
        }
        memcpy(block,bucketsArray,blockBuckets*sizeof(int));
        free(bucketsArray);

        if (BF_WriteBlock(fd,blockNum) < 0)
        {
            BF_PrintError("Error writing block back");
            return -1;
        }

        buckets = buckets - blockBuckets;/*note how many buckets we initialiased*/
    }

    if (BF_CloseFile(fd) < 0) 
    {
        BF_PrintError("Error closing file");
        return -1;
    }

    return 0;
}


HT_info* HT_OpenIndex(char *fileName) {
    /* Add your code here */

    HT_info *info;
    void *block;
    int fd;
    if ((fd = BF_OpenFile(fileName)) < 0) 
    {
        BF_PrintError("Error opening file");
        return NULL;
    }

    if (BF_ReadBlock(fd, 0, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return NULL;
    }

    info = malloc(sizeof(HT_info));

    memcpy(info, block, sizeof(HT_info));

    return info;
} 


int HT_CloseIndex( HT_info* header_info ) {
    /* Add your code here */
    
    int fd = header_info->fileDesc;    
    if (BF_CloseFile(fd) < 0) 
    {
        BF_PrintError("Error closing file");
        return -1;
    }

    free(header_info);

    return 0;

    
}

int HT_InsertEntry(HT_info header_info, Record record) {
    /* Add your code here */
    void *block;
    int fd = header_info.fileDesc;
    int numBuckets = header_info.numBuckets;
    int recordLength = header_info.recordLength;
    int blockInfoSize = header_info.blockInfoSize;

    //printf("Starting HT_InsertEntry function\n"); 
    char *key = malloc(40*sizeof(char));
    char *attrName = header_info.attrName;
    if (!strcmp(attrName,"id"))
    {
        my_itoa(record.id, &key);
    }
    else if (!strcmp(attrName,"name"))
    {
        strcpy(key,record.name);
    }
    else if (!strcmp(attrName,"surname"))
    {
        strcpy(key,record.surname);
    }
    else if (!strcmp(attrName,"city"))
    {
        strcpy(key,record.city);
    }
    
    int hash_key = hash(key, strlen(key)) % numBuckets; //the bucket in which this entry is gonna go
    int currentBlock = get_bucket_data(fd, hash_key);

    int nextBlock;
    BLOCK_info block_info;


    if (currentBlock == -1)   //If there is no records in this bucket
    {
        createBlock_and_addRecord(fd, record);
        change_bucket_data(fd, hash_key, BF_GetBlockCounter(fd)-1);
        printBlock(fd, BF_GetBlockCounter(fd)-1);
    }
    else    //if there is already records in this bucket
    {
        if (BF_ReadBlock(fd, currentBlock, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }
        
        memcpy(&block_info, block, blockInfoSize);
        nextBlock = block_info.nextBlock;
        
        while(nextBlock != -1)   //if no space to add record
        {
            currentBlock = nextBlock;
            if(BF_ReadBlock(fd, currentBlock, &block) < 0)       //Reading nextBlock 
            {

                BF_PrintError("Error getting block");
                return -1;
            }
            memcpy(&block_info, block, blockInfoSize);
            nextBlock = block_info.nextBlock;

        }

        if (BLOCK_SIZE -(blockInfoSize + block_info.records*recordLength) >= recordLength)
        {        	
            addRecordToBlock(fd, currentBlock, record);
        }
        else    
        {
            createBlock_and_addRecord(fd, record);
            block_info.nextBlock = BF_GetBlockCounter(fd) -1;
            memcpy(block,&block_info, blockInfoSize);
            
            if (BF_WriteBlock(fd,currentBlock) < 0)
    		{
        		BF_PrintError("Error writing block back");
        		return -1;
    		}
        }    

    }
    free(key);
    return 0;
}


int HT_GetAllEntries(HT_info header_info, void *value) {
    /* Add your code here */
    void *block;

    int fd = header_info.fileDesc;
    int numBuckets = header_info.numBuckets;
    int recordLength = header_info.recordLength;
    int blockInfoSize = header_info.blockInfoSize;
    char *attrName = header_info.attrName;
    
    //printf("Starting HT_GetAllEntries function\n"); 
    char *key = malloc(40*sizeof(char));
    strcpy(key,value);
	int hash_key = hash(key, strlen(key)) % numBuckets; //the bucket in which this entry is gonna go
    
    int currentBlock = get_bucket_data(fd, hash_key);
    Record * records;
    int numOfRecords;
    int blockCounter = 0;
    int i;
    BLOCK_info block_info;

    int (*compareKeyFunction)(Record,char *);/*function that checks if records key matches the value*/
    if (!strcmp(attrName,"id"))
    {
        compareKeyFunction = cmpID;
    }
    else if (!strcmp(attrName,"name"))
    {
        compareKeyFunction = cmpName;
    }
    else if (!strcmp(attrName,"surname"))
    {
        compareKeyFunction = cmpSurname;
    }
    else if (!strcmp(attrName,"city"))
    {
        compareKeyFunction = cmpCity;
    }

    while (currentBlock != -1)   //while we there is a block to search for records
    {
		blockCounter++;

     	if (BF_ReadBlock(fd, currentBlock, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }
        
        memcpy(&block_info, block, blockInfoSize);
        numOfRecords = block_info.records;

        records = malloc(numOfRecords*recordLength);
       	block += blockInfoSize;
       	memcpy(records,block,numOfRecords*recordLength);

        for(i=0;i<numOfRecords;i++)
        {
        	if (compareKeyFunction(records[i],key) == 1)
        		printRecord(records[i]);
        }

        free(records);

        currentBlock = block_info.nextBlock;
    }

    //fprintf(stderr,"\nHT_GetAllEntries finished after looking at %d blocks.\n",blockCounter);
    free(key);

    return blockCounter;
}


int HashStatistics(char* filename) {
    /* Add your code here */
    
    return -1;
    
}
