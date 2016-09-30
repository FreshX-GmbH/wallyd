'use strict';

var wally = new Wally();
var gui = new GUI();
var date = new Date();
var uv = nucleus.uv;
var date = new Date();

var memstartb = wally.getrss();
var memstart = Math.round((memstartb/1024/1024)*100)/100+'mb';
var start = uv.hrtime();
var div = 1;

log.error('MemDBG');

function oninterval() {
    try {
	var now = new Date();
	var mymem = wally.getrss();
	div = now.getTime()-config.wally.uptime*1000;
	var grow = Math.ceil((mymem-memstartb)/div);
    	var stat = 'Mem: '+memstart+
		 ' / = '+Math.ceil((mymem/(1024*1024))*100)/100+'mb'+
		 ' / + '+grow+'b/s';
	wally.startTransaction();
        gui.clearTexture('memdbg');
	wally.setText('memdbg','black','logfont',0,1,stat);
	wally.commitTransaction();
	//log.error('memdbg black logfont 0  1 ',stat);
    } catch(err) {
	//log.error('Error in memdbg : '+err);
    }
}

try {
    var memtimer = new uv.Timer();
    memtimer.start( 0, 2000, oninterval);
} catch(e) {
    //log.error('Error in memdbg timer : '+e);
}
