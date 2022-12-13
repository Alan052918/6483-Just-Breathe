// Documents
// Dev board dev manual: https://www.st.com/resource/en/user_manual/um1670-discovery-kit-with-stm32f429zi-mcu-stmicroelectronics.pdf
// Accelerometer datasheet: https://www.mouser.com/datasheet/2/389/dm00168691-1798633.pdf
#include "mbed.h"

// BW_RATE configurations:
// D7 | D6 | D5 | D4               | D3 | D2 | D1 | D0
// ---+----+----+------------------+----+----+----+----
// reserved     | power mode       | data rate
//              | normal operation | 200 Hz (bandwidth 100 Hz)
#define BW_RATE 0x2C
#define BW_RATE_CONFIG 0b000'0'1011

// DATA_FORMAT configurations:
// D7        | D6         | D5             | D4       | D3              | D2              | D1 | D0
// ----------+------------+----------------+----------+-----------------+-----------------+----+----
// self test | SPI        | interrupt mode | reserved | resolution      | justified mode  | g range
// disabled  | 4-wire SPI | active high    |          | full resolution | right-justified | +/- 2 g
#define DATA_FORMAT 0x31
#define DATA_FORMAT_CONFIG 0b0'0'0'0'1'0'00

// address of the first output register
#define DATAX0 0x32

#define SPI_FLAG 1

SPI spi(PA_7, PA_6, PA_5, PA_4, use_gpio_ssel); // mosi, miso, sclk, ssel
EventFlags flags;

uint8_t write_buf[32];
uint8_t read_buf[32];

// /* Read DEVID register
DigitalOut cs(PC_1);

// The spi.transfer() function requires that the callback
// provided to it takes an int parameter
void spi_cb_devid(int event)
{
    // deselect the sensor
    cs = 1;
    flags.set(SPI_FLAG);
}

void read_devid()
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
        spi.transfer(write_buf, 2, read_buf, 2, spi_cb_devid, SPI_EVENT_COMPLETE);

        flags.wait_all(SPI_FLAG);
        int devid = read_buf[1]; // 0xE5 == 345
        printf("DEVID register = 0x%X\n", devid);

        thread_sleep_for(1'000);
    }
}
//*/

// /* Read x, y, z, axis data
void spi_cb_data(int event)
{
    flags.set(SPI_FLAG);
}

void read_data()
{
    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spi.format(8, 3);
    spi.frequency(1'000'000);

    write_buf[0] = BW_RATE;
    write_buf[1] = BW_RATE_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb_data, SPI_EVENT_COMPLETE);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = DATA_FORMAT;
    write_buf[1] = DATA_FORMAT_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb_data, SPI_EVENT_COMPLETE);
    flags.wait_all(SPI_FLAG);

    while (1) {
        int16_t raw_gx, raw_gy, raw_gz;
        float gx, gy, gz;

        // TODO: Understand what this does and fix it
        // prepare the write buffer to trigger a sequential read
        write_buf[0] = DATAX0 | 0x80 | 0x40;

        // start sequential sample reading
        spi.transfer(write_buf, 7, read_buf, 7, spi_cb_data, SPI_EVENT_COMPLETE);
        flags.wait_all(SPI_FLAG);

        raw_gx = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
        raw_gy = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
        raw_gz = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);

        printf("Raw|\tgx: %4.5d\tgy: %4.5d\tgz: %4.5d\n", raw_gx, raw_gy, raw_gz);

        // TODO: Add correct calculations
        gx = raw_gx;
        gy = raw_gy;
        gz = raw_gz;

        printf("Actual|\tgx: %4.5f\tgy: %4.5f\tgz: %4.5f\n", gx, gy, gz);

        thread_sleep_for(100);
    }
}
//*/

int main()
{
    read_devid();
    // read_data();
}
