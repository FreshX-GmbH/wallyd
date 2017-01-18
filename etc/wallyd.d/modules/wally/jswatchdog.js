'use strict';

var wally = new Wally();
var uv = nucleus.uv;

function oninterval() {
    try {
	var now = new Date().getTime();
	log.debug('JSWatchdog alive : ',now);
    } catch(err) {
	log.error('Error in jswatchdog : '+err);
    }
}

try {
    var memtimer = new uv.Timer();
    memtimer.start( 0, 10000, oninterval);
} catch(e) {
    log.error('Error in jswatchdog timer : '+e);
}
