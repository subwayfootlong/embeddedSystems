#include "sd_driver.h"
#include <stdio.h>
#include <string.h>

// Initialize the SD card
bool sd_init(SD_Manager *sd) {
    FRESULT fr;
    
    printf("Initializing SD card...\n");
    
    // Initialize structure
    sd->mounted = false;
    
    // Mount the SD card
    fr = f_mount(&sd->fs, "", 1);
    if (fr != FR_OK) {
        printf("ERROR: Failed to mount SD card (error code: %d)\n", fr);
        printf("Possible causes:\n");
        printf("  - SD card not inserted\n");
        printf("  - SD card not formatted (use FAT32)\n");
        printf("  - Wiring issue\n");
        return false;
    }
    
    sd->mounted = true;
    printf("SD card mounted successfully!\n");
    
    return true;
}

// Read and print a file from the SD card
bool sd_read_file(SD_Manager *sd, const char *filename) {
    FRESULT fr;
    char buffer[256];
    
    if (!sd->mounted) {
        printf("ERROR: SD card not mounted!\n");
        return false;
    }
    
    printf("\n=== Reading file: %s ===\n", filename);
    
    // Open file for reading
    fr = f_open(&sd->fil, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file '%s' (error code: %d)\n", filename, fr);
        printf("Make sure the file exists on your SD card!\n");
        return false;
    }
    
    printf("File opened successfully!\n");
    printf("File size: %llu bytes\n\n", f_size(&sd->fil));
    
    printf("--- File Contents ---\n");
    
    // Read file line by line
    while (f_gets(buffer, sizeof(buffer), &sd->fil)) {
        printf("%s", buffer);
    }
    
    printf("\n--- End of File ---\n");
    
    // Close the file
    f_close(&sd->fil);
    
    return true;
}

// Write data to a file on the SD card
bool sd_write_data(SD_Manager *sd, const char *filename, const char *data, bool append) {
    FRESULT fr;
    UINT bytes_written;
    
    if (!sd->mounted) {
        printf("ERROR: SD card not mounted!\n");
        return false;
    }
    
    printf("Writing to file: %s\n", filename);
    
    // Open file for writing
    if (append) {
        // Append mode - add to end of file
        fr = f_open(&sd->fil, filename, FA_WRITE | FA_OPEN_APPEND);
    } else {
        // Overwrite mode - create new or truncate existing
        fr = f_open(&sd->fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    }
    
    if (fr != FR_OK) {
        printf("ERROR: Could not open file for writing (error code: %d)\n", fr);
        return false;
    }
    
    // Write data to file
    fr = f_write(&sd->fil, data, strlen(data), &bytes_written);
    if (fr != FR_OK) {
        printf("ERROR: Write failed (error code: %d)\n", fr);
        f_close(&sd->fil);
        return false;
    }
    
    if (strlen(data) > 0) {
        printf("Successfully wrote %d bytes: %s", bytes_written, data);
    } else {
        printf("File cleared successfully\n");
    }
    
    // Close the file
    f_close(&sd->fil);
    
    return true;
}

// Unmount the SD card
void sd_unmount(SD_Manager *sd) {
    if (sd->mounted) {
        f_unmount("");
        sd->mounted = false;
        printf("SD card unmounted.\n");
    }
}