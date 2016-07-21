/* 
 * Copyright (c) 2015, Daan Pape
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 *     1. Redistributions of source code must retain the above copyright 
 *        notice, this list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright 
 *        notice, this list of conditions and the following disclaimer in the 
 *        documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * File:   dpt_gpio.c
 * Created on October 5, 2015, 12:42 AM
 */

#include "../../../lib/duktape/src/duktape.h"
#include "dpt_gpio.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

/* GPIO configuration, true if GPIO is exposed */
const bool gpio_config[28] = {
    true, /* GPIO 0 */
    true, /* GPIO 1 */
    false, /* GPIO 2 */
    false, /* GPIO 3 */
    false, /* GPIO 4 */
    false, /* GPIO 5 */
    true, /* GPIO 6 */
    true, /* GPIO 7 */
    true, /* GPIO 8 */
    false, /* GPIO 9 */
    false, /* GPIO 10 */
    false, /* GPIO 11 */
    true, /* GPIO 12 */
    true, /* GPIO 13 */
    true, /* GPIO 14 */
    true, /* GPIO 15 */
    true, /* GPIO 16 */
    true, /* GPIO 17 */
    true, /* GPIO 18 */
    true, /* GPIO 19 */
    true, /* GPIO 20 */
    true, /* GPIO 21 */
    true, /* GPIO 22 */
    true, /* GPIO 23 */
    true, /* GPIO 24 */
    false, /* GPIO 25 */
    false, /* GPIO 26 */
    false, /* GPIO 27 */
};

static bool gpio_reserve(int gpio) {
    int fd;         /* File descriptor for GPIO controller class */
    char buf[3];    /* Write buffer */

    /* Try to open GPIO controller class */
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        /* The file could not be opened */
        return false;
    }

    /* Prepare buffer */
    sprintf(buf, "%d", gpio);

    /* Try to reserve GPIO */
    if (write(fd, buf, strlen(buf)) < 0) {
        close(fd);
        return false;
    }

    /* Close the GPIO controller class */
    if (close(fd) < 0) {
        return false;
    }

    /* Success */
    return true;
}

static bool gpio_release(int gpio) {
    int fd; /* File descriptor for GPIO controller class */
    char buf[3]; /* Write buffer */

    /* Try to open GPIO controller class */
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        /* The file could not be opened */
        return false;
    }

    /* Prepare buffer */
    sprintf(buf, "%d", gpio);

    /* Try to release GPIO */
    if (write(fd, buf, strlen(buf)) < 0) {
        return false;
    }

    /* Close the GPIO controller class */
    if (close(fd) < 0) {
        return false;
    }

    /* Success */
    return true;
}

static bool gpio_set_direction(int gpio, int direction) {
    int fd; /* File descriptor for GPIO port */
    char buf[33]; /* Write buffer */

    /* Make the GPIO port path */
    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);

    /* Try to open GPIO port for writing only */
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        /* The file could not be opened */
        return false;
    }

    /* Set the port direction */
    if (direction == GPIO_OUT) {
        if (write(fd, "out", 3) < 0) {
            return false;
        }
    } else {
        if (write(fd, "in", 2) < 0) {
            return false;
        }
    }

    /* Close the GPIO port */
    if (close(fd) < 0) {
        return false;
    }

    /* Success */
    return true;
}

bool gpio_set_state(int gpio, int state) {
    int fd; /* File descriptor for GPIO port */
    char buf[29]; /* Write buffer */

    /* Make the GPIO port path */
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);

    /* Try to open GPIO port */
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        /* The file could not be opened */
        return false;
    }

    /* Set the port state */
    if (write(fd, (state == GPIO_HIGH ? "1" : "0"), 1) < 0) {
        return false;
    }

    /* Close the GPIO port */
    if (close(fd) < 0) {
        return false;
    }

    /* Success */
    return true;
}

/*
 * Get the state of the GPIO port.
 * @gpio the gpio pin to get the state from.
 * @return GPIO_HIGH if the pin is HIGH, GPIO_LOW if the pin is low. GPIO_ERR
 * when an error occured. 
 */
