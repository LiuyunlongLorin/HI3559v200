#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <string.h>

#define BITLONG 32
#define BITWORD(a) ((a) / BITLONG)
#define MAX_POINT_NUM (2)
static int input_bit_check(int bit, const volatile int* addr)
{
    return 1UL & (addr[BITWORD(bit)] >> (bit & (BITLONG - 1)));
}



typedef struct hi_mtinput
{
    int temp_x[MAX_POINT_NUM];
    int temp_y[MAX_POINT_NUM];
    int temp_p[MAX_POINT_NUM];
    int temp_filled[MAX_POINT_NUM];
    int      temp_slot;
} HI_MTINPUT;


typedef struct hi_mtsample
{
    int     x;
    int     y;
    unsigned int    pressure;
    int     id;
    struct timeval  tv;
} HI_MTSAMPLE;

static  HI_MTINPUT s_stmtinput;

int main(void)
{
    int s_touch_fd = -1;
    int s32ret = -1;
    int input_bits[(ABS_MAX + 31) / 32];
    int total = 0;
    int nPushed = 0;
    struct input_event ev;
    s_touch_fd = open("/dev/input/event0", O_RDONLY);
    if (s_touch_fd < 0)
    {
        perror("open /dev/input/event0 err");
        return -1;
    }
    s32ret = ioctl(s_touch_fd, EVIOCGBIT(EV_ABS, sizeof(input_bits)), input_bits);
    if (s32ret < 0)
    {
        return -1;
    }

    if ( ( 0 == input_bit_check(ABS_MT_POSITION_X, input_bits) )
             || ( 0 == input_bit_check(ABS_MT_POSITION_Y, input_bits) )
             || ( 0 == input_bit_check(ABS_MT_TOUCH_MAJOR, input_bits) )
           )
    {
            printf("error: could not support the device\n");
            printf("EV_SYN=%d\n", input_bit_check(EV_SYN, input_bits));
            printf("EV_ABS=%d\n", input_bit_check(EV_ABS, input_bits));
            printf("ABS_MT_POSITION_X=%d\n", input_bit_check(ABS_MT_POSITION_X, input_bits));
            printf("ABS_MT_POSITION_Y=%d\n", input_bit_check(ABS_MT_POSITION_Y, input_bits));
            printf("ABS_MT_TOUCH_MAJOR=%d\n", input_bit_check(ABS_MT_TOUCH_MAJOR, input_bits));
            return -1;
    }

    while (1)
        {
            s32ret = read(s_touch_fd, &ev, sizeof(struct input_event));

            if (s32ret < sizeof(struct input_event))
            {
                printf("can not get more data\n");
                // no more data
                continue;
            }

            switch (ev.type)
            {
                case EV_KEY:

                    printf("event=EV_KEY:\n");
                    switch (ev.code)
                    {
                            //  impossible jump in here
                        case BTN_TOUCH:
                            printf("this TP is not support key touch\n");
                            break;
                    }

                    break;

                    //0x00
                case EV_SYN:
                    switch (ev.code)
                    {
                        case SYN_REPORT:
                        case SYN_MT_REPORT:
                        default:
                            printf("event=%d.code=%d\n", ev.type, ev.code);
                    }

                    break;

                    //0x3
                case EV_ABS:
                    switch (ev.code)
                    {
                            //0x3a    /* Pressure on contact area */
                        case ABS_PRESSURE:
                            printf("event.code=ABS_PRESSURE.v=%d\n", ev.value);

                            break;

                            //0x2f    /* MT slot being modified */
                        case ABS_MT_SLOT:

                            printf("event.code=ABS_MT_SLOT.v=%d\n", ev.value);

                            if (ev.value < 0)
                            {
                                break;
                            }

                            if ((&s_stmtinput)->temp_slot >= MAX_POINT_NUM)
                            {
                                printf("slot limit error. MAX_POINT_NUM=%d. temp slot=%d\n", MAX_POINT_NUM, (&s_stmtinput)->temp_slot);
                            }

                            break;

                            //0X30 /* Major axis of touching ellipse */
                        case ABS_MT_TOUCH_MAJOR:
                            printf("event.code=ABS_MT_TOUCH_MAJOR.v=%d\n", ev.value);
                            break;

                            //0X35
                        case ABS_MT_POSITION_X:
                            printf("event.code=ABS_MT_POSITION_X.v=%d\n", ev.value);

                            break;

                            //0X36
                        case ABS_MT_POSITION_Y:
                            printf("event.code=ABS_MT_POSITION_Y.v=%d\n", ev.value);

                            break;

                            //0X39
                        case ABS_MT_TRACKING_ID:

                            printf("event.code=ABS_MT_TRACKING_ID.v=%d\n", ev.value);

                            break;
                    }

                break;

            }

    }



    return 0;
}
