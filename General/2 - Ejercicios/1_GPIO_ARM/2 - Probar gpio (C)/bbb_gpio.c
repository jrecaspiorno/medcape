//Includes
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bbb_gpio.h"


//
//    Populate GPIO by writing gpio in the 'export'
//  file, this will create gpioXX directory where
//  XX is gpio.
//
int init_gpio(int gpio) {
    FILE *io;

    io = fopen("/sys/class/gpio/export", "w");
    if (io == NULL) {
        perror("Pin failed to initialize");
        return (-1);
    }
    fseek(io,0,SEEK_SET);
    fprintf(io,"%d",gpio);
    fflush(io);
    fclose(io);

    return 0;
}


//
//    Set a populated GPIO direction by writing in/out
//  in /sys/class/gpio/gpioXX/direction, where XX is gpio.
//
int set_gpio_direction(int gpio, char *dir) {
    FILE *pdir;
    char buf[10];
    char buf2[50] = "/sys/class/gpio/gpio";

    //build file path to the direction file
    sprintf(buf,"%i",gpio);
    strcat(buf2,strcat(buf,"/direction"));

    pdir = fopen(buf2, "w");
    if (pdir == NULL) {
        perror("Direction failed to open\n");
        return -1;
    }
    fseek(pdir,0,SEEK_SET);
    fprintf(pdir,"%s",dir);
    fflush(pdir);
    fclose(pdir);

    return 0;
}


//
//    Set a populated GPIO output value by writing 1/0
//  in /sys/class/gpio/gpioXX/value, where XX is gpio.
//
int set_gpio_value(int gpio, int value) {
    FILE *val;
    char buf[5];
    char buf2[50] = "/sys/class/gpio/gpio";

    //build path to value file
    sprintf(buf,"%i",gpio);
    strcat(buf2,strcat(buf,"/value"));

    val = fopen(buf2, "w");
    if (val == NULL) {
        perror("Value failed to open\n");
        return -1;
    }
    fseek(val,0,SEEK_SET);
    fprintf(val,"%d",value);
    fflush(val);
    fclose(val);

    return 0;
}

//
//    get a populated GPIO input value by reading
//  /sys/class/gpio/gpioXX/value, where XX is gpio.
//
int get_gpio_value(int gpio) {
    FILE *val;
    int value;
    char buf[5];
    char buf2[50] = "/sys/class/gpio/gpio";

    //build file path to value file
    sprintf(buf,"%i",gpio);
    strcat(buf2,strcat(buf,"/value"));

    val = fopen(buf2, "r");
    if (val == NULL) {
        perror("Input value failed to open\n");
        return -1;
    }
    fseek(val,0,SEEK_SET);
    fscanf(val,"%d",&value);
    fclose(val);

    return value;
}



