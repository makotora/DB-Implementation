#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "hash.h"

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


int get_bucket_data(int fd, int bucketNum)
{

    int data;
    int offset=0;
    void *block= NULL;

    int blockNum=1;/*there is at least one block with buckets*/

    if (BF_ReadBlock(fd, 0, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }
    int HT_info_size;
    memcpy(&HT_info_size,block,sizeof(int));

    offset = sizeof(int) + HT_info_size + bucketNum*BUCKET_SIZE;

    int blockBuckets = (BLOCK_SIZE - sizeof(int) - HT_info_size) / BUCKET_SIZE;
    while (offset + 4 > BLOCK_SIZE)
    {
        bucketNum -= blockBuckets;
        offset = bucketNum*BUCKET_SIZE;
        blockBuckets = (BLOCK_SIZE / BUCKET_SIZE);
        blockNum++;
    }
    
    if (BF_ReadBlock(fd, blockNum-1, &block) < 0) 
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
    void *block= NULL;

    int blockNum=1;/*there is at least one block with buckets*/

    if (BF_ReadBlock(fd, 0, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }
    int HT_info_size;
    memcpy(&HT_info_size,block,sizeof(int));

    offset = sizeof(int) + HT_info_size + bucketNum*BUCKET_SIZE;

    int blockBuckets = (BLOCK_SIZE - sizeof(int) - HT_info_size) / BUCKET_SIZE;
    while (offset + 4 > BLOCK_SIZE)
    {
        bucketNum -= blockBuckets;
        offset = bucketNum*BUCKET_SIZE;
        blockBuckets = (BLOCK_SIZE / BUCKET_SIZE);
        blockNum++;
    }
    
    if (BF_ReadBlock(fd, blockNum-1, &block) < 0) 
    {
            BF_PrintError("Error getting block");
            return -1;
    }
    block += offset;
    
    memcpy(block,&newData,sizeof(int));
    if (BF_WriteBlock(fd,blockNum-1) < 0)
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

int max(int *max, int x)
{
    if (*max < x)
        *max = x;
    return 0;
}

int min(int *min, int x)
{
    if (*min > x)
        *min = x;
    return 0;
}
