#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "exhash.h"

int get_bucket_data(int fd, int bucketNum)
{
    int data;
    int i;
    int currBlock;
    int nextBlock;
    int offset=0;
    bucketBlock_info bucket_info;    
    void *block= NULL;

    int blockNum=1;/*there is at least one block with buckets*/

    if (BF_ReadBlock(fd, 0, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }
    int EH_info_size;
    memcpy(&EH_info_size,block,sizeof(int));

    offset = sizeof(int) + EH_info_size + sizeof(bucketBlock_info) + bucketNum*BUCKET_SIZE;

    int blockBuckets = (BLOCK_SIZE - sizeof(int) - EH_info_size - sizeof(bucketBlock_info)) / BUCKET_SIZE;
    while (offset + 4 > BLOCK_SIZE)
    {
        bucketNum -= blockBuckets;
        offset = sizeof(bucketBlock_info) + bucketNum*BUCKET_SIZE;
        blockBuckets = (BLOCK_SIZE - sizeof(bucketBlock_info)) / BUCKET_SIZE;
        blockNum++;
    }
    
    block += sizeof(int);
    block += EH_info_size;
    memcpy(&bucket_info,block,sizeof(bucketBlock_info));
    currBlock = 0;
    nextBlock = bucket_info.nextBucketBlock;


    for (i=1;i<blockNum;i++)
    {
        currBlock = nextBlock;


        if (BF_ReadBlock(fd, currBlock, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }

        memcpy(&bucket_info,block,sizeof(bucketBlock_info));
        nextBlock = bucket_info.nextBucketBlock;
    }

    void * correct_block;
    if (BF_ReadBlock(fd, currBlock, &correct_block) < 0) 
    {
            BF_PrintError("Error getting block");
            return -1;
    }
    correct_block += offset;

    memcpy(&data,correct_block,sizeof(int));

    return data;
}


int change_bucket_data(int fd, int bucketNum, int newData)
{
    int i;
    int currBlock;
    int nextBlock;
    int offset=0;
    bucketBlock_info bucket_info;    
    void *block= NULL;

    int blockNum=1;/*there is at least one block with buckets*/

    if (BF_ReadBlock(fd, 0, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }
    int EH_info_size;
    memcpy(&EH_info_size,block,sizeof(int));

    offset = sizeof(int) + EH_info_size + sizeof(bucketBlock_info) + bucketNum*BUCKET_SIZE;

    int blockBuckets = (BLOCK_SIZE - sizeof(int) - EH_info_size - sizeof(bucketBlock_info)) / BUCKET_SIZE;
    while (offset + 4 > BLOCK_SIZE)
    {
        bucketNum -= blockBuckets;
        offset = sizeof(bucketBlock_info) + bucketNum*BUCKET_SIZE;
        blockBuckets = (BLOCK_SIZE - sizeof(bucketBlock_info)) / BUCKET_SIZE;
        blockNum++;
    }
    
    block += sizeof(int);
    block += EH_info_size;
    memcpy(&bucket_info,block,sizeof(bucketBlock_info));
    currBlock = 0;
    nextBlock = bucket_info.nextBucketBlock;


    for (i=1;i<blockNum;i++)
    {
        currBlock = nextBlock;

        if (BF_ReadBlock(fd, currBlock, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }

        memcpy(&bucket_info,block,sizeof(bucketBlock_info));
        nextBlock = bucket_info.nextBucketBlock;
    }

    if (BF_ReadBlock(fd, currBlock, &block) < 0) 
    {
            BF_PrintError("Error getting block");
            return -1;
    }
    block += offset;
    memcpy(block,&newData,sizeof(int));
    
    if (BF_WriteBlock(fd,currBlock) < 0)
    {
        BF_PrintError("Error writing block back");
        return -1;
    }

    return 0;
}


int split_bucket(EH_info * eh_info, int bucket_num, int block_num, Record record)
{
      int fd = eh_info->fileDesc;
      void *block;
      int records;
      BLOCK_info info;
      if (BF_ReadBlock(fd, block_num, &block) < 0) 
      {
          BF_PrintError("Error getting block");
          return -1;
      }

      /*Copying info*/
      memcpy(&info, block, sizeof(BLOCK_info));

      records = info.records;
   
      /*Copying all the records*/
      Record *recs = malloc(records*sizeof(Record));  
      block += sizeof(BLOCK_info); 
      memcpy(recs, block, records*sizeof(Record));
   
      /*Changing info*/
      info.records = 0;
      info.localDepth++;
      block -= sizeof(BLOCK_info);/*get back to the start*/
      memcpy(block, &info, sizeof(BLOCK_info));

      if (BF_WriteBlock(fd,block_num) < 0)
      {
          BF_PrintError("Error writing block back");
          return -1;
      }

      if (BF_AllocateBlock(fd) < 0)
      {
          BF_PrintError("Error allocating block");
          return -1;
      }

      int new_block = BF_GetBlockCounter(fd)-1;

      if (BF_ReadBlock(fd, new_block, &block) < 0) 
      {
          BF_PrintError("Error getting block");
          return -1;
      }

      /*New block will have info as the old block. Records = 0 and localdepth the same*/
      memcpy(block, &info, sizeof(BLOCK_info));

      if (BF_WriteBlock(fd, new_block) < 0)
      {
          BF_PrintError("Error writing block back");
          return -1;
      }

      change_bucket_data(fd, bucket_num, new_block);

      for (int i = 0; i < records; ++i)
      {

          EH_InsertEntry(eh_info, recs[i]);
      }
      free(recs);
      EH_InsertEntry(eh_info, record);

      return 0;
}



int extend(EH_info *info)
{
    int fd = info->fileDesc;
    int current_buckets, old_buckets;
    int blocks_with_buckets = 0;
    int currBlock = 0;
    int blockBuckets;
    int bucketCounter;
    old_buckets = 1 << info->depth;
    
    int *init_array_buckets = malloc(old_buckets*BUCKET_SIZE); 
    int *array_buckets = init_array_buckets;
    void *block;
    bucketBlock_info bucketsInfo;
    

    if (BF_ReadBlock(fd, currBlock, &block) < 0) 
    {
        BF_PrintError("Error getting block");
        return -1;
    }

    printf("Old buckets: %d\n", old_buckets);
    info->depth++;
    printf("New buckets: %d\n", 1<<info->depth);

    int EH_info_size;
    memcpy(&EH_info_size,block,sizeof(int));
    block += sizeof(int);
    block += 2* sizeof(int) + sizeof(char) + info->attrLength*sizeof(char);
    memcpy(block,&(info->depth), sizeof(int));
    block -= 2* sizeof(int) + sizeof(char) + info->attrLength*sizeof(char);
    block += EH_info_size;
    int test;
    memcpy(&test,block,sizeof(int));

    if (BF_WriteBlock(fd, currBlock) < 0)
    {
        BF_PrintError("Error writing block back");
        return -1;
    }



    memcpy(&bucketsInfo, block, info->bucketBlockInfoSize);
    block += info->bucketBlockInfoSize;
    /*calculate how many buckets fit in bucket 0*/
    blockBuckets = (BLOCK_SIZE - sizeof(int) -EH_info_size - info->bucketBlockInfoSize) / BUCKET_SIZE;
    current_buckets = old_buckets;
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
        
    blocks_with_buckets++;

    /*calculate how many buckets fit in a block (besides block 0)*/
    int bucketsPerBlock = (BLOCK_SIZE - info->bucketBlockInfoSize) / BUCKET_SIZE;

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

        blocks_with_buckets++;        
    }

    array_buckets = init_array_buckets;/*'reset' this pointer/point to the beginning of the array again*/
    
    /*We read what we want to copy.Start Writing!*/

    /*bucketCounter now has the number of buckets for the last block (with buckets)*/
    current_buckets = old_buckets;/*We need to copy old_buckets buckets*/

    int bytes_remaining = (BLOCK_SIZE - info->bucketBlockInfoSize - bucketCounter*BUCKET_SIZE);

    if (blocks_with_buckets == 1)
      bytes_remaining -= (sizeof(int) + EH_info_size);

    int remaining_bucks = bytes_remaining/BUCKET_SIZE;
    /*Continue writing from last block if we can*/
    if( bytes_remaining >= BUCKET_SIZE)
    {
        //Exw akoma xoro sto block
        if(current_buckets <= remaining_bucks)
        {/*if we can write all buckets in this block*/
            bucketsInfo.nextBucketBlock = -1;
            remaining_bucks = current_buckets;
        }
        else
          bucketsInfo.nextBucketBlock = BF_GetBlockCounter(fd);
        
        current_buckets -= remaining_bucks;/*This will become 0 if all buckets fit in this block*/
        
        block -= info->bucketBlockInfoSize;/*set pointer behind bucket block info*/
        memcpy(block, &bucketsInfo , info->bucketBlockInfoSize);
        block += info->bucketBlockInfoSize;
        
        block+= bucketCounter*BUCKET_SIZE;//Vriskomai ekei pou teleiwnoun ta buckets tou teleutaiou block me buckets
        
        memcpy(block, array_buckets, remaining_bucks*BUCKET_SIZE);
        
        if (BF_WriteBlock(fd, currBlock) < 0)
        {
            BF_PrintError("Error writing block back");
            return -1;
        }
        array_buckets += remaining_bucks;
    }

    currBlock = BF_GetBlockCounter(fd) - 1;
    while(current_buckets != 0)/*until we copy all buckets*/
    {
        currBlock++;
        if (BF_AllocateBlock(fd) < 0) 
        {
            BF_PrintError("Error allocating block");
            return -1;
        }
        if (BF_ReadBlock(fd, currBlock, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }

        if(current_buckets <= bucketsPerBlock)
        {
            bucketsInfo.nextBucketBlock = -1;
            bucketCounter = current_buckets;
        }
        else
        {
            bucketsInfo.nextBucketBlock = BF_GetBlockCounter(fd);
            bucketCounter = bucketsPerBlock;
        }

        current_buckets -= bucketCounter;

        memcpy(block, &bucketsInfo, info->bucketBlockInfoSize);
        block += info->bucketBlockInfoSize;

        memcpy(block,array_buckets,bucketCounter*BUCKET_SIZE);

        if (BF_WriteBlock(fd, currBlock) < 0)
        {
            BF_PrintError("Error writing block back");
            return -1;
        }

        array_buckets += bucketCounter;
    }

    //printBuckets(fd,info);
    free(init_array_buckets);

    return 0; 
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

    //printBuckets(fd,EH_OpenIndex("EH_hashFile"));
    if(BF_WriteBlock(fd, blockNum) < 0)
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
