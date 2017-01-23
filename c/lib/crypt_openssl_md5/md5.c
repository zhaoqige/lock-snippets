//**
//* 6Harmonics Qige @ 2015.11.22
//**

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <openssl/md5.h>

#define PARAM_LENGTH_MAX	32
#define MD5_RESULT_LENGTH	33


void app_help(char *self)
{
	printf("Compile: gcc -o %s *.c -lssl\n", self);
	printf("  Usage: %s [-s string]\n", self);
}


int main(int argc, char *argv[])
{
	int c, i;
	unsigned char buf[32] = { 0 };
	unsigned char tmp[2] = { 0 }, md[16];

	unsigned char data[32], _data[32];
	memset(&data, 0, sizeof(data));
	memset(&_data, 0, sizeof(_data));

	while((c = getopt(argc, argv, "s:")) != -1) {
		switch(c) {
			case 's':
				snprintf(data, sizeof(data), "%s", optarg);
				snprintf(_data, sizeof(data), "%s", optarg);
				break;
			default:
				break;
		}
	}


	if (strlen(data) < 1) {
		app_help(argv[0]);
		return 0;
	}
		


	MD5(_data, strlen(_data), md);
	for(i = 0; i < 16; i ++) {
		sprintf(tmp, "%2.2x", md[i]); //printf(" md[%d] = %s\n", i, tmp);
		strcat(buf, tmp);
	}

	printf(" md5(>%s<) = >%s<\n", data, buf);

	return 0;
}
