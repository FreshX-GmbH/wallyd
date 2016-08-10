'use strict';

var wally = new Wally();
var config = wally.getConfig();
//var utils = require('./modules/utils');
//var log = nucleus.dofile('./modules/log.js');
var gui = new GUI();
var date = new Date();
var uv = nucleus.uv;

var file = config.basedir+'/etc/wallyd.d/tests/wallybill.json';
var valsfile = "/tmp/co2.js";

var loopDelay = 600;

context.privates = {};
context.privates.co2 = 520;
context.privates.co2temp = 24;
context.privates.hrtemp = 25;
context.privates.hr = 44;
context.privates.kw = 23;
context.privates.date = date.getDate()+'.'+date.getMonth()+'.'+date.getFullYear();
context.privates.time = date.getHours()+':'+date.getMinutes();

var page=0;
var timer = new uv.Timer();
timer.start( 1000, loopDelay, oninterval);


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
    wallaby.renderScreen(context,context.privates,'main',dat);
}

uv.run();
