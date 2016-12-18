#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hash.h"
#include "BF.h"

void create_Index(char *fileName, char *attrName, char attrType, int buckets) {
    int attrLength = strlen(attrName);
    assert(!HT_CreateIndex(fileName, attrType, attrName, attrLength, buckets));
}

HT_info *open_Index(char *fileName) {
    HT_info *info;
    assert((info = HT_OpenIndex(fileName)) != NULL);
    return info;
}

void close_Index(HT_info *info) {
    assert(!HT_CloseIndex(info));
}

void insert_Entries(HT_info *info) {
    FILE *stream;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    stream = stdin;
    Record record;

    while ((read = getline(&line, &len, stream)) != -1) {
        line[read - 2] = 0;
        char *pch;

        pch = strtok(line, ",");
        record.id = atoi(pch);

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.name, pch, sizeof(record.name));

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.surname, pch, sizeof(record.surname));

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.city, pch, sizeof(record.city));

        assert(!HT_InsertEntry(*info, record));
    }
    free(line);
}

void get_AllEntries(HT_info *info, void *value) {
    assert(HT_GetAllEntries(*info, value) != -1);
}

#define fileName "HT_hashFile"
int main(int argc, char **argv) {
    BF_Init();
    HT_info *info;

    // -- create index
    char attrName[20];
    int buckets = 128;
    strcpy(attrName, "city");
    char attrType = 'c';
    // strcpy(attrName, "id");
    // char attrType = 'i';
    create_Index(fileName, attrName, attrType, buckets);

    // -- open index
    info = open_Index(fileName);

    // -- insert entries
    insert_Entries(info);

    // -- get all entries
    char value[20];
    strcpy(value, "Keratsini");
    printf("Starting get_AllEntries!\n");
    get_AllEntries(info, value);
    // int value = 11903588;
    // get_AllEntries(info, &value);

    // -- close index
    close_Index(info);

    // clean up
    //free(info->attrName);
    //free(info);
    info = NULL;

    return EXIT_SUCCESS;
}
