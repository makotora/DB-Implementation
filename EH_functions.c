#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "exhash.h"

int get_bucket_data(int fd, int bucketNum)
{
    //printf("\n---In get_bucket_data---\n");
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

    offset += sizeof(int) + EH_info_size + sizeof(bucketBlock_info) + bucketNum*BUCKET_SIZE;
    //printf("offset %d\n",offset );
    
    while (offset >= BLOCK_SIZE)
    {
        offset -= BLOCK_SIZE;
        offset += sizeof(bucketBlock_info);
        blockNum++;
      
    }
    //printf("offset %d\n",offset );
    
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
        //printf("currBlock %d\n",currBlock); 
        nextBlock = bucket_info.nextBucketBlock;
    }

    if (BF_ReadBlock(fd, currBlock, &block) < 0) 
    {
            BF_PrintError("Error getting block");
            return -1;
    }
    block += offset;

    memcpy(&data,block,sizeof(int));
    //printf("\nData got :%d",data);
    //printf("\n---Out of get_bucket_data---\n");
    return data;
}


int change_bucket_data(int fd, int bucketNum, int newData)
{
    printf("\n---In change_bucket_data---\n");
    int offset=0;
    int currBlock;
    int nextBlock;
    int i;
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

    offset += sizeof(int) + EH_info_size + sizeof(bucketBlock_info) + bucketNum*BUCKET_SIZE;
    printf("offset %d\n",offset );
    while (offset >= BLOCK_SIZE)
    {
        offset -= BLOCK_SIZE;
        offset += sizeof(bucketBlock_info);
        printf("offset %d\n",offset );
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
//EDWWW GAMW TI MANA TU
        if (BF_ReadBlock(fd, currBlock, &block) < 0) 
        {
            printf("bucketData %d",get_bucket_data(fd,bucketNum));
            fprintf(stderr,"blockNum %d offset %d currBlock %d \n",blockNum,offset,currBlock);
            BF_PrintError("Error getting block");
            return -1;
        }

        memcpy(&bucket_info,block,sizeof(bucketBlock_info));
        printf("currBlock %d\n",currBlock); 
        nextBlock = bucket_info.nextBucketBlock;
    }

    block += offset;
    printf("Gonna change_bucket_data..with new data %d for bucket %d\n",newData,bucketNum);
    printf("offset %d\n",offset );
    printf("\ncurrBlock:%d,blockNum:%d\n",currBlock,blockNum );
    memcpy(block,&newData,sizeof(int));
    
    if (BF_WriteBlock(fd,currBlock) < 0)
    {
        BF_PrintError("Error writing block back");
        return -1;
    }

    printf("Changed bucket %d data (located in block %d) and its newData is %d\n",bucketNum,currBlock,newData);
    printf("\n---Out of change_bucket_data---\n");
    return 0;
}


void printBuckets(int fd,EH_info * info)
{
  printf("\nIn print Buckets!\n\n");
    int bucketCounter;
    void * block_ptr;
    int index;
    int j;
    bucketBlock_info bucketsInfo;

    BF_ReadBlock(fd, 0, &block_ptr);
    int EH_info_size;
    memcpy(&EH_info_size,block_ptr,sizeof(int));

   	block_ptr += sizeof(int);
    block_ptr += EH_info_size;
   	printf("---BLOCK #0---\n");
   	memcpy(&bucketsInfo,block_ptr,sizeof(bucketsInfo));
   	block_ptr += sizeof(bucketsInfo);
    printf("Bucket block info:nextBucketBlock: %d \n\n",bucketsInfo.nextBucketBlock);

   	bucketCounter = 0;
   	for(j = 0;j<(BLOCK_SIZE - sizeof(*info) - sizeof(bucketsInfo)) / sizeof(int);j++)
    {
        memcpy(&index,block_ptr,sizeof(int));
        printf("%d ",index);
        if (index !=0)
        	bucketCounter++;
        block_ptr += sizeof(int);
    }
    printf("\nFound %d buckets!\n\n",bucketCounter);

    int nextBucketBlock = bucketsInfo.nextBucketBlock;
    while (nextBucketBlock != -1)
    {
    	BF_ReadBlock(fd, nextBucketBlock, &block_ptr);
    	printf("---BLOCK #%d---\n",nextBucketBlock);
    	memcpy(&bucketsInfo,block_ptr,sizeof(bucketsInfo));
   		block_ptr += sizeof(bucketsInfo);

   		printf("Bucket block info: buckets:nextBucketsBlock: %d \n\n",bucketsInfo.nextBucketBlock);
    	
    	bucketCounter = 0;
    	for(j = 0;j<(BLOCK_SIZE - sizeof(bucketsInfo)) / sizeof(int);j++)
    	{
        	memcpy(&index,block_ptr,sizeof(int));
        	printf("%d ",index);
        	if (index !=0)
        		bucketCounter++;
        	block_ptr += sizeof(int);
    	}

    	printf("\nFound %d buckets!\n\n",bucketCounter);
    	nextBucketBlock = bucketsInfo.nextBucketBlock;

    }
}

int split_bucket(EH_info * eh_info, int bucket_num, int block_num, Record record)
{
      printf("\n---In split_bucket for bucket %d (has block %d as data)---\n",bucket_num,block_num);
      int fd = eh_info->fileDesc;
      int old_localDepth;
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
      old_localDepth = info.localDepth;

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

      printf("Split:changing bucket data of %d to %d!\n\n",bucket_num,new_block) ;
      int error =  change_bucket_data(fd, bucket_num, new_block);
      if (error)
      {
        printf("Provlima gia bucket %d\n pu dixni sto block %d paei na allaksi to %d",bucket_num,block_num,bucket_num + (1 << old_localDepth));
        printBuckets(fd,eh_info);
      }
      printf("Split: result of changing bucket data\n");
      //printBuckets(fd,eh_info);

      for (int i = 0; i < records; ++i)
      {

          EH_InsertEntry(eh_info, recs[i]);
      }
      free(recs);
      EH_InsertEntry(eh_info, record);

      printf("Split: done with inserts!Result\n");
      //printBuckets(fd,eh_info);
      printf("\n---Out of split_bucket for bucket %d (has block %d as data)---\n",bucket_num,block_num);

      return 0;
}



int extend(EH_info *info)
{
    printf("\n---In extend---\n");
    int fd = info->fileDesc;
    int current_buckets, old_buckets;
    int blocks_with_buckets = 0;
    int currBlock = 0;
    int blockBuckets;
    int bucketCounter;
    int nextBucketBlock;
    old_buckets = 1 << info->depth;
    
    int *init_array_buckets = malloc(old_buckets*BUCKET_SIZE); 
    int *array_buckets = init_array_buckets;
    void *block;
    bucketBlock_info bucketsInfo;

    printf("In extend function\n");
    

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
    printf("first index !%d",test);

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

    printf("Extend:Done.printing result!\n");
    printBuckets(fd,info);
    free(init_array_buckets);
    printf("\n---Out of extend---\n");

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

    //printf("BEFORE\n");
    //printBuckets(fd,EH_OpenIndex("EH_hashFile"));
    if(BF_WriteBlock(fd, blockNum) < 0)
    {
        BF_PrintError("Error writing block back");
        return -1;
    }
    //printf("AFTER\n");
    //printBuckets(fd,EH_OpenIndex("EH_hashFile"));
    return 0;

}