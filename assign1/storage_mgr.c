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
	// Initialising file pointer i.e. storage manager.
	page = NULL;
}

extern RC createPageFile(char *fileName) {
    FILE *page = fopen(fileName, "w+"); // access mode is set to read and write i.e, "w+")

    if (page == NULL) {
        // returning rc_file_not_found if file was not opened
        return RC_FILE_NOT_FOUND;
    }

    // Creating an empty page in memory by allocating memory for it.
    SM_PageHandle emptyPageBuffer = (SM_PageHandle)malloc(PAGE_SIZE);

    if (emptyPageBuffer == NULL) {
        // Handle memory allocation failure.
        fclose(page); // Close the file before returning.
        return RC_MALLOC_FAILED;
    }

    // Initialize the empty page to zero.
    memset(emptyPageBuffer, 0, PAGE_SIZE);

    
    size_t writeResult = fwrite(emptyPageBuffer, sizeof(char), PAGE_SIZE, page) ;
	// Write the empty page to the file.

    if ( writeResult< PAGE_SIZE) {
        printf("Write failed\n"); //print an error message if write operation is failed
        free(emptyPageBuffer); // Free allocated memory.
        fclose(page); // Close the file.
        return RC_WRITE_FAILED;
    } else {
        printf("Write succeeded\n");  //prints success message 
    }

    // Close the page to make sure all the buffers are removed .
    fclose(page);
    free(emptyPageBuffer);  //deallocating the memory previously assigned to emptyPageBuffer

    return RC_OK; //this indicates file is successfully created
}

extern RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    // Attempt to open the file in read mode.
    page = fopen(fileName, "r");

    // Check if the file was not successfully opened.
    if (page == NULL) {
        fclose(page); // Close the file handle.
        return RC_FILE_NOT_FOUND; // Return error: File not found.
    }

    // Initialize the file handle values.
    fHandle->fileName = fileName; // Set the file name.
    fHandle->curPagePos = 0; // Set the current page position.

    // Retrieve file information using fstat.
    struct stat fileInfo;
    if (fstat(fileno(page), &fileInfo) < 0) {
        fclose(page); // Close the file handle.
        return RC_ERROR; // Return error: Unable to obtain file information.
    }

    // Calculate the total number of pages based on file size and page size.
    fHandle->totalNumPages = fileInfo.st_size / PAGE_SIZE;

    // Close the file to flush any internal buffers and release resources.
    fclose(page);
    return RC_OK; // Return success.
}
extern RC closePageFile (SM_FileHandle *fHandle) {
	// checking if page is null or not.
	if(page != NULL){
		//assign the fhandle pointer to null to indicate it is closed.
		fHandle->mgmtInfo =NULL; //mgmtInfo field is more modular and safer approcah because it does not rely on a global variable
		return RC_OK;
	}
	else{
		return RC_FILE_HANDLE_NOT_INIT;
	}
}


