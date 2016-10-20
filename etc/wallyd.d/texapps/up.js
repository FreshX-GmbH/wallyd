'use strict';

var wally = new Wally();
var gui = new GUI();
var date = new Date();
var uv = nucleus.uv;

log.info('Started up texapp');

var loopDelay = 2000;

function oninterval() {
    try {
	var d2 = new Date();
	var passed = 0;
	passed = d2.getTime()-date.getTime();//-3600*1000;
	d2.setTime(passed);
	var m = extra.pad(d2.getMinutes(),2);
	var h = d2.getHours()-1;
	var d = ~~((passed/1000)/24/3600);
	var uts = h+':'+m+'h';
	if(d >= 1){
	   uts = d+' days '+h+':'+m+'h';
	}
    	var stat = '***   Up: '+uts;
	var TA=new Transaction();
	TA.push( gui.clearTexture.bind(null,'up'));
       	TA.push( wally.setText.bind(null,'up','black','logfont',0,1,stat));
	TA.commit();
    } catch(err) {
	log.error('ERROR: Show status failed : '+err);
    }
}

try {
    var timer = new uv.Timer();
    timer.start( 0, loopDelay, oninterval);
} catch(e) {
    log.error('Error in demo timer : '+e);
}
