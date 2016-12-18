#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include "hash.h"
#include "BF.h"
#include "HT_functions.h"

int main(void)
{
	int block_counter,fd,block,j;
	int buckets = 60;
    int bucketCounter;
	int index;
	void *block_ptr;
	HT_info *info;
	HT_CreateIndex("fu",'c',"name",15*sizeof(char),buckets);
	info = HT_OpenIndex("fu");
	printf("Printing info struct\n");
	printf("%d\n%c\n%s\n%d\n%ld\n",info->fileDesc,info->attrType,info->attrName,info->attrLength,info->numBuckets);
	fd = BF_OpenFile("fu");
	block_counter = BF_GetBlockCounter(fd);
    printf("File %d has %d buckets and %d blocks\n", fd,buckets,block_counter);
    printf("Now printing indexes:\n\n");
    /*for block 0,skip the info and print all indexes saved here*/
    printBuckets(fd,info,block_counter);

    Record Valyo = { 201400049, "Valentin", "Ivanov", "Athens" };
    Record Manos = { 201400222, "Manousos", "Torakis", "Glufada" };

    //get_to_bucket( fd, 249);
    Record newrecord;
    char * name = malloc(2*sizeof(char));
    name[0] = 'A';
    name[1] = '\0';
    newrecord.id = 1;
    newrecord.surname[0] = 't';
    newrecord.surname[1] = '\0';
    newrecord.city[0] = 'g';
    newrecord.city[1] = '\0';
    for (int i = 0; i < 50; ++i)
    {
        newrecord.name[0] = name[0];
        newrecord.name[1] = '\0';
        name[0]++;
     	for(j=0;j<=i;j++)
	 		HT_InsertEntry(*info, newrecord);
 
    }
    printf("\n\n\n\n");
    for (int i = 1; i < BF_GetBlockCounter(fd); ++i)
    {
        printf("\n\n\n\n######BLOCK %d######\n", i);
        printBlock(fd, i);
    }

    printf("\n####Testing getallentries!####\n\n");
    printf("\nSearching for 5\n");
    HT_GetAllEntries(*info,"Valentin");
    printf("\nSearching for 20\n");
    HT_GetAllEntries(*info,"A");
    printf("\nSearching for 32\n");
    HT_GetAllEntries(*info,"C");
    printf("\nSearching for 35\n");
    HT_GetAllEntries(*info,"H");
    printf("\nSearching for 41\n");
    HT_GetAllEntries(*info,"G");
    printf("\nSearching for 44\n");
    HT_GetAllEntries(*info,"Z");
    printf("\nSearching for 45\n");
    HT_GetAllEntries(*info,"45");
    printf("\nSearching for 50\n");
    HT_GetAllEntries(*info,"50");


    BF_CloseFile(fd);

	return 0;
}

