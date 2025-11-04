/* hw_config.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use 
this file except in compliance with the License. You may obtain a copy of the 
License at

   http://www.apache.org/licenses/LICENSE-2.0 
Unless required by applicable law or agreed to in writing, software distributed 
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR 
CONDITIONS OF ANY KIND, either express or implied. See the License for the 
specific language governing permissions and limitations under the License.
*/

#include <string.h>
#include "my_debug.h"
#include "hw_config.h"
#include "ff.h"
#include "diskio.h"

// Hardware Configuration of SPI "objects"
// Maker Pi Pico SD card - based on MicroPython example
static spi_t spis[] = {
    {
        .hw_inst = spi1,      // SPI1 for Maker Pi Pico SD card
        .miso_gpio = 12,      // GP12 - MISO
        .mosi_gpio = 11,      // GP11 - MOSI (corrected!)
        .sck_gpio = 10,       // GP10 - SCK (corrected!)
        .baud_rate = 12500 * 1000,  
    }
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {
    {
        .pcName = "0:",           // Name used to mount device
        .spi = &spis[0],          // Pointer to the SPI driving this card
        .ss_gpio = 15,            // GP15 - CS (corrected!)
        .use_card_detect = false, // No card detect
        .card_detect_gpio = -1,   
        .card_detected_true = -1  
    }
};

/* ********************************************************************** */
size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
    if (num <= spi_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */