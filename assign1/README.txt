Team Members
	•	Rohith Kukkadapu -- 20%
	•	Sai Saketh Puchakayala -- 20%
	•	Vinay Yerram -- 20%
	•	Rama Muni Reddy Bandi -- 20%
	•	Maithreyee Doma -- 20%
Contribution 
	•	Rohith - Project Management, File Management Functions - CreatePageFile, openPageFile, closePageFile, and destroyPageFile , Handling file I/O operations, error checking, and update the file handle, Error Handling .

	•	Saketh – Read Methods Implementation- read methods, including readBlock, getBlockPos, readFirstBlock, readLastBlock, readPreviousBlock, readCurrentBlock, and readNextBlock , Reading blocks from disk into memory , Error Handling .

	•	Vinay – Write Methods Implementation - writeBlock, writeCurrentBlock, appendEmptyBlock, and ensureCapacity , Writing blocks from memory to disk, Error Handling .

	•	Rama Muni Reddy – Testing and Documentation – Test Case Creation and Documentation , Written README.txt file, explaining the code structure and important implementation details.

	•	Maithreyee – Integration Testing – Done testing and documentation  to ensure that all methods are thoroughly tested and perform as expected, and also done additional coding work with coding tasks as needed.



RUNNING THE SCRIPT

1) Go to Project root (assign1) using Terminal.

2) Type "ls" to list the files and also to check that are we in the correct directory or not.

3) Type "make clean" to delete old compiled .o files.

4) Type "make" to compile all project files including the "test_assign1_1.c" file 

5) Type "make run_test1" to run the "test_assign1_1.c" file.

6) Type "make test2" to compile the Custom test file "test_assign1_2.c".

7) Type "make run_test2" to run the "test_assign1_2.c" file.



SOLUTION DESCRIPTION
==========================

This assignment aims to develop a straightforward storage manager, functioning as a module with the capacity to retrieve data blocks from a disk file and transfer data blocks from memory to the same disk file. The primary objective is to create an efficient and reliable mechanism for handling data storage operations, facilitating seamless data exchange between memory and persistent storage on disk.

1. FILE RELATED FUNCTIONS
===========================
The file-related functions manage files i.e. create, open and close files.
Once the file is created, we can use those files to perform read and write methods for the storage manager.
initStorageManager()
--> This function initializes the file pointer i.e. File Stream.  We reference the file stream object is set to be NULL in this method.

createPageFile(...)
--> This function generates a page file with the filename specified in the parameter.
--> We used the fopen() C function to Open a created file. We use 'w+' mode which opens it for reading and writing.
--> We return RC_FILE_NOT_FOUND if the file cannot be created and RC_OK if everything goes well.

openPageFile(...)
--> We used the fopen() C function to open the file and 'r' mode to open the file in read-only mode.
--> Also, we initialise struct FileHandle's curPagePos and totalPageNum.
--> We use the Linux fstat() C function which gives various information regarding the file. We retrieve the total information of the file and also the size of the file in bats using fstat() function.
--> We return RC_FILE_NOT_FOUND if the file cannot be opened and RC_OK if everything goes well.

closePageFile(...)
--> We set the file stream pointer to NULL.
-à We assigned the fhandle pointer to NULL to indicate the file is closed
-à We return RC_OK if the file is closed. If the file is not able to close then we return RC_FILE_HANDLE_NOT_INIT.

destroyPageFile:
--> We check if the file is present in memory and if present, we use the remove() C function to remove it from memory.
-We return RC_FILE_DELETE_FAILED if the file cannot be removed from the memory and RC_OK if everything goes well.

2. READ RELATED FUNCTIONS
==========================
The read-related functions are used to read blocks of data from the file pointer into the disk (memory). Also, it helps to navigate through the blocks easily.
C functions - fseek(..) and fread(..) are used.

readBlock(...)
--> We check whether the page number is valid or not. i.e. Page number is between 0 and the total number of pages and we return RC_READ_NON_EXISTING_PAGE if the page number is invalid.
--> Check if the pointer to the file stream is available by calculating the position of the page 
--> Using the valid file pointer we navigate to the given location using fseek()
--> If fseek() is successful, we read the data from the page number specified in the parameter and store it in the memPage.

getBlockPos(...)
--> This function returns the current page position which is retrieved from FileHandle's curPagePos.

readFirstBlock(...)
--> We call the readBlock(...) function by providing the pageNum argument as 0

readPreviousBlock(....)
--> We call the readBlock(...) function by providing the pageNum argument as (current page position - 1)
-à pageNum is calculated by dividing page size and Current page position

readCurrentBlock:
--> We call the readBlock(...) function by providing the pageNum argument as (current page position)
readNextBlock:
--> We call the readBlock(...) function by providing the pageNum argument as (current page position + 1)
readLastBlock:
--> We call the readBlock(...) function by providing the pageNum argument as (total number of pages - 1)

3. WRITE RELATED FUNCTIONS
===========================
The write-related functions are used to write blocks of data from the disk (memory) to the page file.
C functions - fseek(..) and fwrite(..) are used.

writeBlock(...)
--> We check whether the page number is valid or not i.e. Page number is the between 0 and the total number of pages and we return RC_WRITE_FAILED if the page number is invalid.
-à Opens the file in reading and writing mode by using "r+" mode.
--> Check if the pointer to the file stream is available.
--> Using the valid file pointer we navigate to the given location using fseek()
--> If fseek() is successful, we write the data to the appropriate location using fwrite() C function and store it into the memPage passed in the parameter.

writeCurrentBlock(...)
--> We call writeBlock(...) function with pageNum = current page position as the paramter.

appendEmptyBlock(...) 
--> We create an empty block having size = PAGE_SIZE
--> We move the cursor(pointer)  of the file stream to the last page.
--> Write the empty block data and update the total number of pages by 1 since we just added a new page

ensureCapacity(...)
--> Check number of pages required is greater than the total number of pages i.e. we require more pages.
--> Calculate the number of pages required and add that number of empty blocks.
--> Empty blocks are added using the appendEmptyBlock(...) function
 
TEST CASES 2
===============
We have added additional test cases in the source file test_assign_2.c. The instructions to run these test cases are mentioned in this README file.
