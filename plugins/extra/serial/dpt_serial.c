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
 * File:   dpt_serial.c
 * Created on November 21, 2015, 5:44 PM
 */

#include "../../../lib/duktape/src/duktape.h"
#include "dpt_serial.h"

#include <stdlib.h>

/**
 * Install the serial module in the duktape interpreter. 
 * @param ctx the duktape context to register the module in. 
 */
void serial_install_dpt_module(duk_context *ctx)
{
    const duk_function_list_entry serial_module_functions[] = {
        { "set_speed", serial_set_speed, DUK_VARARGS },
        { NULL, NULL, 0 }
    };
    
    duk_push_global_object(ctx);
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, serial_module_functions);
    duk_put_prop_string(ctx, -2, "SerialModule");
    duk_pop(ctx);
}

/**
 * Set the speed of the serial port. Expect one 
 * integer argument, the baud rate. 
 * @param ctx the duktape context. 
 * @return true on success, false on error for example when
 * the baud rate is not valid. 
 */
duk_ret_t serial_set_speed(duk_context *ctx)
{
    char* buff[256];
    
    // Expect 1 arguments 
    int n = duk_get_top(ctx);
    if(n != 1) {
        return DUK_RET_API_ERROR;
    }
    
    // Get the desired baud rate
    int baud = duk_to_int(ctx, 0);
    
    // Generate the stty command
    sprintf(buff, "stty -F /dev/ttyATH0 %d clocal cread cs8 -cstopb -parenb", baud);
    
    // Apply
    int result = system(buff);
    
    // Push the result to the stack
    duk_push_boolean(ctx, result == 0);
    return 1;
}