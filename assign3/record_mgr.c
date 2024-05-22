#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"


typedef struct RecordManager 
{
	RID recordIdentifier;      
	int recordScanCount,totalRecords,firstFreePage;          
    BM_PageHandle bufferPage;  
    Expr *filterCondition;     
	BM_BufferPool pagePool;    
} RecordManager;


const int MAX_PAGES = 100;
const int ATTRIBUTE_NAME_LENGTH = 15; 

RecordManager *recordManager;

int findFreeSlot(char *data, int recordSize)
{
    int i = 0;
    int totalSlots = PAGE_SIZE / recordSize;

    while (i < totalSlots) {
        if (data[i * recordSize] != '+')
            return i;
        i++;
    }
    return -1;
}

extern RC initRecordManager(void *mgmtData) 
{
    initStorageManager();
    return RC_OK;
}


extern RC shutdownRecordManager() 
{
	recordManager = NULL; 
    return RC_OK;
}


extern RC createTable(char *name, Schema *schema) 
{

    recordManager = (RecordManager*)calloc(1, sizeof(RecordManager));
    initBufferPool(&recordManager->pagePool, name, MAX_PAGES, RS_LRU, NULL);
    char data[PAGE_SIZE];
    char *pageHandle = data;
    int result, k;
    *((int*)pageHandle) = 0;
    pageHandle += sizeof(int);
    *((int*)pageHandle) = 1;
    pageHandle += sizeof(int);
    *((int*)pageHandle) = schema->numAttr;
    pageHandle += sizeof(int);
    *((int*)pageHandle) = schema->keySize;
    pageHandle += sizeof(int);
    k = 0;

    while (k < schema->numAttr) 
	{

        strncpy(pageHandle, schema->attrNames[k], ATTRIBUTE_NAME_LENGTH);
        pageHandle += ATTRIBUTE_NAME_LENGTH;
        *((int*)pageHandle) = (int)schema->dataTypes[k];
        pageHandle += sizeof(int);
        *((int*)pageHandle) = (int)schema->typeLength[k];
        pageHandle += sizeof(int);
        k++;
    }

    SM_FileHandle fileHandle;

    result = createPageFile(name);

    if (result != RC_OK)
        return result;

    result = openPageFile(name, &fileHandle);

    if (result != RC_OK)
        return result;

    result = writeBlock(0, &fileHandle, data);

    if (result != RC_OK)
        return result;

    result = closePageFile(&fileHandle);

    if (result != RC_OK)
        return result;

    return RC_OK;
}



extern RC openTable(RM_TableData *rel, char *name) {
    
    SM_PageHandle pageHandle;
    int attributeCount, k;
    rel->mgmtData = recordManager;
    rel->name = name;

    if (pinPage(&recordManager->pagePool, &recordManager->bufferPage, 0) != RC_OK) 
	{
        return RC_PIN_PAGE_FAILED;
    }

    pageHandle = (SM_PageHandle)recordManager->bufferPage.data;
    recordManager->totalRecords = *(int*)pageHandle;

    
    if (recordManager->totalRecords < 0) 
	{
        unpinPage(&recordManager->pagePool, &recordManager->bufferPage);
        return RC_INVALID_PAGE_DATA;
    }

    pageHandle += sizeof(int);
    recordManager->firstFreePage = *(int*)pageHandle;

    if (recordManager->firstFreePage < 0) 
	{
        unpinPage(&recordManager->pagePool, &recordManager->bufferPage);
        return RC_INVALID_PAGE_DATA;
    }

    pageHandle += sizeof(int);
    attributeCount = *(int*)pageHandle;
 
    if (attributeCount < 0) 
	{
        unpinPage(&recordManager->pagePool, &recordManager->bufferPage);
        return RC_INVALID_PAGE_DATA;
    }

    pageHandle += sizeof(int);
    Schema *schema;
    schema = (Schema*)calloc(1, sizeof(Schema));
    schema->numAttr = attributeCount;
    schema->attrNames = (char**)calloc(attributeCount, sizeof(char*));
    schema->dataTypes = (DataType*)calloc(attributeCount, sizeof(DataType));
    schema->typeLength = (int*)calloc(attributeCount, sizeof(int));
    k = 0;

    while (k < attributeCount) 
	{
        schema->attrNames[k] = (char*)calloc(ATTRIBUTE_NAME_LENGTH, sizeof(char));
        k++;
    }
    k = 0;

    while (k < schema->numAttr) 
	{
        strncpy(schema->attrNames[k], pageHandle, ATTRIBUTE_NAME_LENGTH);
        pageHandle += ATTRIBUTE_NAME_LENGTH;
        schema->dataTypes[k] = *(int*)pageHandle;
        pageHandle += sizeof(int);
        schema->typeLength[k] = *(int*)pageHandle;
        pageHandle += sizeof(int);
        k++;
    }

    rel->schema = schema;
	BM_BufferPool *bufferpoolpointer = &recordManager->pagePool;
	BM_PageHandle *bufferpagepointer = &recordManager->bufferPage;
    unpinPage(bufferpoolpointer, bufferpagepointer);
    forcePage(bufferpoolpointer, bufferpagepointer);
    return RC_OK;
}

