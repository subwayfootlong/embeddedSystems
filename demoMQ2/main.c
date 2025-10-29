#include <stdio.h>
#include "pico/stdlib.h"
#include "mq2_driver.h"

int main()
{
    stdio_init_all();
    
    if (mq2_start() != MQ2_OK) {
        return -1;
    }

    mq2_loop();

    return 0;
}