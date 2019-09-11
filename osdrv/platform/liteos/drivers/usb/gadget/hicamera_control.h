
#define CMD_MAX_NUM 16

enum CAMERA_CMD
{
    CMD_GET_CAMERA_VERSION = 0x01,
    CMD_START_CAMERA_UPDATE,
    CMD_START_CAMERA,
    CMD_SHUTDOWN_CAMERA,
    CMD_RESET_CAMERA,
    CMD_VIBRATOR_UP = 0x06,
    CMD_VIBRATOR_DOWN,
    CMD_VIBRATOR_LEFT,
    CMD_VIBRATOR_RIGHT,
};

typedef unsigned int (*camera_control_func)(void *buf, unsigned int cmdtype);

struct uvc_camera_cmd {
    unsigned int id;
    unsigned char name[32];
    camera_control_func uvc_control_func;
};

extern int hi_camera_register_cmd(struct uvc_camera_cmd *cmd);
