'use strict';

var wally, wallaby, gui, data, header;

var loopDelay = 10000;

// for direct test in nucleus
if(typeof(Wally) === 'undefined')
{
        context = nucleus.dofile('modules/compat.js');
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
var ukurl="https://"+host+":"+port+"/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==3";
var errurl="https://"+host+":"+port+"/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==2";
var warnurl="https://"+host+":"+port+"/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==1";
var okurl="https://"+host+":"+port+"/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==0";
var file = config.wally.basedir+'/etc/wallyd.d/tests/icinga.json';

context.privates = {};

function oninterval() {
   log.debug('Render wallaby');
   var eObj,wObj,oObj,uObj;
   try{
        var e  = curl.get(ukurl ,header);
        if(e.body){
	   uObj = JSON.parse(e.body);
	   log.info("Unknown Object parsed",uObj.results.length);
	} 
        context.privates.ukn = uObj.results.length;
   }catch(err){
	   log.error("Could not parse icinga err response : "+err);
	   log.debug(e);
   }
   try{
        var e  = curl.get(errurl ,header);
        if(e.body){
	   eObj = JSON.parse(e.body);
	   log.info("Err Object parsed",eObj.results.length);
	} 
        context.privates.down = eObj.results.length;
   }catch(err){
	   log.error("Could not parse icinga err response : "+err);
	   log.debug(e);
   }
   try{
      var w = curl.get(warnurl,header);
      if(w.body){
	   wObj = JSON.parse(w.body);
	   log.info("Warn Object parsed",wObj.results.length);
      } else {
	   log.error("No valid response from Server");
      }
      context.privates.warn = wObj.results.length;
   } catch(err){
	   log.error("Could not parse icinga warn response : "+err);
	   log.debug(e);
   }
   try{
      var o = curl.get(okurl  ,header);
      if(o.body){
	   var oObj = JSON.parse(o.body);
	   log.info("OK Object parsed : ",oObj.results.length);
      } else {
	   log.error("No valid response from Server");
      }
      context.privates.up = oObj.results.length;
   } catch(err){
	   log.error("Could not parse icinga ok response : "+err);
	   log.debug(e);
   }
   try {
        wally.startTransaction();
        gui.clearTextureNoPaint('main');
        wallaby.renderScreen(context,context.privates,'main',data.pages[0]);
        wally.render('main');
        wally.commitTransaction();
    } catch(err) {
	log.error('ERROR: Show status failed : '+err);
    }
}

if(curl){
	header = curl.mkBasicAuth(user,password);
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
