#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "bbb_gpio.h"

int main() {
    
    int gpio = 67;
    init_gpio(gpio);
    set_gpio_direction(gpio, "out");
    while(1){
        printf("Main....\n");
	set_gpio_value(gpio, 1);
        sleep(2);
	set_gpio_value(gpio, 0);
        sleep(2);
	
    }

    return 0;

}
