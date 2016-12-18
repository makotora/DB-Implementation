#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "exhash.h"

int get_bucket_data(int fd, int bucketNum);
int change_bucket_data(int fd, int bucketNum, int newData);
void printBuckets(int fd,EH_info * info);
int split_bucket(EH_info * eh_info, int bucket_num, int block_num, Record record);
int extend(EH_info *info);
char *my_itoa(int num, char **str);
void printRecord(Record record);
int createBlock_and_addRecord(int fd, Record record,int global_depth);
int addRecordToBlock(int fd, int blockNum, Record record);


#endif