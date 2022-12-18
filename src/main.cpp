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
// disabled  | 4-wire SPI | active high    |          | full resolution | right-justified | +/-2 g
#define DATA_FORMAT 0x31
#define DATA_FORMAT_CONFIG 0b0'0'0'0'1'0'00

// address of the first output register
#define DATAX0 0x32

#define SPI_FLAG 1

ADXL345 adxl(PB_9, PB_8);

void read_data_adxllib()
{
    adxl.setDataRate(BW_RATE_CONFIG);
    adxl.setPowerControl(POWER_CTL_CONFIG);
    adxl.setDataFormatControl(DATA_FORMAT_CONFIG);

    while (1) {
        int devid = adxl.getDevId();
        if (devid != 0xE5) {
            printf("ERROR: DEVID is not 0xE5\n");
            return;
        }

        float readings[3];
        adxl.getOutput(readings);
        printf("x: %4.5f\ty: %4.5f\tz: %4.5f\n", readings[0], readings[1], readings[2]);

        thread_sleep_for(100);
    }
}

int main()
{
    read_data_adxllib();
}
