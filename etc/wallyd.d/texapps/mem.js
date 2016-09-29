'use strict';

var wally = new Wally();
var gui = new GUI();
var date = new Date();
var uv = nucleus.uv;

var memstartb = wally.getrss();
var memstart = Math.round((memstartb/1024/1024)*100)/100+'mb';
var start = uv.hrtime();
var div = 1;

function oninterval() {
    try {
	var mymem = wally.getrss();
	div = uv.hrtime()-start;
	var grow = Math.ceil((mymem-memstartb)/div);
    	var stat = 'MemSt: '+memstart+
		 ' / Cur: '+Math.ceil((mymem/(1024*1024))*100)/100+'mb'+
		 ' / Gr: '+grow+'b/s';
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
    memtimer.start( 0, 100, oninterval);
} catch(e) {
    //log.error('Error in memdbg timer : '+e);
}
