#ifndef _BBB_GPIO_H_
#define _BBB_GPIO_H_

#include <sys/epoll.h>

int init_gpio(int gpio);
int set_gpio_direction(int gpio, char* dir);
int get_gpio_value(int gpio);
int set_gpio_value(int gpio, int value);

#endif
