#include <time.h>
#include <sys/time.h>

#define AT_DBG 1

// TYPE
#define EV_SYN 0
#define EV_KEY 1
#define EV_ABS 3

// CODE
#define SYN_REPORT 0
#define SYN_MT_REPORT 2
#define ABS_PRESSURE 24
#define ABS_MT_TOUCH_MAJOR 48
#define ABS_MT_WIDTH_MAJOR 50
#define ABS_MT_TRACKING_ID 57
#define ABS_MT_POSITION_X 53
#define ABS_MT_POSITION_Y 54
#define BTN_TOUCH 330
#define KEY_HOME 102
#define KEY_POWER 116
// VALUE
#define UP 0
#define DOWN 1

#define INPUT_TOUCH_DEV_CLASS_ST 		0x01
#define INPUT_TOUCH_DEV_CLASS_MT 		0x02
#define INPUT_TOUCH_DEV_CLASS_MT_SYNC 	0x04

#define EVIOCGVERSION _IOR('E', 0x01, int) /* get driver version */

struct device_info {
    char name[10];
    char dev_touch[18];
    char dev_key[18];
    __u16 length;
    __u16 width;
	__u8  num_pts;
	__u16 delay_us;
	__u8 dev_flag;
};

struct action {
    __u16 start_x;
    __u16 start_y;
    __u16 end_x;
    __u16 end_y;
    __u8  num_pts;
    __u16 delay_us;
};

enum action_type {
	UNKNOWN_ACTION = -1,
	SLIDE_UP = 0,
	SLIDE_DOWN,
	SLIDE_RIGHT,
	SLIDE_LEFT,
	SCREEN_UNLOCK,
	SHOW_STATUSBAR,
	NUM_ACTIONS,
};

enum devices {
    UNKNOWN_DEVICE = -1,
    FLAME = 0,
    WOODDUCK,
};

struct input_event {
        struct timeval time;
        __u16 type;
        __u16 code;
        __s32 value;
};

int read_config(const char * cfg_file_name, struct device_info *dev);
int read_test_case( const char * test_case_name);

void init_actions(struct device_info *dev);
void slide_up(struct device_info * dev);
void slide_down(struct device_info * dev);
void slide_right(struct device_info * dev);
void slide_left(struct device_info * dev);
void screen_unlock(struct device_info * dev);
void press_pwrkey(struct device_info *dev);
void long_press_pwrkey(struct device_info *dev);
void press_homekey(struct device_info *dev);
void long_press_homekey(struct device_info *dev);
void touch(struct device_info * dev, unsigned int x, unsigned int y);
void long_touch(struct device_info * dev, unsigned int x, unsigned int y);
int suspend(struct device_info * dev, unsigned int seconds);
void resume(struct device_info * dev);

