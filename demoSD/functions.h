#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>
#include "ff.h"
#include "hw_config.h"

// SD Card Manager structure
typedef struct {
    FATFS fs;           // FatFS file system object
    FIL fil;            // File object
    bool mounted;       // SD card mount status
} SD_Manager;

/**
 * Initialize the SD card
 * Returns true on success, false on failure
 */
bool sd_init(SD_Manager *sd);

/**
 * Read and print a file from the SD card
 * filename: name of file to read (e.g., "read.txt")
 * Returns true on success, false on failure
 */
bool sd_read_file(SD_Manager *sd, const char *filename);

/**
 * Write data to a file on the SD card
 * filename: name of file to write to (e.g., "write.txt")
 * data: string data to write
 * append: if true, append to file; if false, overwrite file
 * Returns true on success, false on failure
 */
bool sd_write_data(SD_Manager *sd, const char *filename, const char *data, bool append);

/**
 * Unmount the SD card
 */
void sd_unmount(SD_Manager *sd);

#endif // FUNCTIONS_H