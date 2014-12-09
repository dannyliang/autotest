#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <linux/android_alarm.h>

#include "autotest.h"

struct action actions[NUM_ACTIONS];

void init_actions(struct device_info *dev)
{
	int i;
	for( i=0; i< NUM_ACTIONS; i++){
		switch(i){
			case SLIDE_UP:
				actions[SLIDE_UP].start_x = dev->width/2;
				actions[SLIDE_UP].start_y = dev->length*5/6;
				actions[SLIDE_UP].end_x = actions[SLIDE_UP].start_x;
				actions[SLIDE_UP].end_y = dev->length*1/6;
				actions[SLIDE_UP].num_pts = dev->num_pts;
				actions[SLIDE_UP].delay_us = dev->delay_us;
				break;
			case SLIDE_DOWN:
				actions[SLIDE_DOWN].start_x = dev->width/2;
				actions[SLIDE_DOWN].start_y = dev->length*1/6;
				actions[SLIDE_DOWN].end_x = actions[SLIDE_UP].start_x;
				actions[SLIDE_DOWN].end_y = dev->length*5/6;
				actions[SLIDE_DOWN].num_pts = dev->num_pts;
				actions[SLIDE_DOWN].delay_us = dev->delay_us;
				break;
			case SLIDE_RIGHT:
				actions[SLIDE_RIGHT].start_x = dev->width/4;
				actions[SLIDE_RIGHT].start_y = dev->length/2;
				actions[SLIDE_RIGHT].end_x = dev->width*3/4;
				actions[SLIDE_RIGHT].end_y = actions[SLIDE_UP].start_y;
				// folowing two parameters migh need to adjust
				actions[SLIDE_RIGHT].num_pts = dev->num_pts;
				actions[SLIDE_RIGHT].delay_us = dev->delay_us;
				break;
			case SLIDE_LEFT:
				actions[SLIDE_LEFT].start_x = dev->width*3/4;
				actions[SLIDE_LEFT].start_y = dev->length/2;
				actions[SLIDE_LEFT].end_x = dev->width/4;
				actions[SLIDE_LEFT].end_y = actions[SLIDE_UP].start_y;
				// folowing two parameters migh need to adjust
				actions[SLIDE_LEFT].num_pts = dev->num_pts;
				actions[SLIDE_LEFT].delay_us = dev->delay_us;
				break;
			case SCREEN_UNLOCK:
				actions[SCREEN_UNLOCK].start_x = dev->width/2;
				actions[SCREEN_UNLOCK].start_y = dev->length*7/8;
				actions[SCREEN_UNLOCK].end_x = dev->width*5/6;
				actions[SCREEN_UNLOCK].end_y = actions[SCREEN_UNLOCK].start_y;
				// folowing two parameters migh need to adjust
				actions[SCREEN_UNLOCK].num_pts = 5;
				actions[SCREEN_UNLOCK].delay_us = dev->delay_us;
				break;
			case SHOW_STATUSBAR:
				actions[SHOW_STATUSBAR].start_x = dev->width/2;
				actions[SHOW_STATUSBAR].start_y = 0;
				actions[SHOW_STATUSBAR].end_x = actions[SCREEN_UNLOCK].start_x;
				actions[SHOW_STATUSBAR].end_y = dev->length/2;
				// folowing two parameters migh need to adjust
				actions[SHOW_STATUSBAR].num_pts = dev->num_pts;
				actions[SHOW_STATUSBAR].delay_us = dev->delay_us;
			default:
				break;
		}
	}
}

