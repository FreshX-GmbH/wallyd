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
 * File:   dpt_i2c.c
 * Created on November 13, 2015, 3:24 PM
 */

#include "../../../lib/duktape/src/duktape.h"
#include "dpt_i2c.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdint.h>

/**
 * Install the I2C module in the duktape interpreter. 
 * @param ctx the duktape context to register the module in. 
 */
void i2c_install_dpt_module(duk_context *ctx)
{
    const duk_function_list_entry i2c_module_functions[] = {
        { "enable_device", i2c_enable_device, DUK_VARARGS },
        { "open_bus", i2c_open_bus, DUK_VARARGS },
        { "set_device", i2c_set_device, DUK_VARARGS },
        { "write_byte", i2c_write_byte, DUK_VARARGS },
        { NULL, NULL, 0 }
    };
    
    duk_push_global_object(ctx);
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, i2c_module_functions);
    duk_put_prop_string(ctx, -2, "I2CModule");
    duk_pop(ctx);
}

/**
 * Enable the I2C bus on 2 GPIO pins. Expect two integer 
 * arguments, this first is the SDA gpio pin number and
 * the second is the SCL gpio pin number. 
 * @param ctx the duktape context. 
 * @return the name of the created i2c device. 
 */
duk_ret_t i2c_enable_device(duk_context *ctx)
{
    char* buff[256];
    
    // Expect 2 arguments 
    int n = duk_get_top(ctx);
    if(n != 2) {
        return DUK_RET_API_ERROR;
    }
    
    // Get the SDA and SDL pin number 
    int sda_pin = duk_to_int(ctx, 0);
    int scl_pin = duk_to_int(ctx, 1);
    
    // Generate the insmod command
    sprintf(buff, "insmod i2c-gpio-custom bus0=0,%d,%d > /dev/null", sda_pin, scl_pin);
    
    // Insert the kernel modules 
    system("insmod i2c-dev > /dev/null");
    system(buff);
    
    // Push the name of the i2c device to the stack.
    duk_push_string(ctx, "/dev/i2c-0");
    return 1;
}

/**
 * Open an i2c bus from the file system. Expect one string
 * argument giving the i2c device file. 
 * @param ctx the duktape context. 
 * @return 1 string pushed on the stack.  
 */
duk_ret_t i2c_open_bus(duk_context *ctx)
{
    // Expect 1 argument
    int n = duk_get_top(ctx);
    if(n != 1) {
        return DUK_RET_API_ERROR;
    }
    
    // Get the device path from the stack
    const char* device = duk_to_string(ctx, 0);
    
    // Try to open the I2C device 
    int fd = open(device, O_RDWR);
    if(fd < 0) {
        return DUK_RET_INTERNAL_ERROR;
    }
    
    // Return the file descriptor 
    duk_push_int(ctx, fd);
    return 1;
}

/**
 * Initialize an I2C device on the bus. This function expects 
 * a file descriptor as its first argument and an I2C slave address
 * as it's second argument. 
 * @param ctx the duktape context. 
 * @return success status. 
 */
duk_ret_t i2c_set_device(duk_context *ctx)
{
    // Expect 2 arguments
    int n = duk_get_top(ctx);
    if(n != 2) {
        return DUK_RET_API_ERROR;
    }
    
    // Get the arguments 
    int fd = duk_to_int(ctx, 0);
    uint8_t addr = (uint8_t) duk_to_uint(ctx, 1);
    
    // Init the device
    if(ioctl(fd, I2C_SLAVE, addr) < 0) {
        return DUK_RET_INTERNAL_ERROR;
    }
    
    // 0 arguments pushed to the stack
    return 0;
}

/**
 * Write to an I2C bus device. This function expects a file
 * descriptor as its first argument and a byte of data
 * as its second argument. 
 * @param ctx duktape context. 
 * @return 0 ('undefined') on success DUK_RET_XXX on error.
 */
duk_ret_t i2c_write_byte(duk_context *ctx)
{
    // Expect two arguments 
    int n = duk_get_top(ctx);
    if(n != 2) {
        return DUK_RET_API_ERROR;
    }
    
    int fd = duk_to_int(ctx, 0);
    uint8_t data = (uint8_t) duk_to_uint(ctx, 1);
    
    int ret = write(fd, &data, 1);
    if(ret < 1) {
        return DUK_RET_INTERNAL_ERROR;
    }
    
    return 0;
}