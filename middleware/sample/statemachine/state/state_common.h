#ifndef STATE_COMMON_H_
#define STATE_COMMON_H_
#include "hi_hfsm.h"

#define HI_STATE_IDLE                   "Idle"
#define HI_STATE_REC                    "Record"
#define HI_STATE_REC_NORMAL             "Rec_Normal"
#define HI_STATE_REC_LOOP               "Rec_Loop"
#define HI_STATE_REC_LAPSE              "Rec_Lapse"
#define HI_STATE_REC_SLOW               "Rec_Slow"
#define HI_STATE_PHOTO                  "Photo"
#define HI_STATE_PHOTO_SINGLE           "Photo_Single"
#define HI_STATE_PHOTO_DELAY            "Photo_Delay"
#define HI_STATE_PLAYBACK               "PlayBack"

typedef enum tagSAMPLE_STATE_E
{
    SAMPLE_STATE_IDLE,
    SAMPLE_STATE_REC,
    SAMPLE_STATE_REC_NORMAL,
    SAMPLE_STATE_REC_LOOP,
    SAMPLE_STATE_REC_LAPSE,
    SAMPLE_STATE_REC_SLOW,
    SAMPLE_STATE_PHOTO,
    SAMPLE_STATE_PHOTO_SINGLE,
    SAMPLE_STATE_PHOTO_DELAY,
    SAMPLE_STATE_PLAYBACK,

    SAMPLE_STATE_BUTT
} HI_SAMPLE_STATE_E;

extern HI_STATE_S Idle;
extern HI_STATE_S Record;
extern HI_STATE_S Normal;
extern HI_STATE_S Loop;
extern HI_STATE_S Lapse;
extern HI_STATE_S Slow;
extern HI_STATE_S Photo;
extern HI_STATE_S Single;
extern HI_STATE_S Delay;
extern HI_STATE_S PlayBack;
#endif
