#include <assert.h>
#include <string.h>
//
#include "hw_config.h"
//
#include "ff.h" /* Obtains integer types */
//
#include "diskio.h" /* Declarations of disk functions */

// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
static spi_t spis[] = {  // One for each SPI.
    {
        .hw_inst = spi0,  // SPI component
        .miso_gpio = 0,  // GPIO number (not Pico pin number)
        .mosi_gpio = 3,
        .sck_gpio = 2,

        // .baud_rate = 1000 * 1000
        .baud_rate = 12500 * 1000
        // .baud_rate = 25 * 1000 * 1000 // Actual frequency: 20833333.
    }};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {
        .pcName = "0:",   // Name used to mount device
        .spi = &spis[0],  // Pointer to the SPI driving this card
        .ss_gpio = 1,    // The SPI slave select GPIO for this SD card
        .use_card_detect = false,
        .card_detect_gpio = 22,  // Card detect
        .card_detected_true = 1  // What the GPIO read returns when a card is
                                 // present.
    }};

/* ********************************************************************** */
size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
    assert(num <= sd_get_num());
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
    assert(num <= spi_get_num());
    if (num <= spi_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */
