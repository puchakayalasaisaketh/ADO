#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>

#include "storage_mgr.h"

FILE *page;

extern void initStorageManager (void) {
    
    page = NULL;
}

extern RC createPageFile(char *fileName) {
    FILE *page = fopen(fileName, "w+"); 

    if (page == NULL) return RC_FILE_NOT_FOUND;
    
    SM_PageHandle emptyPageBuffer = (SM_PageHandle)malloc(PAGE_SIZE);

    if (emptyPageBuffer == NULL) {
        
        fclose(page); 
        return RC_MALLOC_FAILED;
    }

   
    memset(emptyPageBuffer, 0, PAGE_SIZE);
    size_t writeResult = fwrite(emptyPageBuffer, sizeof(char), PAGE_SIZE, page) ;
    
    if ( writeResult< PAGE_SIZE) {
        const char *failedoutput = "Write failed";
        printf("%s\n", failedoutput); 
        free(emptyPageBuffer); 
        fclose(page); 
        return RC_WRITE_FAILED;
    } 
    else 
    {
        const char *writeoutput = "Write Succeeded";
        printf("%s\n", writeoutput);   
    }

    
    fclose(page);
    free(emptyPageBuffer);  

    return RC_OK; 
}

extern RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    
    page = fopen(fileName, "rb+");

    
    if (page == NULL) {
        fclose(page); 
        return RC_FILE_NOT_FOUND; 
    }

    
    fHandle->fileName = fileName; 
    fHandle->curPagePos = 0; 

    
    struct stat fileInfo;
    if (fstat(fileno(page), &fileInfo) < 0) {
        fclose(page); 
        return RC_ERROR; 
    }

    
    fHandle->totalNumPages = fileInfo.st_size / PAGE_SIZE;

    
    fclose(page);
    return RC_OK; 
}
extern RC closePageFile (SM_FileHandle *fHandle) {
    
    if(page != NULL){
        
        fHandle->mgmtInfo =NULL; 
        return RC_OK;
    }
    else
    {
        return RC_FILE_HANDLE_NOT_INIT;
    }
}


extern RC destroyPageFile (char *fileName) {
       
    page = fopen(fileName, "r");
    
    if (page == NULL) return RC_FILE_NOT_FOUND; 
    
    
    fclose(page);
    
    if (remove(fileName)==0)
        return RC_OK;
    else
        return RC_FILE_DELETE_FAILED;
}
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    
    if (pageNum < 0 || pageNum > fHandle->totalNumPages)
            return RC_READ_NON_EXISTING_PAGE;

    
    page = fopen(fHandle->fileName, "r");

    
    if (page == NULL) return RC_FILE_NOT_FOUND;
    
    
    if(fseek(page, (pageNum * PAGE_SIZE), SEEK_SET) != 0) {
        
        return RC_READ_NON_EXISTING_PAGE;
        
    } 
    else 
    {
        fread(memPage, sizeof(char), PAGE_SIZE, page); 
    }

    int curPos = ftell(page);   
    fHandle->curPagePos = curPos; 
              
    fclose(page);
    
    return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
    
    int newPosition = fHandle->curPagePos;
    return newPosition;
}

extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    FILE *page; 
    page = fopen(fHandle->fileName, "r");

    
    if (page == NULL) return RC_FILE_NOT_FOUND;

    int i = 0;
    int c;
    while (i < PAGE_SIZE && (c = fgetc(page)) != EOF) {
        memPage[i] = c;
        i++;
    }

    
    fHandle->curPagePos = ftell(page);    
    fclose(page);
    return RC_OK;
}


extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    
    if (fHandle->curPagePos <= PAGE_SIZE) {
        printf("\n First block: Previous block not available for the first block.");
        return RC_READ_NON_EXISTING_PAGE;
    } 
    else 
    {
        
        int fhnadelCurPageposPointer =fHandle->curPagePos;
        int currentPageNumber = fhnadelCurPageposPointer / PAGE_SIZE;
        int currPagePointer = currentPageNumber - 2;
        int startPosition = (PAGE_SIZE * currPagePointer);
   
        page = fopen(fHandle->fileName, "r");

        if (page == NULL) return RC_FILE_NOT_FOUND;

        
        fseek(page, startPosition, SEEK_SET);

        int i = 0;
        int c;
        
        while (i < PAGE_SIZE && (c = fgetc(page)) != EOF) {
            memPage[i] = c;
            i++;
        }

        int curPos = ftell(page);
        fHandle->curPagePos = curPos;
        
        fclose(page);
        return RC_OK;
    }
}


extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {

    int fhnadelCurPageposPointer =fHandle->curPagePos;
    int currentPageNumber = fhnadelCurPageposPointer / PAGE_SIZE;
    int currPagePointer = currentPageNumber - 2;
    int startPosition = (PAGE_SIZE * currPagePointer);

    page = fopen(fHandle->fileName, "r");
  
    if (page == NULL) return RC_FILE_NOT_FOUND;
    
    fseek(page, startPosition, SEEK_SET);

    int i = 0;
    int c;
    while (i < PAGE_SIZE) {
        c = fgetc(page);
        if (feof(page)) {
            break;
        }
        memPage[i] = c;
        i++;
    }


    int curPos = ftell(page);
    fHandle->curPagePos = curPos;

    fclose(page);
    return RC_OK;
}


extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {

    if (fHandle->curPagePos == PAGE_SIZE) {
        printf("\n Last block: No next block available for the last block");
        return RC_READ_NON_EXISTING_PAGE;
    } 
    else 
    {
    
        int fhnadelCurPageposPointer =fHandle->curPagePos;
        int currentPageNumber = fhnadelCurPageposPointer / PAGE_SIZE;
        int currPagePointer = currentPageNumber - 2;
        int startPosition = (PAGE_SIZE * currPagePointer);

        page = fopen(fHandle->fileName, "r");
        if (page == NULL) {
            return RC_FILE_NOT_FOUND;
        }
        fseek(page, startPosition, SEEK_SET);

        int i = 0;
        int c;
        while (i < PAGE_SIZE) {
            c = fgetc(page);
            if (feof(page)) {
                break;
            }
            memPage[i] = c;
            i++;
        }
        int curPos = ftell(page);
        fHandle->curPagePos = curPos;
        fclose(page);

        return RC_OK;
    }
}


extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    page = fopen(fHandle->fileName, "rb+");

    if (page == NULL) return RC_FILE_NOT_FOUND;
    int fHandleTotalpointer =fHandle->totalNumPages - 1;
    int startPosition = fHandleTotalpointer * PAGE_SIZE;
    fseek(page, startPosition, SEEK_SET);

    int i = 0;
    int c;
    while (i < PAGE_SIZE) {
        c = fgetc(page);
        if (feof(page)) {
            break;
        }
        memPage[i] = c;
        i++;
    }

    int curPos = ftell(page);
    fHandle->curPagePos = curPos;

    fclose(page);

    return RC_OK;
}


extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

    int fHandletotalNumPagesPointer = fHandle->totalNumPages;
	if (pageNum < 0 || pageNum > fHandletotalNumPagesPointer)
        	return RC_WRITE_FAILED;

	FILE *page = fopen(fHandle->fileName, "rb+");
	if (page == NULL) return RC_FILE_NOT_FOUND;

	int startPosition = pageNum * PAGE_SIZE;

	if(pageNum != 0) { 
		fHandle->curPagePos = startPosition;
		fclose(page);
		writeCurrentBlock(fHandle, memPage);
		
			
	} else {	
        fseek(page, startPosition, SEEK_SET);	
		//int i;
		int j = 0;
        while (j < PAGE_SIZE) 
        {
            if (feof(page)) {
                appendEmptyBlock(fHandle);
            }
            fputc(memPage[j], page);
            j++;
        }
		int curPos = ftell(page);
        fHandle->curPagePos = curPos; 
		fclose(page);
		
	}
	return RC_OK;
}

extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    FILE *page = fopen(fHandle->fileName, "rb+");
    if (page == NULL) return RC_FILE_NOT_FOUND;
    appendEmptyBlock(fHandle);

    fseek(page, fHandle->curPagePos, SEEK_SET);

    int i = 0;
    while (memPage[i] != '\0') {
        fputc(memPage[i], page);
        i++;
    }

    int curPos = ftell(page);
    fHandle->curPagePos = curPos;
    fclose(page);
    return RC_OK;
}



extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
    
    SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));

    
    if( fseek(page, 0, SEEK_END) != 0 ) {
        free(emptyBlock);
        return RC_WRITE_FAILED;
        
    } 
    if (fwrite(emptyBlock, sizeof(char), PAGE_SIZE, page) != PAGE_SIZE) {
        free(emptyBlock);
        return RC_WRITE_FAILED; 
    }
    
    free(emptyBlock);

    fHandle->totalNumPages++;
    return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {

    if(fopen(fHandle->fileName, "a") == NULL)
        return RC_FILE_NOT_FOUND;

    while(numberOfPages > fHandle->totalNumPages)
        appendEmptyBlock(fHandle);
    
    fclose(page);
    return RC_OK;
}