int gpio_get_state(int gpio) {
    int fd;             /* File descriptor for GPIO port */
    char buf[29];       /* Write buffer */
    char port_state; /* Character indicating the port state */
    int state; /* API integer indicating the port state */

    /* Make the GPIO port path */
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);

    /* Try to open GPIO port */
    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        /* The file could not be opened */
        return -1;
    }

    /* Read the port state */
    if (read(fd, &port_state, 1) < 0) {
        close(fd);
        return -1;
    }

    /* Translate the port state into API state */
    state = port_state == '1' ? GPIO_HIGH : GPIO_LOW;

    /* Close the GPIO port */
    if (close(fd) < 0) {
        return -1;
    }

    /* Return the state */
    return state;
}

/**
 * Release all GPIO ports from previous sessions. 
 */
void gpio_release_all() {
    int i;
    
    for(i = 1; i < 28; ++i) {
        if(gpio_config[i]) {
            gpio_release(i);
        }
    }
}

/**
 * Install the GPIO module in the duktape interpreter. 
 * @param ctx the duktape context to register the module in. 
 */
void gpio_install_dpt_module(duk_context *ctx)
{
    duk_push_global_object(ctx);
    duk_push_c_function(ctx, digitalWrite, DUK_VARARGS);
    duk_put_prop_string(ctx, -2, "digitalWrite");
    duk_pop(ctx);
    
    duk_push_global_object(ctx);
    duk_push_c_function(ctx, digitalRead, DUK_VARARGS);
    duk_put_prop_string(ctx, -2, "digitalRead");
    duk_pop(ctx);
    
    gpio_release_all();
}

/**
 * Set a GPIO port as output en write a state to it. 
 * @param ctx duktape context. 
 * @return 0 ('undefined') on success DUK_RET_XXX on error.
 */
duk_ret_t digitalWrite(duk_context *ctx)
{
    // Expect two arguments 
    int n = duk_get_top(ctx);
    if(n != 2) {
        return DUK_RET_API_ERROR;
    }
    
    int gpio = duk_to_int(ctx, 0);
    int state = duk_to_int(ctx, 1);
    
    // Check if GPIO is valid 
    if (gpio > 27 || !gpio_config[gpio]) {
        return DUK_RET_RANGE_ERROR;
    }
    
    if(!gpio_reserve(gpio)) {
        return DUK_RET_INTERNAL_ERROR;
    }
    
    if(!gpio_set_direction(gpio, GPIO_OUT) || !gpio_set_state(gpio, state)) {
        gpio_release(gpio);
        return DUK_RET_INTERNAL_ERROR;
    }
    
    if(!gpio_release(gpio)) {
        return DUK_RET_INTERNAL_ERROR;
    }
    
    return 0;
}

/**
 * Set a GPIO port as input and read a state from it.
 * @param ctx duktape context.
 * @return 1 return value on succes and 
 */
duk_ret_t digitalRead(duk_context *ctx)
{
    // Expect 1 arguments 
    int n = duk_get_top(ctx);
    if(n != 1) {
        return DUK_RET_API_ERROR;
    }
    int gpio = duk_to_int(ctx, 0);
    
    // Check if GPIO is valid 
    if (gpio > 27 || !gpio_config[gpio]) {
        return DUK_RET_RANGE_ERROR;
    }
    
    if(!gpio_reserve(gpio)) {
        return DUK_RET_INTERNAL_ERROR;
    }
    
    if(!gpio_set_direction(gpio, GPIO_IN)) {
        gpio_release(gpio);
        return DUK_RET_INTERNAL_ERROR;
    }
    
    int state = gpio_get_state(gpio);
    if(state == -1) {
        gpio_release(gpio);
        return DUK_RET_INTERNAL_ERROR;
    }

    if(!gpio_release(gpio)) {
        return DUK_RET_INTERNAL_ERROR;
    }
    
    duk_push_number(ctx, state);
    
    return 1;
}