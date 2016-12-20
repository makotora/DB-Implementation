#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "hash.h"

void printBuckets(int fd,HT_info * info,int block_counter);
char *my_itoa(int num, char **str);
void printRecord(Record record);

int createBlock_and_addRecord(int fd, Record record);
int addRecordToBlock(int fd, int blockNum, Record record);
int printBlock(int fd, int blockNum);

int get_bucket_data(int fd, int bucketNum);
int change_bucket_data(int fd, int bucketNum, int newData);

int cmpID(Record record,char *value);
int cmpName(Record record,char *value);
int cmpSurname(Record record,char *value);
int cmpCity(Record record,char *value);

int max(int *max, int x);
int min(int *min, int x);

#endif
