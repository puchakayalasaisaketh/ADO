#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>

#include "storage_mgr.h"

FILE *pageFile = NULL;

extern void initStorageManager (void) {
	// Initialising file pointer i.e. storage manager.
	pageFile = NULL;
}

extern RC createPageFile(char *fileName) {
    FILE *pageFile = fopen(fileName, "w+"); // Open file in write and read mode ('w+')

    if (pageFile == NULL) {
        // Handle file opening failure.
        return RC_FILE_NOT_FOUND;
    }

    // Creating an empty page in memory.
    SM_PageHandle emptyPage = (SM_PageHandle)malloc(PAGE_SIZE);

    if (emptyPage == NULL) {
        // Handle memory allocation failure.
        fclose(pageFile); // Close the file before returning.
        return RC_MALLOC_FAILED;
    }

    // Initialize the empty page with zeroes.
    memset(emptyPage, 0, PAGE_SIZE);

    // Write the empty page to the file.
    size_t writeResult = fwrite(emptyPage, sizeof(char), PAGE_SIZE, pageFile);

    if (writeResult < PAGE_SIZE) {
        printf("Write failed\n");S
        free(emptyPage); // Free allocated memory.
        fclose(pageFile); // Close the file.
        return RC_WRITE_FAILED;
    } else {
        printf("Write succeeded\n");
    }

    // Close the file and free allocated memory.
    fclose(pageFile);
    free(emptyPage);

    return RC_OK;
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.
	pageFile = fopen(fileName, "r");

	// Checking if file was successfully opened.
	if(pageFile == NULL) {
		return RC_FILE_NOT_FOUND;
	} else { 
		// Updating file handle's filename and set the current position to the start of the page.
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;

		/* In order to calculate the total size, we perform following steps -
		   1. Move the position of the file stream to the end of file
		   2. Check the file end position
		   3. Move the position of the file stream to the beginning of file  
		
		fseek(pageFile, 0L, SEEK_END);
		int totalSize = ftell(pageFile);
		fseek(pageFile, 0L, SEEK_SET);
		fHandle->totalNumPages = totalSize/ PAGE_SIZE;  */
		
		/* Using fstat() to get the file total size.
		   fstat() is a system call that is used to determine information about a file based on its file descriptor.
		   'st_size' member variable of the 'stat' structure gives the total size of the file in bytes.
		*/

		struct stat fileInfo;
		if(fstat(fileno(pageFile), &fileInfo) < 0)    
			return RC_ERROR;
		fHandle->totalNumPages = fileInfo.st_size/ PAGE_SIZE;

		// Closing file stream so that all the buffers are flushed. 
		fclose(pageFile);
		return RC_OK;
	}
}

