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


int set_gpio_edge(unsigned int gpio, char *edge, struct gpio_edge *st_edge) {
    FILE *pf;
    char buf[64];

    //GPIO, must be an input pin to poll its value ...
    //if( init_gpio(gpio) || set_gpio_direction(gpio, "in") ){
    //    return -1;
    //}

    //Prepare for waiting edges
    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);

    st_edge->gpio=-1;
    if ( (st_edge->epollfd=epoll_create(1))  == -1) {
        perror("GPIO: Failed to create epollfd");
        return -1;
    }
    if ((st_edge->fd = open(buf, O_RDONLY | O_NONBLOCK)) == -1) {
        perror("GPIO: Failed to open file");
        close(st_edge->epollfd);
        return -1;
    }

    //ev.events = read operation | edge triggered | urgent data
    st_edge->ev.events = EPOLLIN | EPOLLET | EPOLLPRI;
    st_edge->ev.data.fd = st_edge->fd;  // attach the file file descriptor

    //Register the file descriptor on the epoll instance, see: man epoll_ctl
    if (epoll_ctl(st_edge->epollfd, EPOLL_CTL_ADD, st_edge->fd,
                  &st_edge->ev) == -1) {
        perror("GPIO: Failed to add control interface");
        close(st_edge->epollfd);
        close(st_edge->fd);
        return -1;
    }

    //Spefify edge
    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/edge", gpio);

    if ( (pf=fopen(buf, "w")) == NULL ) {
        perror("Edge failed to open");
        close(st_edge->epollfd);
        close(st_edge->fd);
        return -1;
    }
    //fseek(pfedge,0,SEEK_SET);
    fprintf(pf,"%s",edge);
    //fflush(pfedge);
    fclose(pf);

    st_edge->gpio=gpio;

    return 0;
}

void close_gpio_edge(struct gpio_edge *st_edge) {
    st_edge->gpio=-1;
    close(st_edge->epollfd);
    close(st_edge->fd);
}

// Blocking Poll - based on the epoll socket code in the epoll man page
int wait_for_edge(struct gpio_edge *st_edge) {
    //int i, count=0;

    if (st_edge->gpio == -1) return -1;

    if ( epoll_wait(st_edge->epollfd, &st_edge->ev, 1, -1) == -1 ) {
        perror("GPIO: Poll Wait fail");
        return -1;
    }
    return 0;
}


/*
int gpio_fd_open(unsigned int gpio){
    int fd;
    char buf[64];

    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY | O_NONBLOCK );
    if (fd < 0) {
        perror("gpio/fd_open");
    }
    return fd;
}
*/