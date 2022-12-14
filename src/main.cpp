// Documents
// Dev board dev manual: https://www.st.com/resource/en/user_manual/um1670-discovery-kit-with-stm32f429zi-mcu-stmicroelectronics.pdf
// Accelerometer datasheet: https://www.mouser.com/datasheet/2/389/dm00168691-1798633.pdf
#include "mbed.h"

#include <ADXL345_I2C.h>

// BW_RATE configurations:
// D7 | D6 | D5 | D4               | D3 | D2 | D1 | D0
// ---+----+----+------------------+----+----+----+----
// reserved     | power mode       | data rate
//              | normal operation | 200 Hz (bandwidth 100 Hz)
#define BW_RATE 0x2C
#define BW_RATE_CONFIG 0b000'0'1011

// POWER_CTL configurations:
// D7 | D6  | D5                             | D4         | D3      | D2       | D1 | D0
// ---+-----+--------------------------------+------------+---------+----------+----+----
// reserved | link                           | auto sleep | measure | sleep    | wake up
//          | concurrent inactivity/activity | disabled   | enabled | disabled | 8 Hz of readings in sleep mode
#define POWER_CTL 0x2D
#define POWER_CTL_CONFIG 0b00'0'0'1'0'00

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
// The spi.transfer() function requires that the callback
// provided to it takes an int parameter
void spi_cb_devid(int event)
{
    flags.set(SPI_FLAG);
}

void read_devid()
{
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

void read_data_spi()
{
    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spi.format(8, 3);
    spi.frequency(1'000'000);

    // configure BW_RATE
    write_buf[0] = BW_RATE;
    write_buf[1] = BW_RATE_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb_data, SPI_EVENT_COMPLETE);
    flags.wait_all(SPI_FLAG);

    // configure POWER_CTL
    write_buf[0] = POWER_CTL;
    write_buf[1] = POWER_CTL_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb_data, SPI_EVENT_COMPLETE);
    flags.wait_all(SPI_FLAG);

    // configure DATA_FORMAT
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

        printf("Raw|\tgx: %d\tgy: %d\tgz: %d\n", raw_gx, raw_gy, raw_gz);

        // TODO: Add correct calculations
        gx = ((float)raw_gx) * (17.5f * 0.017453292519943295769236907684886f / 1000.0f);
        gy = ((float)raw_gy) * (17.5f * 0.017453292519943295769236907684886f / 1000.0f);
        gz = ((float)raw_gz) * (17.5f * 0.017453292519943295769236907684886f / 1000.0f);

        printf("Actual|\tgx: %4.5f\tgy: %4.5f\tgz: %4.5f\n", gx, gy, gz);

        thread_sleep_for(100);
    }
}
//*/

void read_data_github()
{
    // ADXL_InitTypeDef adxl = {
    //     0, // SPIMode
    //     0, // IntMode
    //     0, // LPMode
    //     0b1011, // Rate 200 Hz
    //     RANGE_2G, // 2 g range
    //     RESOLUTION_FULL, // Full resolution
    //     0, // right-justified
    //     0, // auto sleep disabled
    //     0, // link bit disabled
    // };

    // ADXL_Init(&adxl);
    // ADXL_Measure(ON);

    // float data[3];
    // void* raw_data = data;
    // ADXL_getAccel(raw_data, OUTPUT_FLOAT);

    // // float* data = (float*)raw_data;

    // printf("Raw|\tgx: %4.5f\tgy: %4.5f\tgz: %4.5f\n", data[0], data[1], data[2]);
}

void read_data_adxllib()
{
}

int main()
{
    // read_devid();
    // read_data_spi();
    // read_data_github();
    read_data_adxllib();
}
