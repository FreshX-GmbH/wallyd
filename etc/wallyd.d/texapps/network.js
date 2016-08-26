'use strict';

function assert(cond, message) {
  if (!cond) {
    throw new Error(message || 'Assertion Failure');
  }
}
print('network');
//var nwm = uv.new_timer();
//uv.timer_start(nwm, 1000, 1000, monitor, 2);
//
//function monitor() {
//  p("moninterval", nwm);
//}
//
