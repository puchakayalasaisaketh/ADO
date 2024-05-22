Team Members
	•	Rohith Kukkadapu -- 20%
	•	Sai Saketh Puchakayala -- 20%
	•	Maithreyee Doma -- 20%
	•	Rama Muni Reddy Bandi -- 20%
	•	Vinay Yerram -- 20%
Contribution 
•	Saketh – Page Replacement Strategies:Implementing the page replacement strategies in the project like FIFO, LFU, LRU, CLOCK and Ensure that all parts of the code integrate seamlessly. FIFO page replacement strategy, the buffer manager keeps track of the order in which pages were loaded into the buffer pool. LRU page replacement strategy, the buffer manager keeps track of which pages were accessed most recently.
•	Rohith – Buffer Pool Management:Implement the initBufferPool function as per the specifications. Handle buffer pool creation, initialization, and allocation. Implement the shutdownBufferPool function.Manage the shutdown of buffer pools and the release of associated  resources,  implemented  forceFlushPool , and also implemented markDirty which is used to mark Dirty Pages 
• –. Maithreyee -- Page Handling: Implement the pinPage, unpinPage and forcePage functions. Handle page pinning, unpinning, and forcing pages back to disk. pinPage method is used to request a page to be loaded into a page frame in the buffer pool. The forcePage method is used to write a specific page back to disk
•	Rama Muni Reddy – Statistics Functions:Implement the statistics functions getFrameContents, getDirtyFlags, getFixCounts, getNumReadIO, and getNumWriteIO.
Ensure that these functions return the correct statistics about the buffer pool. The getFrameContents function returns an array of PageNumbers that correspond to the page numbers stored in each page frame of the buffer pool.

•	Vinay -- Testing and Debugging: Work on creating test cases in the test_assign2_1.c file. Verify that the buffer manager functions work correctly, including both FIFO, LRU, LFU and COLCK strategies. Assist in debugging and resolving any issues that arise during testing. Continuously test your code as you implement it. Ensure that test cases cover various scenarios and edge cases.
RUNNING THE SCRIPT
1) Go to Project root (assign1) using Terminal.
2) Type "ls" to list the files and also to check that are we in the correct directory or not.
3) Type "make clean" to delete old compiled .o files.
4) Type "make" to compile all project files including the "test_assign2_1.c" file 
5) Type "make run_test1" to run the "test_assign2_2.c" file.
6) Type "make test2" to compile the Custom test file "test_assign2_2.c".
7) Type "make run_test2" to run the "test_assign2_2.c" file.

SOLUTION DESCRIPTION
===========================
We have taken great care to maintain efficient memory management in our storage manager by releasing any allocated memory when it is no longer needed. Additionally, we have incorporated four different page replacement algorithms: FIFO (First In First Out), LRU (Least Recently Used), LFU (Least Frequently Used), and CLOCK.
1. BUFFER POOL FUNCTIONS
===========================
The buffer pool-related functions are utilized to establish a buffer pool for an existing page file stored on disk. This buffer pool is constructed in memory alongside the page file, which remains on the disk. We rely on the Storage Manager (Assignment 1) to carry out operations on the page file located on the disk.
initBufferPool(...)
--> This function will be generating a fresh buffer pool in memory.
--> The "numPages" parameter determines the buffer's size, specifically, the number of page frames that can be accommodated within the buffer.
--> The variable "pageFile" holds the name of the page file for which pages are being cached in memory.
--> Strategy  represents the page replacement strategy (FIFO, LRU, LFU, CLOCK) that will be employed by this buffer pool.
--> stratData is used to pass parameters to the page replacement strategy, if applicable .  
shutdownBufferPool(...)
--> This function will be shutting down, or in other words, destroying the buffer pool.
--> It releases all resources and memory space utilized by the Buffer Manager for the buffer pool, effectively freeing them up.
--> Prior to dismantling the buffer pool, we invoke the "forceFlushPool(...)" function, which is responsible for writing all the modified pages (dirty pages) back to the disk.
--> If any page is currently in use by any client, the system will raise an error with the code "RC_PINNED_PAGES_IN_BUFFER." This error indicates that some pages are still pinned in the buffer and cannot be evicted until they are no longer in use.

