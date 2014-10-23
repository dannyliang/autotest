#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <getopt.h>

#include "autotest.h"

struct device_info dev;

void usage()
{
	printf("Usage: autotest [OPTION...]\n");
	printf("  -c, --config-file		Configure file of device\n");
	printf("  -t, --test-case		The file included test case you'd like to test\n");
	printf("  -h, --help			Show this help message\n");
	exit(0);
}

int main(int argc, char *argv[]){

	int c, err = 0;
	FILE * test_file;
	char action[32] = {0}, line[256] = {0};
	unsigned int delay = 0, touch_x = 0, touch_y = 0;
	struct option long_options[] = {  
		{ "condig-file"	,	1,	NULL,	'c' },  
		{ "test-case"	,	1,	NULL,	't' },  
		{ "help"		,	0,	NULL,	'h' },  
		{ 			   0,	0,	NULL,	 0	},  
	};

    while((c = getopt_long (argc, argv, "c:t:h", long_options, NULL)) != -1) {
	    switch (c){
			case 'c':
				err = read_config(optarg, &dev);
				break;
			case 't':
				test_file = fopen(optarg, "r");
				if( !test_file)
					printf("Error: test file doesn't exist\n");
				break;
			case 'h':
				usage();
				break;
		}  
	}

	init_actions(&dev);

	if (!err && test_file){
		while(!feof(test_file)){
			memset(action, 0, sizeof(action)); 
			fscanf(test_file, "%s", action);

			if(!strcmp(action, "sleep")){
				fscanf(test_file, "%d", &delay);
				sleep(delay);
			} else if (!strcmp(action, "slide_up")){
				slide_up(&dev);
			} else if (!strcmp(action, "slide_down")){
				slide_down(&dev);
			} else if (!strcmp(action, "slide_right")){
				slide_right(&dev);
			} else if (!strcmp(action, "slide_left")){
				slide_left(&dev);
			} else if (!strcmp(action, "screen_unlock")){
				screen_unlock(&dev);
			} else if (!strcmp(action, "touch")){
				fscanf(test_file, "%d %d", &touch_x, &touch_y);
				touch(&dev, touch_x, touch_y);
			} else if (!strcmp(action, "long_touch")){
				fscanf(test_file, "%d %d", &touch_x, &touch_y);
				long_touch(&dev, touch_x, touch_y);
			} else if (!strcmp(action, "press_pwrkey")){
				press_pwrkey(&dev);
			} else if (!strcmp(action, "long_press_pwrkey")){
				long_press_pwrkey(&dev);
			} else if (!strcmp(action, "press_homekey")){
				press_homekey(&dev);
			} else if (!strcmp(action, "long_press_homekey")){
				long_press_homekey(&dev);
			} else if (!strcmp(action, "suspend")){
				fscanf(test_file, "%d", &delay);
				suspend(&dev, delay);
			} else if (!strcmp(action, "resume")){
				resume(&dev);
			} else if (!strcmp(action, "#")){
				fgets(line, sizeof(line), test_file);
			} else if (action[0] == '#'){
				fgets(line, sizeof(line), test_file);
			} else if (action[0]!='\0')
				printf("Input error! Invalid command in test script\n");
		}
	}

	fclose(test_file);
	return 0;
}
