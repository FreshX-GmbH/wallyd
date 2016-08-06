'use strict';

var date = new Date();
var config = wally.getConfig();
var wallaby = require('./modules/wallaby.js');
var utils = require('./modules/utils.js');
var log = require('./modules/log.js');

var file = config.basedir+'/etc/wallyd.d/tests/revo.json';
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


var a = uv.new_timer();
uv.timer_start(a, 1000, loopDelay, oninterval);

function oninterval() {
    log.debug('Render wallaby');
    delete date;
    date = new Date();
    try{
       json = wally.readFile(file);
       data = JSON.parse(json);
       //wally.evalFile(valsfile);
    } catch(err) {
        log.debug('Error loading and parsing screen file : '+err);
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
    context.privates.time = utils.pad(date.getHours(),2)+':'+utils.pad(date.getMinutes(),2)+':'+utils.pad(date.getSeconds(),2);
    wallaby.renderScreen(context,context.privates,'main',data);
}