forceFlushPool(...)
--> This function will be writing all the dirty pages, which are the modified pages marked with a "dirtyBit" equal to 1, back to the disk.
--> It analyses all the page frames within the buffer pool and verifies two conditions: 
            1. If the "dirtyBit" is equal to 1, indicating that the content of the page frame has been modified by some client.
            2. If the "fixCount" is equal to 0, signifying that no user is currently using that page frame.
            If both of these conditions are met, it proceeds to write the contents of the page frame to the page file on the disk.
2. PAGE MANAGEMENT FUNCTIONS
==========================
-->The page management-related functions serve the following purposes:
        1. Loading pages from disk into the buffer pool (pin pages).
        2. Removing a page frame from the buffer pool (unpin page).
        3. Marking a page as dirty (indicating it has been modified).
        4. Forcing a page frame to be written to the disk.
pinPage(...)
--> This function pins the page with the specified page number, "pageNum." It achieves this by reading the page from the page file located on disk and then storing it in the buffer pool.
--> Before pinning a page, the function checks if there is available space in the buffer pool. If there is sufficient empty space, it can store the page frame in the buffer pool directly. However, if the buffer pool is already full, it necessitates the use of a page replacement strategy to replace a page within the buffer pool to make room for the new page.
--> We have incorporated four page replacement strategies, namely FIFO, LRU, LFU, and CLOCK. These strategies are employed when pinning a page and making decisions about which page to replace in the buffer pool if necessary.
--> The page replacement algorithms are responsible for selecting which page needs to be replaced in the buffer pool. When a page is chosen for replacement, it undergoes a check to determine if it is marked as dirty (dirtyBit = 1). If the page is indeed dirty, the content of the page frame is written back to the page file on disk, and the new page is then stored in the same location where the old page was located in the buffer pool.
unpinPage(...)
--> This function unpins the specified page, and the decision on which page to unpin is based on the page's page number, "pageNum."
--> Upon finding the page through a loop, the function decrements the "fixCount" of that page by 1, indicating that the client is no longer using this page. This allows the system to track how many clients are currently using a particular page in the buffer pool.
makeDirty(...)
--> This function sets the "dirtyBit" of the specified page frame to 1, indicating that the contents of the page frame have been modified and need to be written back to the disk.
--> It locates the page frame using its "pageNum" by iteratively checking each page in the buffer pool. When the page is found, it sets the "dirtyBit" to 1 for that specific page, indicating that the page's content has been modified.
forcePage(....)
--> This function writes the content of the specified page frame to the page file that resides on the disk.
--> It finds the specified page using its "pageNum" by inspecting all the pages in the buffer pool using a loop construct.
--> Once the page is located, the function utilizes the Storage Manager functions to write the content of the page frame back to the page file on the disk. Following the successful write operation, it sets the "dirtyBit" to 0 for that particular page, indicating that the page is no longer marked as modified.
3. STATISTICS FUNCTIONS
===========================
The statistics-related functions serve the purpose of collecting and providing various statistical information about the buffer pool. These statistics offer insights and data about how the buffer pool is utilized and can aid in performance analysis and optimization.
getFrameContents(...)
--> This function returns an array of PageNumbers, where the size of the array is equal to the buffer size, denoted as "numPages."
--> To obtain the "pageNum" value of the page frames stored in the buffer pool, the function iterates through all the page frames within the buffer pool.
--> The "n"th element in the array corresponds to the page number of the page stored in the "n"th page frame of the buffer pool. In other words, the order of elements in the array matches the order of page frames in the buffer pool.
getDirtyFlags(...)
--> This function returns an array of booleans, where the size of the array is equal to the buffer size, which is represented as "numPages."
--> To obtain the "dirtyBit" value of the page frames stored in the buffer pool, the function iterates through all the page frames within the buffer pool.
--> The "n"th element in the array is set to TRUE if the page stored in the "n"th page frame of the buffer pool is marked as dirty. 
getFixCounts(...) 
--> This function provides an array of integers, with the size of the array matching the buffer size, which is represented as "numPages."
--> To obtain the "fixCount" value for each page frame in the buffer pool, the function iterates through all the page frames in the buffer pool.
--> The "n"th element in the array corresponds to the fixCount of the page stored in the "n"th page frame within the buffer pool.
getNumReadIO(...)
--> This function returns the count of the total number of I/O reads carried out by the buffer pool, which signifies the number of pages that have been read from the disk into the buffer pool.
--> We keep track of this data using the "rearIndex" variable.
getNumWriteIO(...)
--> This function provides the count of the total number of I/O writes executed by the buffer pool, representing the number of pages that have been written from the buffer pool back to the disk.
--> We maintain this data using the "writeCount" variable. It is initialized to 0 when the buffer pool is created, and we increment it each time a page frame is written to the disk. This helps us keep track of the total number of I/O writes performed.
4. PAGE REPLACEMENT ALGORITHM FUNCTIONS
=========================================
The page replacement strategy functions are responsible for implementing FIFO, LRU, LFU, and CLOCK algorithms, which come into play when a new page needs to be pinned but the buffer pool is already full. In such cases, these strategies determine which page should be replaced within the buffer pool to make room for the new page. Each of these strategies follows its own set of rules to decide which page to evict and replace with the incoming page.
FIFO(...)
--> Indeed, First In First Out (FIFO) is one of the most straightforward and fundamental page replacement strategies employed in memory management. It operates on the principle of evicting the oldest page that was brought into the buffer pool when a new page needs to be pinned.
--> After locating the page that needs to be replaced, the process typically involves two steps:
        1. Writing the content of the page frame to the page file on disk, which is necessary if the page is marked as dirty (modified).
        2. Adding the new page at the location of the old page within the buffer pool.
        These steps ensure that the changes made to the old page are saved to disk, and the new page is appropriately inserted into the buffer pool.
