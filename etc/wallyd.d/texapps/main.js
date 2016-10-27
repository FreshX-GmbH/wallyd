'use strict';

var wally, wallaby, gui, data, couchheader,icingaheader;

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
var settings = context.settings;
var couchdb  = settings.couchdb;
var icinga   = settings.icinga;
var ukurl='https://'+icinga.host+':'+icinga.port+'/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==3';
var errurl='https://'+icinga.host+':'+icinga.port+'/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==2';
var warnurl='https://'+icinga.host+':'+icinga.port+'/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==1';
var okurl='https://'+icinga.host+':'+icinga.port+'/v1/objects/services?attrs=display_name&joins=host.address&filter=service.state==0';
var co2url='http://'+couchdb.user+':'+couchdb.password+'@'+couchdb.host+':'+couchdb.port+'/co2/_all_docs?limit=10&descending=true&include_docs=true';
var file = config.wally.basedir+'/etc/wallyd.d/tests/icinga.json';

context.privates = {};
context.privates.co2U='000';
context.privates.co2D='000';
context.privates.tempU='00';
context.privates.tempD='00';

function oninterval() {
   log.debug('Render wallaby',settings);
   var eObj,wObj,oObj,uObj,co2Obj;
   try{
        var e  = curl.get(co2url ,couchheader);
        if(e.body){
	   co2Obj = JSON.parse(e.body);
	   log.info('CO2 Object parsed');
	   if(co2Obj.rows[0]){
	      var rows = co2Obj.rows;
	      var U=false,D=false;
	      for(var i=0; i<rows.length; i++) {
		  var val = rows[i];
		  var ip = val.id.split(':')[1];
		  log.info(ip,'/',U,'/',D,settings.co2.U,settings.co2.D);
		  if(!U && ip === settings.co2.U){
		     U=true;
		     context.privates.co2U=val.doc.co2;
		     context.privates.tempU=Math.round(val.doc.temp*10)/10;
		     log.info('Found U : ',val);
		  }
		  if(!D && ip === settings.co2.D){
		     D=true;
		     context.privates.co2D=val.doc.co2;
		     context.privates.tempD=Math.round(val.doc.temp*10)/10;
		     log.info('Found D',val);
		  }
		  if(D & U) break;
	      }
	   }
	} 
   }catch(err){
	   log.error("Could not parse couch response : "+err);
	   log.debug(e);
   }

   try{
        var e  = curl.get(ukurl ,icingaheader);
        if(e.body){
	   uObj = JSON.parse(e.body);
	   log.info('Unknown Object parsed',uObj.results.length);
	} 
        context.privates.ukn = uObj.results.length;
   }catch(err){
	   log.error("Could not parse icinga err response : "+err);
	   log.debug(e);
   }
   try{
        var e  = curl.get(errurl ,icingaheader);
        if(e.body){
	   eObj = JSON.parse(e.body);
	   log.debug(JSON.stringify(eObj,0,2));
	   log.info('Err Object parsed',eObj.results.length);
	} 
        context.privates.down = eObj.results.length;
   }catch(err){
	   log.error("Could not parse icinga err response : "+err);
	   log.debug(e);
   }
   try{
      var w = curl.get(warnurl,icingaheader);
      if(w.body){
	   wObj = JSON.parse(w.body);
	   log.info('Warn Object parsed',wObj.results.length);
      } else {
	   log.error('No valid response from Server');
      }
      context.privates.warn = wObj.results.length;
   } catch(err){
	   log.error("Could not parse icinga warn response : "+err);
	   log.debug(e);
   }
   try{
      var o = curl.get(okurl  ,icingaheader);
      if(o.body){
	   var oObj = JSON.parse(o.body);
	   log.info('OK Object parsed : ',oObj.results.length);
      } else {
	   log.error('No valid response from Server');
      }
      context.privates.up = oObj.results.length;
   } catch(err){
	   log.error("Could not parse icinga ok response : "+err);
	   log.debug(e);
   }
   log.info('CO2 : '+context.privates.co2U);
   for(var i = 0; i < data.pages[0].objects.length; i++){
       var obj = data.pages[0].objects[i];
       log.debug(obj.value);
       if(obj.value === "co2U"){
           if(obj.options.style.fillColor){
              if(context.privates.co2U > 1200) {
                  obj.options.style.fillColor="#FF0000";
              } else if (context.privates.co2U < 800) {
                  obj.options.style.fillColor="#339933";
	      } else {
                  obj.options.style.fillColor="#FF8000";
              }
           }
       }
   }

   try {
        var renderTA = new Transaction();
        renderTA.push(gui.clearTextureNoPaint.bind(null,'main'));
        wallaby.renderScreen(renderTA,context,context.privates,'main',data.pages[0]);
        renderTA.push(wally.render.bind(null,'main'));
        renderTA.commit();
    } catch(err) {
	log.error('ERROR: Show wallaby screen failed : '+err);
    }
}

if(curl){
	icingaheader = curl.mkBasicAuth(icinga.user,icinga.password);
	couchheader = curl.mkBasicAuth(couchdb.user,couchdb.password);
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
