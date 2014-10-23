#include <stdio.h>
#include <stdlib.h>
#include "autotest.h"

void config_dump(struct device_info * dev)
{
	printf("name: %s\n", dev->name);
	printf("dev_touch: %s\n", dev->dev_touch);
	printf("dev_key: %s\n", dev->dev_key);
	printf("length: %d\n", dev->length);
	printf("width: %d\n", dev->width);
	printf("num_pts: %d\n", dev->num_pts);
	printf("delay_us: %d\n", dev->delay_us);
}

/*
*  Parse device configure file included name, dev_touch, dev_key, length,
*  width, num_pts and delay_us
*/

int config_parser(FILE * cfg, struct device_info * dev)
{
	char item[16] = {0}, tmp[32] = {0};
	int value, ret = 0;

	while(!feof(cfg)){
		fscanf(cfg, "%s", item);
		if (!strcmp("name", item)){
			fscanf(cfg, "%s", dev->name);
		}else if (!strcmp("dev_touch", item)){
			fscanf(cfg, "%s", dev->dev_touch);
		}else if (!strcmp("dev_key", item)){
			fscanf(cfg, "%s", dev->dev_key);
		}else if (!strcmp("length", item)){
			fscanf(cfg, "%d", &value);
			dev->length = value;
			printf("%d, %d\n",dev->length, value);
		}else if (!strcmp("width", item)){
			fscanf(cfg, "%d", &value);
			dev->width = value;
		}else if (!strcmp("num_pts", item)){
			fscanf(cfg, "%d", &value);
			dev->num_pts = value;
		}else if (!strcmp("delay_us", item)){
			fscanf(cfg, "%d", &value);
			dev->delay_us = value;
		}else{
			fscanf(cfg, "%s", tmp);
			printf("Error: invalid configuration, %s\n", item);
			ret = -1;
		}
	}

	if (!dev->dev_touch || !dev->dev_key || !dev->length || !dev->width){
		printf("Error: necesary settings are empty, please check it!\n");
		ret = -1;
	}

	if (!dev->num_pts)
		dev->num_pts = 18;

	if (!dev->delay_us)
		dev->delay_us = 21500;

	config_dump(dev);
	return ret;
}

int read_config(const char * cfg_file_name, struct device_info * dev)
{
	int ret = 0;
	FILE *cfg_file;

	cfg_file = fopen(cfg_file_name, "r");
	if( cfg_file != NULL){
		ret = config_parser( cfg_file, dev);
	}
	else{
		printf("Error: configure file doesn't exist\n");
		ret = -1;
	}

	fclose(cfg_file);
	return ret;
}


int read_test_case( const char * test_case_name)
{
	return 0;
}
