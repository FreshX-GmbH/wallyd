'use strict';

var wally = new Wally();
var gui = new GUI();
var date = new Date();
var uv = nucleus.uv;

var file = config.wally.basedir+'/etc/wallyd.d/tests/wallybill.json';
var valsfile = "/tmp/co2.js";

var loopDelay = 1000;
var memstartb = wally.getrss();
var memstart = Math.round((memstartb/1024/1024)*100)/100+'mb';

context.privates = {};
context.privates.co2 = 520;
context.privates.co2temp = 24;
context.privates.hrtemp = 25;
context.privates.hr = 44;
context.privates.kw = 23;
context.privates.date = date.getDate()+'.'+date.getMonth()+'.'+date.getFullYear();
context.privates.time = date.getHours()+':'+date.getMinutes();

var page=0;

function oninterval() {
    var wallaby = require('./modules/wallaby');
    log.debug('Render wallaby');
    try{
       var json = wally.readFile(file);
       var data = JSON.parse(json);
    } catch(err) {
        log.debug('Error loading and parsing screen file ',file,' : '+err);
        return;
    }
    if(context.privates.co2 > 800) {
         for(var i = 0; i < data.objects.length; i++){
             var obj = data.objects[i];
             if(obj.value === "CO2" || obj.value === "$_.co2; ppm"){
                 if(obj.style.fontColor){
                    if(context.privates.co2 > 1200) {
                        obj.style.fontColor="#FF6666";
                    } else {
                        obj.style.fontColor="#FFF020";
                    }
                 }
             }
         }
    }
    context.privates.date = date.getDate()+'.'+date.getMonth()+'.'+date.getFullYear();
    context.privates.time = extra.pad(date.getHours(),2)+':'+extra.pad(date.getMinutes(),2)+':'+extra.pad(date.getSeconds(),2);
    var dat = data.pages[page];
    if(data.pages.length > 1){
	page++;	
	if(page > data.pages.length-1) { page = 0;}
    }
    log.info("Presenting page : "+page+" of "+data.pages.length+" with "+dat.objects.length+" elements");
    gui.clearTextureNoPaint('main');
    try {
	var d2 = new Date();
        var start = d2.getTime();
        wallaby.renderScreen(context,context.privates,'main',dat);
	if(typeof uv.uptime === 'function'){
	    var d2.setTime(uv.uptime());	
	} else {
	    var passed = d2.getTime()-config.wally.uptime*1000;//-3600*1000;
	}
	var d3 = new Date();
	var fin = d3.getTime();
	d2.setTime(passed);
	var m = extra.pad(d2.getMinutes(),2);
	var h = d2.getHours();
	var d = (passed/1000)/24/3600;
	var uts = h+':'+m+'h';
	var name = 'unknown';
	var conn = 'no';
	if(d > 0){
	   uts = d+' days '+h+':'+m+'h';
	}
	if(typeof(config.conn) !== 'undefined' && typeof(config.conn.name) !== 'undefined' ){
		name=config.conn.name;
	}
	if(typeof(config.conn) !== 'undefined' && typeof(config.conn.host) !== 'undefined' ){
		conn=config.conn.host;
	}
	var mymem = wally.getrss();
	var grow = Math.ceil((memstartb-mymem)/passed);
    	var stat = 'WallyTV  v'+config.wally.release/1000+
		 '   ***   Res: '+config.wally.width+'x'+config.wally.height+
		 '   ***   Name: '+name+
		 '   ***   Arch: '+config.wally.arch+
		 '   ***   Connected: '+conn+
		 '   ***   Mem start: '+memstart+
		 '   ***   Mem curr: '+Math.ceil((mymem/(1024*1024))*100)/100+'mb'+
		 '   ***   Mem grow: '+grow+'b/s'+
		 '   ***   Up: '+uts+
		 '   ***   Render time: '+(fin-start)/1000+'s';
	log.debug(stat);
    	wally.log(stat);
    } catch(err) {
	log.error('ERROR: Show status failed : '+err);
    }
}

try {
    var timer = new uv.Timer();
    timer.start( 1000, loopDelay, oninterval);
} catch(e) {
    log.error('Error in demo timer : '+e);
}