static int exec_sendevent(const char* event_dev, int type, int code, int val)
{
    int i, fd, ret, version;
    struct input_event event;

    fd = open(event_dev, O_RDWR);
    if(fd < 0) {
        fprintf(stderr, "could not open %s, %s\n", event_dev, strerror(errno));
        return 1;
    }
    if (ioctl(fd, EVIOCGVERSION, &version)) {
        fprintf(stderr, "could not get driver version for %s, %s\n", event_dev, strerror(errno));
        return 1;
    }
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.code = code;
    event.value = val;
    ret = write(fd, &event, sizeof(event));
    if(ret < sizeof(event)) {
        fprintf(stderr, "write event failed, %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

static void event_end( const char* event_dev) {
    exec_sendevent(event_dev, EV_SYN, SYN_MT_REPORT, 0);
    exec_sendevent(event_dev, EV_SYN, SYN_REPORT, 0);
}

static int _slide(struct device_info *dev, enum action_type type)
{
    char cmd[50] = {0};
    int x = actions[type].start_x, y = actions[type].start_y;
	int off_x, off_y, i;
    exec_sendevent(dev->dev_touch, EV_ABS, ABS_MT_TOUCH_MAJOR, 100);
    exec_sendevent(dev->dev_touch, EV_ABS, ABS_PRESSURE, 100);
    exec_sendevent(dev->dev_touch, EV_KEY, BTN_TOUCH, DOWN);
	
    off_x = (actions[type].end_x - actions[type].start_x)/actions[type].num_pts;
    off_y = (actions[type].end_y - actions[type].start_y)/actions[type].num_pts;

    for( i=0; i < actions[type].num_pts; i++){
        if(i == 0)
            exec_sendevent(dev->dev_touch, EV_ABS, ABS_MT_TOUCH_MAJOR, 64);

        exec_sendevent(dev->dev_touch, EV_ABS, ABS_MT_POSITION_X, x);
        exec_sendevent(dev->dev_touch, EV_ABS, ABS_MT_POSITION_Y, y);
        event_end(dev->dev_touch);
        x = x + off_x;
        y = y + off_y;
        usleep(actions[type].delay_us);
    }
    exec_sendevent(dev->dev_touch, EV_KEY, BTN_TOUCH ,UP);
    exec_sendevent(dev->dev_touch, EV_ABS, ABS_PRESSURE, 0);
    event_end(dev->dev_touch);
    event_end(dev->dev_touch);
    event_end(dev->dev_touch);

    return 0;
}

static int check_boundary( struct device_info *dev, unsigned int x, unsigned int y)
{
	int retval = 0;

	if (x >= 0 && x <= dev->width && y == 510 && 
			!strcmp(dev->name, "woodduck"))
		retval = 0;
	else if ( x < 0 || x > dev->width || y < 0 || y > dev->length)
		retval = -EINVAL;

	return retval;
}

static int _touch(struct device_info *dev, unsigned int x, unsigned y, int pts)
{
    char cmd[50] = {0};
	int i, retval;

	retval = check_boundary(dev, x, y);

	if(!retval){
	    exec_sendevent(dev->dev_touch, EV_ABS, ABS_MT_TOUCH_MAJOR, 100);
    	exec_sendevent(dev->dev_touch, EV_ABS, ABS_PRESSURE, 100);
    	exec_sendevent(dev->dev_touch, EV_KEY, BTN_TOUCH, DOWN);

	    for( i=0; i < pts; i++){
    	    if(i == 0)
        	    exec_sendevent(dev->dev_touch, EV_ABS, ABS_MT_TOUCH_MAJOR, 64);

	        exec_sendevent(dev->dev_touch, EV_ABS, ABS_MT_POSITION_X, x);
    	    exec_sendevent(dev->dev_touch, EV_ABS, ABS_MT_POSITION_Y, y);
        	event_end(dev->dev_touch);
	        usleep(dev->delay_us);
    	}
	    exec_sendevent(dev->dev_touch, EV_KEY, BTN_TOUCH ,UP);
   		exec_sendevent(dev->dev_touch, EV_ABS, ABS_PRESSURE, 0);
	    event_end(dev->dev_touch);
	    event_end(dev->dev_touch);
	    event_end(dev->dev_touch);
	}
	else if(AT_DBG)
		printf("Error: Position is out of range\n");

    return retval;
}

void slide_up(struct device_info * dev)
{
	if (AT_DBG)
		printf("%s\n",__func__);

	_slide(dev, SLIDE_UP);
}

void slide_down(struct device_info * dev)
{
	if (AT_DBG)
		printf("%s\n",__func__);

	_slide(dev, SLIDE_DOWN);
}

void slide_right(struct device_info * dev)
{
	if (AT_DBG)
		printf("%s\n",__func__);

	_slide(dev, SLIDE_RIGHT);
}

void slide_left(struct device_info * dev)
{
	if (AT_DBG)
		printf("%s\n",__func__);

	_slide(dev, SLIDE_LEFT);
}

void screen_unlock(struct device_info * dev)
{
	if (AT_DBG)
		printf("%s\n",__func__);

	_slide(dev, SCREEN_UNLOCK);
}

static void _press_key(struct device_info *dev, int code,  unsigned int delay)
{
	exec_sendevent(dev->dev_key, EV_KEY, code, DOWN);
	exec_sendevent(dev->dev_key, EV_SYN, 0, 0);
	if(delay)
		sleep(delay);
	exec_sendevent(dev->dev_key, EV_KEY, code, UP);
	exec_sendevent(dev->dev_key, EV_SYN, 0, 0);
}

void press_pwrkey(struct device_info *dev)
{
	if (AT_DBG)
		printf("%s\n", __func__);

	_press_key(dev, KEY_POWER, 0);
}

void long_press_pwrkey(struct device_info *dev)
{
	if (AT_DBG)
		printf("%s\n", __func__);

	_press_key(dev, KEY_POWER, 1);
}

void press_homekey(struct device_info *dev)
{
	if (AT_DBG)
		printf("%s\n", __func__);

	if(!strcmp(dev->name, "woodduck"))
		_touch(dev, 159, 510, 1);
	else
		_press_key(dev, KEY_HOME, 0);
}

void long_press_homekey(struct device_info *dev)
{
	if (AT_DBG)
		printf("%s\n", __func__);

	if(!strcmp(dev->name, "woodduck"))
		_touch(dev, 159, 510, 3);
	else
		_press_key(dev, KEY_HOME, 1);
}

void touch(struct device_info * dev, unsigned int x, unsigned int y)
{
	if (AT_DBG)
		printf("%s, x = %d, y = %d\n", __func__, x, y);

	_touch(dev, x, y, 1);
}

void long_touch(struct device_info * dev, unsigned int x, unsigned int y)
{
	if (AT_DBG)
		printf("%s, x = %d, y = %d\n", __func__, x, y);

	_touch(dev, x, y, 3);
}

static int add_rtc_time (struct rtc_wkalrm *alarm, unsigned int seconds)
{
	/* ideally, we won't set RTC alarm which exceed 24hrs*/
	if ( seconds > 86400 )
		return -EINVAL;

	alarm->time.tm_sec += seconds;
	if (alarm->time.tm_sec > 60 ){
		alarm->time.tm_min += alarm->time.tm_sec/60 ;
		alarm->time.tm_sec %= 60;
		if (alarm->time.tm_min > 60){
			alarm->time.tm_hour += alarm->time.tm_min/60 ;
			alarm->time.tm_min %= 60;
			if (alarm->time.tm_hour > 24){
				alarm->time.tm_wday ++;
				alarm->time.tm_wday %= 7;
				alarm->time.tm_mday ++;
				/* TODO */
			}
		}
	}
	return 0;
}

static void dump_rtc_alarm(struct rtc_wkalrm * alarm)
{
	printf("RTC alarm: %d:%d:%d, mday: %d, mon: %d, year: %d, wday: %d, yday: %d\n",
		alarm->time.tm_sec, alarm->time.tm_min, alarm->time.tm_hour,
		alarm->time.tm_mday, alarm->time.tm_mon, alarm->time.tm_year,
		alarm->time.tm_wday, alarm->time.tm_yday);
}


static int set_rtc_alarm (unsigned int seconds)
{
	int fd;
	int retval = 0;
	struct rtc_wkalrm alarm;
	struct rtc_time now;

#if 0
	do{
		fd = open("/dev/alarm", O_RDWR);
		printf("Open device fd %d\n", fd);
	}while (fd == -1 && errno == EINTR);

  	if (fd < 0) {
    	printf("Failed to open alarm device, %s\n", strerror(errno));
		close(fd);
    	return -1;
  	}


	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec += seconds;
#endif

	fd = open("/dev/rtc0", O_RDWR);
  	if (fd < 0) {
    	printf("Failed to open alarm device, %s\n", strerror(errno));
		close(fd);
    	return -1;
  	}
	
	retval = ioctl(fd, RTC_RD_TIME, &(alarm.time));

	if(AT_DBG)
		dump_rtc_alarm(&alarm);

	if (retval < 0) {
	   	printf("Unable to read time, %s\n", strerror(errno));
		close(fd);
	    return -1;
	}

	add_rtc_time( &alarm, seconds);	

	if(AT_DBG)
		dump_rtc_alarm(&alarm);

	retval = ioctl(fd, RTC_WKALM_SET, &alarm);

	if (retval < 0) {
	   	printf("Unable to set alarm, %s\n", strerror(errno));
		close(fd);
	    return -1;
	}

#if 0
	result = ioctl(42, ANDROID_ALARM_SET(ANDROID_ALARM_RTC_WAKEUP), &t);

	if (result < 0) {
    	printf("Unable to set alarm, %s(fd is %d)\n", strerror(errno), fd);
		close(fd);
	    return -1;
	}
#endif 

	close(fd);
	return 0;
}

int suspend(struct device_info * dev, unsigned int seconds)
{
	struct timespec t, now;
	if(AT_DBG)
		printf("%s\n",__func__);

	clock_gettime(CLOCK_REALTIME, &t);
	set_rtc_alarm(seconds);
	press_pwrkey(dev);

	while(1){
		clock_gettime(CLOCK_REALTIME, &now);
		if(now.tv_sec >= (t.tv_sec + seconds)){
			system("echo autotest > /sys/power/wake_lock");
			break;
		}
		sleep(1);
	}

	return 0;
}

void resume(struct device_info * dev)
{
	if(AT_DBG)
		printf("%s\n",__func__);

	press_pwrkey(dev);
	sleep(2);
	screen_unlock(dev);
	sleep(2);

	system("echo autotest > /sys/power/wake_unlock");
}