extern RC destroyPageFile (char *fileName) {
	// Opening in read mode!	
	page = fopen(fileName, "r");
	
	if(page == NULL)
		return RC_FILE_NOT_FOUND; 
	
	//closing the file stream
	fclose(page);
	//Remove the file from file System
	//remove(fileName);
	if (remove(fileName)==0)
		return RC_OK;
	else
		return RC_FILE_DELETE_FAILED;
}
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Checking if the pageNumber parameter is less than Total number of pages and less than 0, then return respective error code
	if (pageNum < 0 || pageNum > fHandle->totalNumPages)
        	return RC_READ_NON_EXISTING_PAGE;

	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.	
	page = fopen(fHandle->fileName, "r");

	// Checking if file was successfully opened.
	if(page == NULL)
		return RC_FILE_NOT_FOUND;
	
	// Adjusting the pointer position of the file stream. The seek operation is considered successful if fseek() returns 0; otherwise, return RC_WRITE_FAILED.
	// And the seek is success if fseek() return 0
	if(fseek(page, (pageNum * PAGE_SIZE), SEEK_SET) != 0) {
		// We're reading the content and storing it in the location pointed out by memPage.
		return RC_READ_NON_EXISTING_PAGE;
		
	} else {
		fread(memPage, sizeof(char), PAGE_SIZE, page); 
	}
    	
	// Updating the current page position to match the pointer position of the file stream
	fHandle->curPagePos = ftell(page); 
	
	// Closing file stream so that all the buffers are flushed.     	
	fclose(page);
	
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
	// we will get the current page position obtained from the file handle	
	return fHandle->curPagePos;
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// calling the readBlock function with pageNumber = 0, indicating the first block.	
	return readBlock(0, fHandle, memPage);
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

	// Determining the current page number by dividing the current page position by the page size.	
	int currPageNumber = fHandle->curPagePos / PAGE_SIZE;

	// Invoking the readBlock(...) function with pageNumber = currentPagePosition - 1, which corresponds to the previous block.
	return readBlock(currPageNumber - 1, fHandle, memPage);
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Deriving the current page number by dividing the current page position by the page size	
	int currPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	//calling readBlock function with pageNumber = currentPagePosition + 1, corresponding to the current block.
	return readBlock(currPageNumber, fHandle, memPage);
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	// Determining the current page number by dividing the page size by the current page position.	
	int currPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	printf("CURRENT PAGE POSITION = %d \n", fHandle->curPagePos);
	// calling the readBlock function with pageNumber = currentPagePosition + 1, corresponding to the last block.
	return readBlock(currPageNumber + 1, fHandle, memPage);
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	// calling to readBlock function with pageNumber = totalNumPages i.e. last block	
	return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Verifying whether the pageNumber parameter is both less than the total number of pages and less than 0; if so, return the corresponding error code.
	if (pageNum < 0 || pageNum > fHandle->totalNumPages)
        	return RC_WRITE_FAILED;
	
	// Opening the file stream in both read and write mode using 'r+' mode, which allows both reading and writing to the file.	
	page = fopen(fHandle->fileName, "r+");
	
	// Verifying if the file was opened successfully; otherwise, return the RC_FILE_NOT_FOUND error.
	if(page == NULL)
		return RC_FILE_NOT_FOUND;

	// Adjusting the pointer(fHandle) position of the file stream. The seek operation is considered successful if fseek() returns 0; otherwise, return RC_WRITE_FAILED.
	
	if(fseek(page, (pageNum * PAGE_SIZE), SEEK_SET) != 0) {
		return RC_WRITE_FAILED;	
	} 
	else {
		// Copying the content from memPage to the page stream.
		fwrite(memPage, sizeof(char), strlen(memPage), page);

		// Updating the current page position to match the pointer(fHandle) position of the file stream.
		fHandle->curPagePos = ftell(page);
		

		// Closing the file stream to ensure that all the buffers are flushed.   	
		fclose(page);
		
	}	
	
	return RC_OK;
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Determining the current page number by dividing the page size by the current page position.	
	int currPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	// Increasing the total number of pages because we are adding this content to a new location, specifically in the current empty block..
	fHandle->totalNumPages++;
	return writeBlock(currPageNumber, fHandle, memPage);
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	// Generating a blank page with a size of PAGE_SIZE bytes.
	SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	
	// Adjusting the pointer position to the beginning of the file stream.
	// The seek operation is considered successful if fseek() returns 0.

	
	if( fseek(page, 0, SEEK_END) != 0 ) {
		free(emptyBlock);
		return RC_WRITE_FAILED;
		
	} 
	if (fwrite(emptyBlock, sizeof(char), PAGE_SIZE, page) != PAGE_SIZE) {
        free(emptyBlock);
        return RC_WRITE_FAILED; 
    }
	// Freeing the memory that was previously allocated for 'emptyPage'.
	free(emptyBlock);
	
	// Increasing the total number of pages since we added an empty block.
	fHandle->totalNumPages++;
	return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	// Opening the file stream in append mode ('a' mode), which allows data to be appended to the end of the file.
	if(fopen(fHandle->fileName, "a") == NULL)
		return RC_FILE_NOT_FOUND;
	
	// Verifying if the value of numberOfPages is greater than the value of totalNumPages.
	// If that condition is met, then add empty pages until the value of numberOfPages becomes equal to the value of totalNumPages.
	while(numberOfPages > fHandle->totalNumPages)
		appendEmptyBlock(fHandle);
	
	// Closing the file stream to ensure that all buffers are flushed.
	fclose(page);
	return RC_OK;
}
