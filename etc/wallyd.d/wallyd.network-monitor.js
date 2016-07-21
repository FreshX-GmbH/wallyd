'use strict';

var utils = require('./modules/utils.js');
var p = utils.prettyPrint;

function assert(cond, message) {
  if (!cond) {
    throw new Error(message || 'Assertion Failure');
  }
}

//var nwm = uv.new_timer();
//uv.timer_start(nwm, 1000, 1000, monitor, 2);
//
//function monitor() {
//  p("moninterval", nwm);
//}
//
