#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include "exhash.h"
#include "BF.h"
#include "EH_functions.h"

int main(void)
{
	int block_counter,fd,block,j;
	int depth = 1;
	int bucketCounter;
	int index;
	void *block_ptr;
	EH_info *info;
	EH_CreateIndex("EH_fu","name",'c',15*sizeof(char),depth);
	info = EH_OpenIndex("EH_fu");
	
		printf("\n\n\nDepth = %d\n\n\n", info->depth);
    	printf("\n\n\n attrName = %s\n\n\n", info->attrName);
	
		fd = BF_OpenFile("EH_fu");
		block_counter = BF_GetBlockCounter(fd);
    	printf("File %d has %d depth and %d blocks\n", fd,depth,block_counter);
    	printf("Now printing indexes:\n\n");
    	/*for block 0,skip the info and print all indexes saved here*/
    	printBuckets(fd,info);
    //	printf("get bucket data: %d\n",get_bucket_data(fd,0));
    //	printf("get bucket data: %d\n",get_bucket_data(fd,242));
    //	change_bucket_data(fd,23,18);
    //	change_bucket_data(fd,132,122);
    //	change_bucket_data(fd,256,256);

    	Record Manos = { 4, "Manousos", "Torakis", "Glufada" };
      	printBuckets(fd,info);

    	EH_InsertEntry(info, Manos);
    	Record Valentin = { 5, "Valentin", "Ivanov", "Aigalew" };
    
      	printBuckets(fd,info);

	EH_CloseIndex(info);


    BF_CloseFile(fd);

	return 0;
}

