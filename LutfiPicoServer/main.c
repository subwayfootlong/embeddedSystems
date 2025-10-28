#include "server_driver.h"
#include "pico/stdlib.h"

int main() {
    // Initialize server components
    if (server_init() != 0) {
        return -1;
    }

    // Main processing loop
    printf("\nStarting main processing loop...\n");
    while (true) {
        // Keep the connection alive and process any pending messages
        sleep_ms(5000);
    }
   
    // Cleanup (will never reach here in this example)
    return 0;
}