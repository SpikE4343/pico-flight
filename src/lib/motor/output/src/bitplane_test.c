
#include <stdio.h>
#include <stdlib.h>

#include "motor_output_dshot_pio.h"

#include <assert.h>
#include <string.h>

uint16_t throttle[MAX_MOTORS];
DshotPacket_t packets[MAX_MOTORS];
uint8_t bitPlane[sizeof(DshotPacket_t)*8];

int setup_test()
{
    memset(throttle, 0, sizeof(throttle));
    memset(packets, 0, sizeof(packets));
}

int run_test()
{
    for(int c=0; c < MAX_MOTORS; ++c)
    {
        throttle[c] = rand() % MOTOR_MAX_OUTPUT;
        packets[c] = dshotBuildPacket(throttle[c]);

        printf("p[%u]: %04X\n", c, packets[c].value);
    }

    dshot_update_bitplanes(bitPlane, packets);
    printf("====\n");

    for(int bit=0; bit < sizeof(DshotPacket_t)*8; ++bit)
    {
        printf(" %02X", bitPlane[bit]);
    }

    printf("\n");

    for(int c=MAX_MOTORS-1; c >= 0; --c)
    {
        printf("[%u]: %04X = ", c, packets[c].value);
        for(int bit=0; bit < sizeof(DshotPacket_t)*8; ++bit)
        {
            printf("_%u", (bitPlane[bit] & (1 << c)) > 0 ? 1 : 0);
        }

        printf("\n");
    }    

    return 0;
}


int main()
{
    setup_test();
    return run_test();
}