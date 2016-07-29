#ifndef _BBB_GPIO_H_
#define _BBB_GPIO_H_

#include <sys/epoll.h>

struct gpio_edge {
    unsigned int gpio;
    int fd;
    int epollfd;
    struct epoll_event ev;
};

int init_gpio(int gpio);
int set_gpio_direction(int gpio, char* dir);
int get_gpio_value(int gpio);
int set_gpio_value(int gpio, int value);
int set_gpio_edge(unsigned int gpio, char* edge, struct gpio_edge* st_edge);
int wait_for_edge(struct gpio_edge* st_edge);
void close_gpio_edge(struct gpio_edge* st_edge);

#endif
