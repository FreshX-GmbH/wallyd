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
 * File:   dpt_gpio.h
 * Created on October 5, 2015, 12:41 AM
 */

#ifndef DPT_GPIO_H
#define	DPT_GPIO_H

#include "../../../lib/duktape/src/duktape.h"

/* Direction macros */
#define GPIO_OUT	0		/* GPIO output direction */
#define GPIO_IN		1		/* GPIO input direction */

/* State macros */
#define GPIO_HIGH	1		/* GPIO high state */
#define GPIO_LOW	0		/* GPIO low state */

/**
 * Install the GPIO module in the duktape interpreter. 
 * @param ctx the duktape context to register the module in. 
 */
void gpio_install_dpt_module(duk_context *ctx);

/**
 * Set a GPIO port as output en write a state to it. 
 * @param ctx duktape context. 
 * @return 0 ('undefined') on success, DUK_RET_XXX on error.
 */
duk_ret_t digitalWrite(duk_context *ctx);

/**
 * Set a GPIO port as input and read a state from it.
 * @param ctx duktape context.
 * @return 1 return value on succes and 
 */
duk_ret_t digitalRead(duk_context *ctx);
#endif

