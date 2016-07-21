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
 * File:   dpt_i2c.h
 * Created on November 13, 2015, 10:07 AM
 */

#ifndef DPT_I2C_H
#define	DPT_I2C_H

#include "../../../lib/duktape/src/duktape.h"

/**
 * Install the I2C module in the duktape interpreter. 
 * @param ctx the duktape context to register the module in. 
 */
void i2c_install_dpt_module(duk_context *ctx);

/**
 * Enable the I2C bus on 2 GPIO pins. Expect two integer 
 * arguments, this first is the SDA gpio pin number and
 * the second is the SCL gpio pin number. 
 * @param ctx the duktape context. 
 * @return the status. 
 */
duk_ret_t i2c_enable_device(duk_context *ctx);

/**
 * Open an i2c bus from the file system. Expect one string
 * argument giving the i2c device file. 
 * @param ctx the duktape context. 
 * @return 1 number pushed on the stack.  
 */
duk_ret_t i2c_open_bus(duk_context *ctx);

/**
 * Initialize an I2C device on the bus. This function expects 
 * a file descriptor as its first argument and an I2C slave address
 * as it's second argument. 
 * @param ctx the duktape context. 
 * @return success status. 
 */
duk_ret_t i2c_set_device(duk_context *ctx);

/**
 * Write to an I2C bus device. This function expects a file
 * descriptor as its first argument and a byte of data
 * as its second argument. 
 * @param ctx duktape context. 
 * @return 0 ('undefined') on success DUK_RET_XXX on error.
 */
duk_ret_t i2c_write_byte(duk_context *ctx);

#endif

