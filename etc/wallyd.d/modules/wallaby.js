'use strict';

var gui = new GUI();
var wally = new Wally();
var config = wally.getConfig();
var utils = require('./utils.js');
var log = require('./log.js');
var curl = require('./curl.js');
var qr = require('./qr/qr.js');
var p = utils.prettyPrint;

function parseString(str){
    if( str === undefined ) return "";
    var matchTable={};
    var splitter = /\$_.[A-Za-z0-9_]*;/g;
    var match = str.match(splitter);
    if(match === null) return str;
    for (var j = 0; j<match.length; j++)
    {
        var smatch = match[j].replace(/\$_./,'').replace(/;$/,'');
        log.debug(smatch);
            if(context.privates && context.privates[smatch]){
                val = context.privates[smatch].toString();
                matchTable[smatch] = val;
            } else {
            log.error("Key ",smatch," not found in context.privates");
        }
    }
    var destVal = str;
    for (var k in matchTable){
        var re=RegExp('._.'+k+';');
        var t = destVal.replace(re,matchTable[k]);
//        log.debug(t);
        destVal = t;
    }
    return destVal;
}

function renderScreen(context, tree, screen, data)
{
   var svg = new SVG();
   var json,data;
   var maxWidth=0, maxHeight=0;
   var xScale=1.0, yScale=1.0;
   var rX=0, rY=0;
   var start = uv.hrtime();
	
   data = data.pages[0];

   // try{
   //    json = wally.readFile(file);
   //    data = JSON.parse(json);
   // } catch(err) {
   //     log.debug('Error loading and parsing screen file : '+err);
   //     return;
   // }
 
   wally.setAutoRender(false);
   gui.setTargetTexture(screen);

    xScale = (config.width-10)/data._options.width;
    yScale = (config.height-30)/data._options.height;
    
    for(var i = 0; i < data.objects.length; i++){
        var obj = data.objects[i].options;
	var value = data.objects[i].value;
        var X = ~~((obj.geometry.x+rX)*xScale);
        var Y = ~~((obj.geometry.y+rY)*yScale);
        var W = 0;
        var H = 0;
        var color = 0;
        var alpha = 255;
        var fillColor = 0;
        var gradientColor = undefined;
        if(obj.geometry.width){
            W = ~~(obj.geometry.width*xScale);
        }
        if(obj.geometry.height){
            H = ~~(obj.geometry.height*yScale);
        }
        if(obj.style.strokeColor) {
            color= parseInt(obj.style.strokeColor.replace('#','0x'));
        }
        if(obj.style.fontColor) {
            color= parseInt(obj.style.fontColor.replace('#','0x'));
        }
        if(obj.style.fillColor) {
            fillColor= parseInt(obj.style.fillColor.replace('#','0x'));
        } else {
            alpha = 0;
        }
        if(obj.style.gradientColor) {
            gradientColor = parseInt(obj.style.gradientColor.replace('#','0x'));
        }

        //utils.dumpJSON(obj);
    
        if(obj.style.text === true){
            var h=~~(obj.geometry.height*yScale);
            var fName = 'font'+h;
            var val = "";
            wally.loadFont(fName, config.basedir+'/etc/wallyd.d/fonts/Lato-Bol.ttf', h);
            if(value.match(/\$_./)){
               var destVal = parseString(value);
               gui.drawText(screen,X, Y, fName, color, destVal);
            } else {
               gui.drawText(screen,X, Y, fName, color, value);
            }
        } else if(obj.style.endArrow !== undefined) {
            log.debug('Dont know how to handle object connectors yet.');
            continue;
        } else if(obj.style.line === true){
            log.debug('Line : ',JSON.stringify(obj.geometry));
            gui.drawLine(screen,X, Y, X + W, Y, 0);
        } else {
            var stroke = obj.style.stroke || 1;
            if (obj.style === {} || obj.geometry === {}){
                continue;
            }
            if(gradientColor){
                log.debug('Gradient box : ',JSON.stringify(obj.geometry));
                gui.drawGradient(screen, X, Y, W, H, gradientColor, fillColor, true, false);
            } else {
                if(obj.value && obj.value.match(/^qr:/)){
                    var qrtext = parseString(obj.value.replace('qr:',''));
                    log.debug('Box is QR Code : ',qrtext);
                    //var res = qr.image(qrtext, { type: 'svg' });
                    //log.debug(res);
                  var res = qr.file(qrtext,'/tmp/qr.svg', { type: 'svg' });
                  svg.svgToPng("/tmp/qr.svg","/tmp/qr.png");
                  log.debug('Placing QR Code at : ',X,Y,W,H);
                  gui.loadImage(screen,'/tmp/qr.png',X,Y,W,H,255);
                    //var ptr = svg.svgToImage(res.toString());
                    //var preview = Array.prototype.slice.call(ptr, 0, ptr.length).map(function (byte) {
                    //    return byte < 16 ? "0" + byte.toString(16) : byte.toString(16);
                    //}).join(" ");
                    //log.debug(preview);
                    //gui.putImage(screen,ptr,X,Y,W,H,255);
                    ////log.debug(preview);
                    //gui.putImage(screen,ptr,X,Y,W,H,255);
                    continue;
                } else {
                    log.debug('Filled box : ',JSON.stringify(obj.geometry));
                    gui.drawFilledBox(screen, X, Y, W, H, stroke, fillColor, color, alpha);
                    // TODO : print text
                    log.debug('Box has text : ',obj.value);
                }
            }
        }
    }
    
    wally.render(screen);
    
    end = uv.hrtime()-start;
    log.debug({'time': end /1000000000 });
    log.debug('Wallaby Screen has max size '+maxWidth+'x'+maxHeight+' Scaling by '+xScale+'x'+yScale+' Relocating by '+rX+'/'+rY);
}

function drawCO2JSON(screen,chunk){
    var logs = JSON.parse(chunk);
    var co = [];
    var max = 0;
    log.debug(logs.rows.length);
    for(var i = 0; i < logs.rows.length; i++){
      var obj = logs.rows[i].doc;
      var time = obj._id.split(':')[1];
      co.push([ time, obj.temp, obj.co2 ]);
      if(max < obj.co2) max =obj.co2;
    }
    var rel = config.height/(2*max);
    var colrel = 255/max;
    //print(rel);
    co.sort(function(a, b) {return a[0] - b[0]})
    var color;
    for(var i = 0; i < co.length; i++){
        if((i % 20) == 0){
            gui.resetTargetTexture();
            wally.render(screen);
            gui.setTargetTexture(screen);
            time = uv.hrtime();
            log.debug({'delay': (time-start)/1000000000});
        }
        color = 0x30FF00;
        if(co[i][2] > 800){
            color = 0xFFC000;
        }
        if(co[i][2] > 1200){
            color = 0xFF4D00;
        }
        gui.drawLine(screen,i,config.height,i,config.height-co[i][2]*rel, color);
        gui.drawLine(screen,i,0,i,co[i][1], color);
    }
}

exports.renderScreen = renderScreen;
