#include "mbed.h"
// Documents
// Dev board dev manual: https://www.st.com/resource/en/user_manual/um1670-discovery-kit-with-stm32f429zi-mcu-stmicroelectronics.pdf
// Accelerometer datasheet: https://www.mouser.com/datasheet/2/389/dm00168691-1798633.pdf

SPI spi(PA_7, PA_6, PA_5, PA_4, use_gpio_ssel); // mosi, miso, sclk, ssel
DigitalOut cs(PC_1);

uint8_t write_buf[32];
uint8_t read_buf[32];
EventFlags flags;
#define SPI_FLAG 1

// The spi.transfer() function requires that the callback
// provided to it takes an int parameter
void spi_cb(int event)
{
    // deselect the sensor
    cs = 1;
    flags.set(SPI_FLAG);
}

int main()
{
    // Chip must be deselected
    cs = 1;

    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spi.format(8, 3);
    spi.frequency(1'000'000);

    while (1) {
        // Send 0x80, the command to read the DEVID register
        // address of DEVID register: 0x00
        // 0x80 to indicate reading
        // 0x80 | 0x00 = 0x80 0b1000'0000
        write_buf[0] = 0x80;

        // Select the device by seting chip select low
        cs = 0;
        spi.transfer(write_buf, 2, read_buf, 2, spi_cb, SPI_EVENT_COMPLETE);

        flags.wait_all(SPI_FLAG);
        int devid = read_buf[1];
        printf("DEVID register = 0x%X\n", devid);

        thread_sleep_for(1'000);
    }
}
