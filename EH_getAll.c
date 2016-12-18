int HT_GetAllEntries(EH_info header_info, void *value) {
    /* Add your code here */
    void *block;

    int fd = header_info.fileDesc;
    int numBuckets = 1 << header_info.depth;
    int recordLength = header_info.recordLength;
    int blockInfoSize = header_info.blockInfoSize;
    char *attrName = header_info.attrName;
    
    //printf("Starting HT_GetAllEntries function\n"); 
    char *key = malloc(40*sizeof(char));
    strcpy(key,value);
	int hash_key = hash(key, strlen(key)) % numBuckets; //the bucket in which this entry is gonna go
    
    int currentBlock = get_bucket_data(fd, hash_key);
    Record * records;
    int numOfRecords;
    int blockCounter = 0;
    int i;
    BLOCK_info block_info;

    int (*compareKeyFunction)(Record,char *);/*function that checks if records key matches the value*/
    if (!strcmp(attrName,"id"))
    {
        compareKeyFunction = cmpID;
    }
    else if (!strcmp(attrName,"name"))
    {
        compareKeyFunction = cmpName;
    }
    else if (!strcmp(attrName,"surname"))
    {
        compareKeyFunction = cmpSurname;
    }
    else if (!strcmp(attrName,"city"))
    {
        compareKeyFunction = cmpCity;
    }


     	if (BF_ReadBlock(fd, currentBlock, &block) < 0) 
        {
            BF_PrintError("Error getting block");
            return -1;
        }
        
        memcpy(&block_info, block, blockInfoSize);
        numOfRecords = block_info.records;

        records = malloc(numOfRecords*recordLength);
       	block += blockInfoSize;
       	memcpy(records,block,numOfRecords*recordLength);

        for(i=0;i<numOfRecords;i++)
        {
        	if (compareKeyFunction(records[i],key) == 1)
        		printRecord(records[i]);
        }

        free(records);

    }

    //fprintf(stderr,"\nHT_GetAllEntries finished after looking at %d blocks.\n",blockCounter);
    free(key);

    return blockCounter;
}