extern RC closePageFile (SM_FileHandle *fHandle) {
	// Checking if file pointer or the storage manager is intialised. If initialised, then close.
	if(pageFile != NULL)
		pageFile = NULL;	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName) {
	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.	
	pageFile = fopen(fileName, "r");
	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND; 
	
	// Deleting the given filename so that it is no longer accessible.	
	remove(fileName);
	return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Checking if the pageNumber parameter is less than Total number of pages and less than 0, then return respective error code
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_READ_NON_EXISTING_PAGE;

	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.	
	pageFile = fopen(fHandle->fileName, "r");

	// Checking if file was successfully opened.
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	// Setting the cursor(pointer) position of the file stream. Position is calculated by Page Number x Page Size
	// And the seek is success if fseek() return 0
	int isSeekSuccess = fseek(pageFile, (pageNum * PAGE_SIZE), SEEK_SET);
	if(isSeekSuccess == 0) {
		// We're reading the content and storing it in the location pointed out by memPage.
		fread(memPage, sizeof(char), PAGE_SIZE, pageFile);
	} else {
		return RC_READ_NON_EXISTING_PAGE; 
	}
    	
	// Setting the current page position to the cursor(pointer) position of the file stream
	fHandle->curPagePos = ftell(pageFile); 
	
	// Closing file stream so that all the buffers are flushed.     	
	fclose(pageFile);
	
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
	// Returning the current page position retrieved from the file handle	
	return fHandle->curPagePos;
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Re-directing (passing) to readBlock(...) function with pageNumber = 0 i.e. first block	
	return readBlock(0, fHandle, memPage);
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	//printf("CURRENT PAGE POSITION = %d \n", fHandle->curPagePos);
	//printf("TOTAL PAGES = %d \n", fHandle->totalNumPages);

	// Calculating current page number by dividing page size by current page position	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;

	// Re-directing (passing) to readBlock(...) function with pageNumber = currentPagePosition - 1 i.e. previous block
	return readBlock(currentPageNumber - 1, fHandle, memPage);
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Calculating current page number by dividing page size by current page position	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	// Re-directing (passing) to readBlock(...) function with pageNumber = currentPagePosition i.e. current block
	return readBlock(currentPageNumber, fHandle, memPage);
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	// Calculating current page number by dividing page size by current page position	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	printf("CURRENT PAGE POSITION = %d \n", fHandle->curPagePos);
	// Re-directing (passing) to readBlock(...) function with pageNumber = currentPagePosition + 1 i.e. last block
	return readBlock(currentPageNumber + 1, fHandle, memPage);
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	// Re-directing (passing) to readBlock(...) function with pageNumber = totalNumPages i.e. last block
	// printf("TOTAL PAGES = %d \n", fHandle->totalNumPages);	
	return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Checking if the pageNumber parameter is less than Total number of pages and less than 0, then return respective error code
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_WRITE_FAILED;
	
	// Opening file stream in read & write mode. 'r+' mode opens the file for both reading and writing.	
	pageFile = fopen(fHandle->fileName, "r+");
	
	// Checking if file was successfully opened.
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;

	// Setting the cursor(pointer) position of the file stream. The seek is successfull if fseek() return 0
	int isSeekSuccess = fseek(pageFile, (pageNum * PAGE_SIZE), SEEK_SET);
	if(isSeekSuccess == 0) {
		/*int k;
		for(k = 0; k < PAGE_SIZE; k++) {
			// Check after each iteration, if file is ending.
			if(feof(pageFile)) {
				// If we reached end of file, append an empty block at the end of file.
				appendEmptyBlock(fHandle);
			}
			// Writing content from memPage to pageFile stream
			fputc(memPage[k], pageFile);
		}*/

		// Writing content from memPage to pageFile stream
		fwrite(memPage, sizeof(char), strlen(memPage), pageFile);

		// Setting the current page position to the cursor(pointer) position of the file stream
		fHandle->curPagePos = ftell(pageFile);
		

		// Closing file stream so that all the buffers are flushed.     	
		fclose(pageFile);
	} else {
		return RC_WRITE_FAILED;
	}	
	
	return RC_OK;
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Calculating current page number by dividing page size by current page position	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	// Incrementing total number of pages since we are adding this content to a new location as in current empty block.
	fHandle->totalNumPages++;
	return writeBlock(currentPageNumber, fHandle, memPage);
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	// Creating an empty page of size PAGE_SIZE bytes
	SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	
	// Moving the cursor (pointer) position to the begining of the file stream.
	// And the seek is success if fseek() return 0
	int isSeekSuccess = fseek(pageFile, 0, SEEK_END);
	
	if( isSeekSuccess == 0 ) {
		// Writing an empty page to the file
		fwrite(emptyBlock, sizeof(char), PAGE_SIZE, pageFile);
	} else {
		free(emptyBlock);
		return RC_WRITE_FAILED;
	}
	
	// De-allocating the memory previously allocated to 'emptyPage'.
	// This is optional but always better to do for proper memory management.
	free(emptyBlock);
	
	// Incrementing the total number of pages since we added an empty black.
	fHandle->totalNumPages++;
	return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	// Opening file stream in append mode. 'a' mode opens the file to append the data at the end of file.
	pageFile = fopen(fHandle->fileName, "a");
	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	// Checking if numberOfPages is greater than totalNumPages.
	// If that is the case, then add empty pages till numberofPages = totalNumPages
	while(numberOfPages > fHandle->totalNumPages)
		appendEmptyBlock(fHandle);
	
	// Closing file stream so that all the buffers are flushed. 
	fclose(pageFile);
	return RC_OK;
}
