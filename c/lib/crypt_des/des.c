//**
//* 6Harmonics Qige @ 2015.11.25
//*

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <crypt.h>

typedef unsigned char	uchar;
typedef unsigned int	uint;

void app_help(char *app);


int main(int argc, char *argv[])
{
	uint _c, _help = 0;
	uchar key[8], slat[2];

	while((_c = getopt(argc, argv, "s:h")) != -1) {
		switch(_c) {
			case 's':
				memset(key, 0, sizeof(key));
				snprintf(key, sizeof(key), "%s", optarg);
				slat[0] = key[0]; 
				slat[1] = key[1];
				break;
			case 'h':
			default:
				_help = 1;
				break;
		}
	}

	if (_help > 0 || argc <= 1) {
		app_help(argv[0]);
		return 0;
	}

	printf(" * DES(%s) = %s\n", key, crypt(key, slat));
	return 0;
}


void app_help(char *app)
{
	printf("Compile: gcc -o %s *.c\n", app);
	printf("  Usage: %s [-s string] [-h]\n", app);
	printf("typical: %s -s test\n", app);
}

