#include <stdio.h>
#include "pico/stdlib.h"
#include "functions.h"

// GPIO pin numbers for buttons
#define BTN_GP20 20
#define BTN_GP21 21
#define BTN_GP22 22

// Global SD card manager instance
SD_Manager sd_mgr;

int main() {
    // Initialize standard I/O for serial communication
    stdio_init_all();
    sleep_ms(2000);  // Wait for USB serial connection
    
    printf("\n\n");
    printf("==================================== \n ");
    printf("  Pico SD Card Read/Write Demo\n");
    printf("====================================\n\n");
    
    // Initialize buttons
    gpio_init(BTN_GP20);
    gpio_set_dir(BTN_GP20, GPIO_IN);
    gpio_pull_up(BTN_GP20);
    
    gpio_init(BTN_GP21);
    gpio_set_dir(BTN_GP21, GPIO_IN);
    gpio_pull_up(BTN_GP21);
    
    gpio_init(BTN_GP22);
    gpio_set_dir(BTN_GP22, GPIO_IN);
    gpio_pull_up(BTN_GP22);
    
    printf("Buttons initialized (GP20, GP21, GP22)\n");
    
    // Initialize SD card
    if (!sd_init(&sd_mgr)) {
        printf("FATAL: SD card initialization failed!\n");
        printf("Please check connections and restart.\n");
        while (1) {
            sleep_ms(1000);
        }
    }
    
    printf("SD card initialized successfully!\n\n");
    
    // Read and display read.txt from SD card
    sd_read_file(&sd_mgr, "read.txt");
    
    // Clear write.txt on boot (overwrite with empty/initial content)
    printf("\nClearing write.txt on boot...\n");
    sd_write_data(&sd_mgr, "write.txt", "", false);  // false = overwrite
    
    printf("\n====================================\n");
    printf("Press GP20, GP21, or GP22 to write\n");
    printf("Each press appends to write.txt\n");
    printf("====================================\n\n");
    
    // Button state tracking for debouncing
    bool last_btn_20 = true;  // true = not pressed (pull-up)
    bool last_btn_21 = true;
    bool last_btn_22 = true;
    
    // Main loop
    while (true) {
        bool btn_20 = gpio_get(BTN_GP20);
        bool btn_21 = gpio_get(BTN_GP21);
        bool btn_22 = gpio_get(BTN_GP22);
        
        // Check Button GP20 (detect falling edge - button press)
        if (!btn_20 && last_btn_20) {
            printf("\n[Button GP20 Pressed]\n");
            sd_write_data(&sd_mgr, "write.txt", "Button GP20 pressed\n", true);  // true = append
            sleep_ms(200);  // Debounce delay
        }
        
        // Check Button GP21
        if (!btn_21 && last_btn_21) {
            printf("\n[Button GP21 Pressed]\n");
            sd_write_data(&sd_mgr, "write.txt", "Button GP21 pressed\n", true);  // true = append
            sleep_ms(200);  // Debounce delay
        }
        
        // Check Button GP22
        if (!btn_22 && last_btn_22) {
            printf("\n[Button GP22 Pressed]\n");
            sd_write_data(&sd_mgr, "write.txt", "Button GP22 pressed\n", true);  // true = append
            sleep_ms(200);  // Debounce delay
        }
        
        last_btn_20 = btn_20;
        last_btn_21 = btn_21;
        last_btn_22 = btn_22;
        
        sleep_ms(10);
    }
    
    return 0;
}