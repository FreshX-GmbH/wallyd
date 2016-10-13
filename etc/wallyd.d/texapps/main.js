'use strict';

var wally, wallaby, gui;
// for direct test in nucleus
if(typeof(Wally) === 'undefined')
{
        var context = nucleus.dofile('modules/compat.js');
	gui = wally = context.wally;
        nucleus.uv.run();
} else  {
	wally = new Wally();
	gui = new GUI();
	wallaby = require('./modules/wallaby');
}

var date = new Date();
var curl = nucleus.dofile('modules/curl.js');

var user="dashing";
var password="icinga2ondashingr0xx";
var host="monitor.int.freshx.de";
var port=5665;
var errurl="https://"+host+":"+port+"/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==2";
var warnurl="https://"+host+":"+port+"/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==1";
var okurl="https://"+host+":"+port+"/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==0";
var file = config.wally.basedir+'/etc/wallyd.d/tests/icinga.json';
var loopDelay = 1000;
var data,header;

function oninterval() {
   log.debug('Render wallaby');
   var err  = curl.get(errurl,header);
   var warn = curl.get(warnurl,header);
   var ok   = curl.get(okurl,header);
   if(err.body){
	log.info(err.body);
   }
   if(warn.body){
	log.info(warn.body);
   }
   if(ok.body){
	log.info(ok.body);
   }
   //if(context.co2 > 800) {
   //      for(var i = 0; i < data.objects.length; i++){
   //          var obj = data.objects[i];
   //          if(obj.value === "CO2" || obj.value === "$_.co2; ppm"){
   //              if(obj.style.fontColor){
   //                 if(context.privates.co2 > 1200) {
   //                     obj.style.fontColor="#FF6666";
   //                 } else {
   //                     obj.style.fontColor="#FFF020";
   //                 }
   //              }
   //          }
   //      }
   // }
   var dat = data.pages[page];
   var dat2 = data.pages[page+1];
   if(data.pages.length > 1){
	page++;	
	if(page > data.pages.length-1) { page = 0;}
	if(page > data.pages.length-2) { dat2 =  data.pages[0]; }
   }
   log.info("Presenting page : "+page+" of "+data.pages.length+" with "+dat.objects.length+" elements");
   try {
        wally.startTransaction();
        gui.clearTextureNoPaint('main');
        wallaby.renderScreen(context,context.privates,'main',dat);
        wally.render('main');
        wally.commitTransaction();
    } catch(err) {
	log.error('ERROR: Show status failed : '+err);
    }
}

if(curl){
	header = curl.mkBasicAuth(user,password);
	log.info(header);	

	try{
	    var json = wally.readFile(file);
	    data = JSON.parse(json);
	} catch(err) {
	    log.debug('Error loading and parsing screen file ',file,' : '+err);
	}
	
	try {
	    var timer = new uv.Timer();
	    timer.start( 0, loopDelay, oninterval);
	} catch(e) {
	    log.error('Error in icinga timer : '+e);
	}
}
