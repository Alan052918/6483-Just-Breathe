// Documents
// Dev board dev manual: https://www.st.com/resource/en/user_manual/um1670-discovery-kit-with-stm32f429zi-mcu-stmicroelectronics.pdf
// Accelerometer datasheet: https://www.mouser.com/datasheet/2/389/dm00168691-1798633.pdf
#include "mbed.h"

#include <ADXL345_I2C.h>

ADXL345 adxl(PB_9, PB_8);

void read_data_adxllib()
{
    adxl.setDataRate(0x0B);
    adxl.setPowerMode(NORMAL_OPERATION);
    adxl.setPowerControl(0b00'0'0'1'0'00);
    adxl.setDataFormatControl(0b0'0'0'0'1'0'00);

    while (1) {
        int devid = adxl.getDevId();
        if (devid != 0xE5) {
            printf("ERROR: DEVID is not 0xE5\n");
            return;
        }

        int readings[3];
        adxl.getOutput(readings);
        printf("x: %5d\ty: %5d\tz: %5d\n", readings[0], readings[1], readings[2]);

        thread_sleep_for(100);
    }
}

int main()
{
    read_data_adxllib();
}
