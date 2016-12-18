#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "hash.h"

void printBuckets(int fd,HT_info * info,int block_counter)
{
    printf("BLOCK #0\n");
    int bucketCounter = 0;
    void * block_ptr;
    int block;
    int index;
    int j;
    BF_ReadBlock(fd, 0, &block_ptr);
    
    printf("BLOCK #0\n");
    block_ptr += sizeof(*info);
    for(j = 0;j<(BLOCK_SIZE - sizeof(*info)) / sizeof(int);j++)
    {
        memcpy(&index,block_ptr,sizeof(int));
        printf("%d ",index);
        if (index == -1)
            bucketCounter++;
        block_ptr += sizeof(int);
    }
    printf("\nFound %d buckets!\n\n",bucketCounter);
    for(block=1;block<block_counter;block++)
    {
        printf("BLOCK #%d\n",block);
        BF_ReadBlock(fd, block, &block_ptr);
        bucketCounter = 0;
        for (j=0;j<BLOCK_SIZE / sizeof(int);j++)
        {
            memcpy(&index,block_ptr,sizeof(int));
            printf("%d ",index);
            if (index == -1)
                bucketCounter++;
            block_ptr += sizeof(int);
        }
        printf("\nFound %d buckets!\n\n",bucketCounter);
    }
}


char *my_itoa(int num, char **str)
{
    if(*str == NULL)
    {
        return NULL;
    }
    sprintf(*str, "%d", num);
    return *str;
}

void printRecord(Record record)
{
    printf("{%d, %s, %s, %s}\n", record.id, record.name, record.surname, record.city);
}


int createBlock_and_addRecord(int fd, Record record)
{
    void *block;
    if(BF_AllocateBlock(fd) < 0) 
    {
        BF_PrintError("Error allocating block");
        return -1;
    }
    
    int blockNum = BF_GetBlockCounter(fd)-1;
    
    if (BF_ReadBlock(fd, blockNum, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }
   
    BLOCK_info block_info = {1, -1};
    memcpy(block, &block_info,sizeof(BLOCK_info));

    block += sizeof(BLOCK_info);
    memcpy(block, &record, sizeof(record));

    if(BF_WriteBlock(fd, blockNum) < 0)
    {
        BF_PrintError("Error writing block back");
        return -1;
    }   
    return 0;
}


int addRecordToBlock(int fd, int blockNum, Record record)
{
    void *block;
    if (BF_ReadBlock(fd, blockNum, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }

    BLOCK_info block_info;
    memcpy(&block_info, block, sizeof(BLOCK_info));     //Reading block info
    
    block_info.records++;                               //Block records++
    memcpy(block, &block_info, sizeof(BLOCK_info));     

    block += sizeof(BLOCK_info) + (block_info.records-1)*sizeof(Record);
    memcpy(block, &record, sizeof(Record));

    if(BF_WriteBlock(fd, blockNum) < 0)
    {
        BF_PrintError("Error writing block back");
        return -1;
    }   
    return 0;

}


int printBlock(int fd, int blockNum)
{
    int i;
    void *block;
    if (BF_ReadBlock(fd, blockNum, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }

    BLOCK_info block_info;
    memcpy(&block_info, block, sizeof(BLOCK_info));     //Reading block info
    //printf("Block info\nRecords:%d\nNext Block: %d\n", block_info.records, block_info.nextBlock);
    
    block+= sizeof(BLOCK_info);

    Record record;
    for (i = 0; i < block_info.records; i++)
    {
        memcpy(&record, block, sizeof(Record));
        printRecord(record);
        block+= sizeof(Record);
    }

    //printf("Block %d has %d bytes free space\n", blockNum, BLOCK_SIZE - sizeof(BLOCK_info) - block_info.records*sizeof(Record));
    return 0;
}


int get_bucket_data(int fd, int bucketNum)
{

    int data;
    int offset=0;
    int blockNum=0;

    void *block= NULL;
    offset += sizeof(HT_info) + bucketNum*BUCKET_SIZE;
    while (offset >= BLOCK_SIZE)
    {
        offset -= BLOCK_SIZE;
        blockNum++;
    	
    }

    if (BF_ReadBlock(fd, blockNum, &block) < 0) 
    {
            BF_PrintError("Error getting block");
            return -1;
    }
    block += offset;
    
    memcpy(&data,block,sizeof(int));
    return data;
}


int change_bucket_data(int fd, int bucketNum, int newData)
{
    int offset=0;
    int blockNum=0;

    void *block= NULL;
    offset += sizeof(HT_info) + bucketNum*BUCKET_SIZE;
    while (offset >= BLOCK_SIZE)
    {
        offset -= BLOCK_SIZE;
        blockNum++;
    }

    if (BF_ReadBlock(fd, blockNum, &block) < 0) 
    {
            BF_PrintError("Error getting block");
            return -1;
    }
    block += offset;
    
    memcpy(block,&newData,sizeof(int));
    if (BF_WriteBlock(fd,blockNum) < 0)
    {
        BF_PrintError("Error writing block back");
        return -1;
    }

    return 0;
}


int cmpID(Record record,char *value)
{
	char * idString = malloc(40*sizeof(char));
	int cmp;

	my_itoa(record.id,&idString);
	cmp = strcmp(idString,value);
	free(idString);

	return cmp == 0;
}
int cmpName(Record record,char *value)
{
	return strcmp(record.name,value) == 0;
}
int cmpSurname(Record record,char *value)
{
	return strcmp(record.surname,value) == 0;
}
int cmpCity(Record record,char *value)
{
	return strcmp(record.city,value) == 0;
}