LFU(...)
--> Least Frequently Used (LFU) removes the page frame which is used the least times (lowest number of times) amongst the other page frames in the buffer pool.
--> The variable (field) refNum in each page frame serves this purpose. refNum keeps a count of of the page frames being accessed by the client.
--> So when we are using LFU, we just need to find the position of the page frame having the lowest value of refNum.
--> We then write the content of the page frame to the page file on disk and then add the new page at that location.
--> Also, we store the position of the least frequently used page frame in a variable "lfuPointer" so that is useful next time when we are replacing a page in the buffer pool. It reduces the number of iterations from 2nd page replacement onwards.
LRU(...)
--> Least Recently Used (LRU) removes the page frame which hasn't been used for a long time (least recent) amongst the other page frames in the buffer pool.
--> The variable (field) hitNum in each page frame serves this purpose. hitNum keeps a count of of the page frames being accessed and pinned by the client. Also a global variable "hit" is used for this purpose.
--> So when we are using LRU, we just need to find the position of the page frame having the lowest value of hitNum.
--> We then write the content of the page frame to the page file on disk and then add the new page at that location.
CLOCK(...)+
--> CLOCK algorithm keeps a track of the last added page frame in the buffer pool. Also, we use a clockPointer which is a counter to point the page frames in the buffer pool.
--> When a page has to be replaced we check the "clockPointer"s position. If that position's page's hitNum is not 1 (i.e. it wasn't the last page added), then replace that page with the new page.
--> In case, hitNum = 1, then we set it's hitNum = 0, increment clockPointer i.e. we go to the next page frame to check the same thing. This process goes on until we find a position to replace the page. We set hitNum = 0 so that we don't enter into an infinite loop.
 
TEST CASES 2
===============
We have added additional test cases in source file test_assign2_2.c. The instructions to run these test cases is mentioned in this README file. These test cases test LFU and CLOCK page replacement strategies.
