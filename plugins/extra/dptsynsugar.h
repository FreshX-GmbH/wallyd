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
 * File:   dptsynsugar.h
 * Created on October 5, 2015, 3:37 AM
 */

#ifndef DPTSYNSUGAR_H
#define	DPTSYNSUGAR_H

#define DPT_SYN_SUGAR   "function setTimeout(f, timeout){var timer = uv.new_timer.call({}); uv.timer_start(timer, timeout, 0, f); return timer;};" \
                        "function setInterval(f, timeout){var timer = uv.new_timer.call({}); uv.timer_start(timer,timeout,timeout,f);return timer;};" \
                        "function clearTimeout(tmr){uv.close.call({}, tmr, function(){});};" \
                        "function clearInterval(tmr){uv.close.call({}, tmr, function(){});};" \
                        "function I2C(e,i){this.i2c_sda=e,this.i2c_scl=i;var t=-1;this.begin=function(){var e=I2CModule.enable_device(this.i2c_sda,this.i2c_scl);t=I2CModule.open_bus(e)},this.setSlaveAddress=function(e){I2CModule.set_device(t,e)},this.writeByte=function(e){I2CModule.write_byte(t,e)}}" \
                        "function I2CLCD(t,i,n){var e=1,o=4,u=8,s=1,c=8,f=12,r=15,h=14,M=24,y=28,A=128,C=192,w=148,D=212,B=[A,C,w,D];this.n_cols=i,this.n_rows=n;var N=t,O=u,T=f,a=!0,m={DATA:0,COMMAND:1},l=function(t,i,n){var u=(15&t)<<4,s=240&t,c=O,f=0;i===m.DATA?(c|=e,f=1):f=5,N.writeByte(s|c),setTimeout(function(){N.writeByte(s|c|o),setTimeout(function(){N.writeByte(s|c),setTimeout(function(){N.writeByte(u|c),setTimeout(function(){N.writeByte(u|c|o),setTimeout(function(){N.writeByte(u|c),setTimeout(function(){\"function\"==typeof n&&n()},f)},1)},1)},f)},1)},1)};this.begin=function(t){N.writeByte(48),N.writeByte(48|o),setTimeout(function(){N.writeByte(48),N.writeByte(48|o),setTimeout(function(){N.writeByte(48),N.writeByte(48|o),setTimeout(function(){N.writeByte(32),N.writeByte(32|o),setTimeout(function(){l(40,m.COMMAND,function(){l(1,m.COMMAND,function(){l(12,m.COMMAND,t)})})},5)},1)},1)},5)},this.clear=function(t){l(s,m.COMMAND,t)},this.home=function(t){this.setCursor(0,0,t)},this.setCursor=function(t,i,n){var e=i%this.n_rows,o=t%this.n_cols;l(B[e]+o,m.COMMAND,n)};var p=function(t,i,n){t.length>i?l(t[i].charCodeAt(0),m.DATA,function(){p(t,i+1,n)}):\"function\"==typeof n&&n()};this.print=function(t,i){p(t,0,i)},this.display=function(t){a=!0,l(T,m.COMMAND,t)},this.noDisplay=function(t){a=!1,l(c,m.COMMAND,t)},this.cursor=function(t){T=h,a&&l(T,m.COMMAND,t)},this.noCursor=function(t){T=f,a&&l(T,m.COMMAND,t)},this.blink=function(t){T=r,a&&l(T,m.COMMAND,t)},this.backlight=function(t){O=u,N.writeByte(O),\"function\"==typeof t&&t()},this.noBacklight=function(t){O=0,N.writeByte(O),\"function\"==typeof t&&t()},this.shiftRight=function(t){l(y,m.COMMAND,t)},this.shiftLeft=function(t){l(M,m.COMMAND,t)}}" \
                        "function Serial(){var e=-1,t=null;this.begin=function(n,i){SerialModule.set_speed(n)?uv.fs_open(\"/dev/ttyATH0\",\"+\",777,function(n){e=n;var u=uv.new_pipe(!1);uv.pipe_open(u,e),uv.read_start(u,function(e,n){if(\"function\"==typeof t){var i=0;null===e&&(i=parseInt(Duktape.enc(\"hex\",n),16)),t(e,i)}}),i(!0)}):i(!1)},this.onReceive=function(e){t=e},this.writeByte=function(t){var n=Duktape.Buffer(1);n[0]=t,uv.fs_write(e,n,-1)},this.writeString=function(t){uv.fs_write(e,t,-1)}}"
#endif