extern RC closeTable(RM_TableData *table) 
{
	RecordManager *temptablemanager =table->mgmtData;
    RecordManager *tableManager = temptablemanager;
	BM_BufferPool *bufferPool = &tableManager->pagePool;
    shutdownBufferPool(bufferPool);
    return RC_OK;
}
void tempDestroyPageFile(char *name) 
	{ 
		destroyPageFile(name); 
	}

extern RC deleteTable(char *name) 
{
	
    tempDestroyPageFile(name);
    return RC_OK;
}

extern int getNumTuples(RM_TableData *rel) 
{
	
    RecordManager *rectemppointer = rel->mgmtData;
	RecordManager *recMgr = rectemppointer;
    int tupleCount = recMgr->totalRecords;
    return tupleCount;
}

extern RC insertRecord(RM_TableData *rel, Record *record) 
{
    RecordManager *recMgrpointer = rel->mgmtData;
	BM_BufferPool *bufferpoolpointer = &recMgrpointer->pagePool;
    RecordManager *recMgr = recMgrpointer;
    BM_BufferPool *bufferPool = bufferpoolpointer;
    BM_PageHandle *pageHandle = &recMgrpointer->bufferPage;
    RID *recordID = &record->id;
    char *data, *slotPtr;
    int recordSize = getRecordSize(rel->schema);
    recordID->page = recMgr->firstFreePage;
    
    do 
	{
        pinPage(bufferPool, pageHandle, recordID->page);
        data = pageHandle->data;
        recordID->slot = findFreeSlot(data, recordSize);
        
        if (recordID->slot == -1) 
		{
            unpinPage(bufferPool, pageHandle);
            recordID->page++;
            pinPage(bufferPool, pageHandle, recordID->page);
        }
    } while (recordID->slot == -1);
    
    slotPtr = data + (recordID->slot * recordSize);
    *slotPtr = '+';
    memcpy(slotPtr + 1, record->data + 1, recordSize - 1);
    recMgr->totalRecords++;
    pinPage(bufferPool, pageHandle, 0); 
    unpinPage(bufferPool, pageHandle);
    return RC_OK;
}

extern RC deleteRecord(RM_TableData *rel, RID id) 
{
    RecordManager *recordMgr = rel->mgmtData;
    BM_BufferPool *bufferPool = &recordMgr->pagePool;
    BM_PageHandle *pageHandle = &recordMgr->bufferPage;
    pinPage(bufferPool, pageHandle, id.page);
    recordMgr->firstFreePage = id.page;  
    char *pageData = pageHandle->data; 
    int recordSize = getRecordSize(rel->schema); 
    char *recordData = pageData + (id.slot * recordSize);
    *recordData = '-';
    markDirty(bufferPool, pageHandle);
    unpinPage(bufferPool, pageHandle);
    return RC_OK;
}

extern RC updateRecord(RM_TableData *rel, Record *record) 
{
    RecordManager *recordMgr = rel->mgmtData;
    BM_BufferPool *bufferPool = &recordMgr->pagePool;
    BM_PageHandle *pageHandle = &recordMgr->bufferPage;
    pinPage(bufferPool, pageHandle, record->id.page);
    int recordSize = getRecordSize(rel->schema);
    RID id = record->id;
    char *pageData = pageHandle->data;
    char *recordData = pageData + id.slot * recordSize;
    *recordData = '+';
    memcpy(recordData + 1, record->data + 1, recordSize - 1);
    markDirty(bufferPool, pageHandle);
    unpinPage(bufferPool, pageHandle);
    return RC_OK;
}

