#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "exhash.h"
#include "BF.h"

void create_Index(char *fileName, char *attrName, char attrType, int depth) {
    int attrLength = strlen(attrName);
    assert(!EH_CreateIndex(fileName, attrName, attrType, attrLength, depth));
}

EH_info *open_Index(char *fileName) {
    EH_info *info;
    assert((info = EH_OpenIndex(fileName)) != NULL);
    return info;
}

void close_Index(EH_info *info) {
    assert(!EH_CloseIndex(info));
}

void insert_Entries(EH_info *info) {
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

        assert(!EH_InsertEntry(info, record));
    }
    free(line);
}

void get_AllEntries(EH_info *info, void *value) {
    assert(EH_GetAllEntries(*info, value) != -1);
}

#define fileName "EH_hashFile"
int main(int argc, char **argv) {
    BF_Init();
    EH_info *info;

    // -- create index
    char attrName[20];
    int depth = 7;
    strcpy(attrName, "city");
    char attrType = 'c';
    // strcpy(attrName, "id");
    // char attrType = 'i';
    create_Index(fileName, attrName, attrType, depth);

    // -- open index
    info = open_Index(fileName);

    // -- insert entries
    insert_Entries(info);

    // -- get all entries
    char value[20];
    strcpy(value, "Keratsini");
    get_AllEntries(info, value);
    // int value = 11903588;
    // get_AllEntries(info, &value);

    // -- close index
    close_Index(info);

    HashStatistics(fileName);

    // clean up
//    free(info->attrName);
//    free(info);
    info = NULL;

    return EXIT_SUCCESS;
}
