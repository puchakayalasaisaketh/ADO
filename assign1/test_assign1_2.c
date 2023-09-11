#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// Test output file
#define TESTPF "test_pagefile.bin"

// Function prototypes
static void testSinglePageReadWrite(void);
static void cleanup(SM_FileHandle *fh, SM_PageHandle ph);

int main(void) {
    initStorageManager();
    testSinglePageReadWrite();
    return 0;
}

static void cleanup(SM_FileHandle *fh, SM_PageHandle ph) {
    free(ph);
    closePageFile(fh);
    destroyPageFile(TESTPF);
}

static void testSinglePageReadWrite(void) {
    char *testName = "Test Single Page Read/Write";
    SM_FileHandle fh;
    SM_PageHandle ph;
    int i;

    ph = (SM_PageHandle)malloc(PAGE_SIZE);

    if (ph == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    // Create a new page file
    if (createPageFile(TESTPF) != RC_OK) {
        printf("Failed to create page file.\n");
        cleanup(&fh, ph);
        return;
    }

    // Open the page file
    if (openPageFile(TESTPF, &fh) != RC_OK) {
        printf("Failed to open page file.\n");
        cleanup(&fh, ph);
        return;
    }

    // Read the first page into the handle
    if (readFirstBlock(&fh, ph) != RC_OK) {
        printf("Failed to read the first block.\n");
        cleanup(&fh, ph);
        return;
    }

    // Ensure the page is empty (zero bytes)
    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == 0), "Expected zero byte in the first page of a freshly initialized page");
    }

    printf("The first block was empty\n");

    // Fill the page with a pattern and write it to disk
    for (i = 0; i < PAGE_SIZE; i++) {
        ph[i] = (i % 10) + '0';
    }

    // Write the string ph to various blocks of the file
    if (writeBlock(0, &fh, ph) != RC_OK ||
        writeCurrentBlock(&fh, ph) != RC_OK ||
        writeBlock(1, &fh, ph) != RC_OK ||
        writeCurrentBlock(&fh, ph) != RC_OK ||
        writeBlock(3, &fh, ph) != RC_OK) {
        printf("Failed to write blocks.\n");
        cleanup(&fh, ph);
        return;
    }

    // Read and verify the content of the written blocks
    if (readFirstBlock(&fh, ph) != RC_OK) {
        printf("Failed to read the first block.\n");
        cleanup(&fh, ph);
        return;
    }

    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "Character in the page read from disk is the one we expected.");
    }

    if (readPreviousBlock(&fh, ph) != RC_OK) {
        printf("Failed to read the previous block.\n");
        cleanup(&fh, ph);
        return;
    }

    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "Character in the page read from disk is the one we expected.");
    }

    if (readNextBlock(&fh, ph) != RC_OK) {
        printf("Failed to read the next block.\n");
        cleanup(&fh, ph);
        return;
    }

    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "Character in the page read from disk is the one we expected.");
    }

    if (readCurrentBlock(&fh, ph) != RC_OK) {
        printf("Failed to read the current block.\n");
        cleanup(&fh, ph);
        return;
    }

    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "Character in the page read from disk is the one we expected.");
    }

    if (readBlock(2, &fh, ph) != RC_OK) {
        printf("Failed to read the specified block.\n");
        cleanup(&fh, ph);
        return;
    }

    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "Character in the page read from disk is the one we expected.");
    }

    if (readLastBlock(&fh, ph) != RC_OK) {
        printf("Failed to read the last block.\n");
        cleanup(&fh, ph);
        return;
    }

    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "Character in the page read from disk is the one we expected.");
    }

    // Test ensureCapacity function
    if (ensureCapacity(6, &fh) != RC_OK) {
        printf("Failed to ensure capacity.\n");
        cleanup(&fh, ph);
        return;
    }

    cleanup(&fh, ph);
    printf("%s completed successfully.\n", testName);
    TEST_DONE();
}