extern RC getRecord(RM_TableData *rel, RID id, Record *record) 
{
    RecordManager *recordMgr = rel->mgmtData;
    BM_BufferPool *bufferPool = &recordMgr->pagePool;
    BM_PageHandle *pageHandle = &recordMgr->bufferPage;
    pinPage(bufferPool, pageHandle, id.page);
    int recordSize = getRecordSize(rel->schema);
    char *recordDataPtr = pageHandle->data;
    recordDataPtr += id.slot * recordSize;

    while (*recordDataPtr != '+') 
	{
        unpinPage(bufferPool, pageHandle);
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    record->id = id;
    char *recordData = record->data;
    memcpy(++recordData, recordDataPtr + 1, recordSize - 1);
    unpinPage(bufferPool, pageHandle);
    return RC_OK;
}

extern RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) 
{
    if (cond == NULL) 
	{
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    openTable(rel, "ScannedTable");

    RecordManager *scanMgr = (RecordManager*)calloc(1, sizeof(RecordManager));
    
    int initialPage = 1;
    int initialSlot = 0;
    int initialScanCount = 0;

	scan->mgmtData = scanMgr;
    scanMgr->recordIdentifier.page = initialPage;
    scanMgr->recordIdentifier.slot = initialSlot;
    scanMgr->recordScanCount = initialScanCount;
    scanMgr->filterCondition = cond;
    scan->rel = rel;

    RecordManager *tableMgr = rel->mgmtData;
    tableMgr->totalRecords = ATTRIBUTE_NAME_LENGTH;

    return RC_OK;
}

extern RC next(RM_ScanHandle *scan, Record *record) 
{
    RecordManager *sManager = scan->mgmtData, *tManager = scan->rel->mgmtData;
    Schema *sch = scan->rel->schema;

    if (sManager->filterCondition == NULL)
	{
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    Value *result = (Value *)calloc(1, sizeof(Value));

    int rSize = getRecordSize(sch);
    int totalSlots = PAGE_SIZE / rSize;
    int tuplesCount = tManager->totalRecords;

    if (tuplesCount == 0) 
	{
        free(result);
        return RC_RM_NO_MORE_TUPLES;
    }

    while (sManager->recordScanCount < tuplesCount) 
	{
        int currentPage = 1 + sManager->recordScanCount / totalSlots;
        int currentSlot = sManager->recordScanCount % totalSlots;
        pinPage(&tManager->pagePool, &sManager->bufferPage, currentPage);
        char *data = sManager->bufferPage.data;
        data += currentSlot * rSize;
        record->id.page = currentPage;
        record->id.slot = currentSlot;
        char *dataPointer = record->data;
        *dataPointer = '-';
        memcpy(++dataPointer, data + 1, rSize - 1);
        sManager->recordScanCount++;
        evalExpr(record, sch, sManager->filterCondition, &result);

        if (result->v.boolV) 
		{
            unpinPage(&tManager->pagePool, &sManager->bufferPage);
            free(result);
            return RC_OK;
        }

        unpinPage(&tManager->pagePool, &sManager->bufferPage);
    }

    sManager->recordScanCount = 0;
    free(result);
    return RC_RM_NO_MORE_TUPLES;
}

extern RC closeScan(RM_ScanHandle *scan) 
{
    RecordManager *scanManager = (RecordManager *)scan->mgmtData;  
    RecordManager *recordManager = scan->rel->mgmtData;

    if (scanManager == NULL) 
	{
        return RC_SCAN_NOT_INITIALIZED; 
    }

    while (scanManager->recordScanCount > 0) 
	{
        unpinPage(&recordManager->pagePool, &scanManager->bufferPage);
        scanManager->recordScanCount--;
    }

    free(scan->mgmtData);
    return RC_OK;
}

extern int getRecordSize(Schema *schema) 
{
    int rsize = 0;
    int i = 0;
    int dataTypeSizes[] = {sizeof(int), sizeof(float), sizeof(bool)};

    while (i < schema->numAttr) 
	{
        int dataType = schema->dataTypes[i];

        if (dataType == DT_STRING) 
		{
            rsize += schema->typeLength[i];
        } 
		else if (dataType >= DT_INT && dataType <= DT_BOOL) 
		{
            rsize += dataTypeSizes[dataType - DT_INT];
        }
        i++; 
    }

    return rsize;
}

Schema *createSchema(int numAttrs, char **attrNames, DataType *dataTypes, int *typeLengths, int keySize, int *keyAttrs) 
{
    Schema *sch = (Schema *)calloc(1, sizeof(Schema));
    
    if (sch == NULL) 
	{
        return NULL;
    }

    sch->numAttr = numAttrs;
    sch->attrNames = attrNames;
    sch->dataTypes = dataTypes;
    sch->typeLength = typeLengths;
    sch->keySize = keySize;
    sch->keyAttrs = keyAttrs;

    return sch;
}



extern RC freeSchema(Schema *schema) 
{
    if (schema != NULL) 
	{
        free(schema);
        schema = NULL; 
    }
    return RC_OK;
}

extern RC createRecord(Record **record, Schema *schema) 
{
    Record *newRecord = (Record*)calloc(1, sizeof(Record));
    int recordSize = getRecordSize(schema);
    newRecord->data = (char*)calloc(recordSize, sizeof(char));
    newRecord->id.page = -1;
    newRecord->id.slot = -1;
    char *dataPointer = newRecord->data;
    char t = '-';
    *dataPointer = t;
    dataPointer++;
    char nullTerminator = '\0';
    *dataPointer = nullTerminator;
    *record = newRecord;
    return RC_OK;
}



RC attrOffset(Schema *schema, int attrNum, int *result) 
{
    *result = 1;
    int i = 0;
    int dataTypeSizes[] = {sizeof(int), sizeof(float), sizeof(bool)};

    while (i < attrNum) 
	{
        if (schema->dataTypes[i] == DT_STRING) 
		{
            *result += schema->typeLength[i];
        } 
		else if (schema->dataTypes[i] >= DT_INT && schema->dataTypes[i] <= DT_BOOL) 
		{
            *result += dataTypeSizes[schema->dataTypes[i] - DT_INT];
        }
        i++;
    }

    return RC_OK;
}

extern RC freeRecord(Record *record) 
{
    if (record != NULL) 
	{
        free(record);
        record = NULL; 
    }
    return RC_OK;
}


extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) 
{
    int attributeOffset = 0;
    attrOffset(schema, attrNum, &attributeOffset);
    Value *attrValue = (Value *)calloc(1, sizeof(Value));
    char *dataPtr = record->data;
    dataPtr += attributeOffset;
    schema->dataTypes[attrNum] = (attrNum == 1) ? 1 : schema->dataTypes[attrNum];
    if (schema->dataTypes[attrNum] == DT_STRING) 
	{
		int schemaattrNum= schema->typeLength[attrNum];
        int length = schemaattrNum;
		int incLength = schemaattrNum+1;
        attrValue->v.stringV = (char *)malloc(incLength);
        strncpy(attrValue->v.stringV, dataPtr, length);
        attrValue->v.stringV[length] = '\0';
        attrValue->dt = DT_STRING;
    } 
	else if (schema->dataTypes[attrNum] == DT_INT) 
	{
        int intValue = 0;
        memcpy(&intValue, dataPtr, sizeof(int));
        attrValue->v.intV = intValue;
        attrValue->dt = DT_INT;
    } 
	else if (schema->dataTypes[attrNum] == DT_FLOAT) 
	{
        float floatValue;
        memcpy(&floatValue, dataPtr, sizeof(float));
        attrValue->v.floatV = floatValue;
        attrValue->dt = DT_FLOAT;
    } 
	else if (schema->dataTypes[attrNum] == DT_BOOL) 
	{
        
        bool boolValue;
        memcpy(&boolValue, dataPtr, sizeof(bool));
        attrValue->v.boolV = boolValue;
        attrValue->dt = DT_BOOL;
    } 
	else 
	{
        printf("Serializer not defined for the given datatype.\n");
    }

    *value = attrValue;
    return RC_OK;
}



extern RC setAttr(Record *record, Schema *schema, int attrNum, Value *value)
{
    int attributeOffset = 0;
    attrOffset(schema, attrNum, &attributeOffset);
	char *dataPtrpointer = record->data;
    char *dataPtr = dataPtrpointer;
    dataPtr += attributeOffset;
    DataType dataType = schema->dataTypes[attrNum];

    switch (dataType)
    {
        case DT_STRING:
        {
            
            int length = schema->typeLength[attrNum];
            const char *strValue = value->v.stringV;
            while (length-- > 0)
            {
                *dataPtr++ = *strValue++;
            }
            break;
        }

        case DT_INT:
    
            *(int *)dataPtr = value->v.intV;
            break;

        case DT_FLOAT:
            
            *(float *)dataPtr = value->v.floatV;
            break;

        case DT_BOOL:
            
            *(bool *)dataPtr = value->v.boolV;
            break;

        default:
            printf("Serializer not defined for the given datatype.\n");
            break;
    }

    return RC_OK;
}
