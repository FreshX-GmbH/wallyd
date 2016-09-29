'use strict';

//var qr = require('modules/qr/qr');
var uv = nucleus.uv;
var gui = new GUI();
var wally = new Wally();
var config = wally.getConfig();

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
             var val = context.privates[smatch].toString();
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
   var json;
   var maxWidth=0, maxHeight=0;
   var xScale=1.0, yScale=1.0;
   var rX=0, rY=0;
   var start = new Date().getTime();

   //log.info(data);

   wally.setAutoRender(false);
   gui.setTargetTexture(screen);

   var width = data._options.width ? data._options.width : data._options.size[0];
   var height = data._options.height ? data._options.height : data._options.size[1];
   xScale = (config.width-10)/width;
   yScale = (config.height-30)/height;
    
   for(var i = 0; i < data.objects.length; i++){
        var obj = data.objects[i];
	var opts = obj.options;
	var value = obj.value;
        var X = ~~((opts.geometry.x+rX)*xScale);
        var Y = ~~((opts.geometry.y+rY)*yScale);
        var W = 0;
        var H = 0;
        var color = 0;
        var alpha = 255;
        var fillColor = 0;
        var gradientColor = undefined;
        if(opts.geometry.width){
            W = ~~(opts.geometry.width*xScale);
        }
        if(opts.geometry.height){
            H = ~~(opts.geometry.height*yScale);
        }
        if(opts.style.strokeColor) {
            color= parseInt(opts.style.strokeColor.replace('#','0x'));
        }
        if(opts.style.fontColor) {
            color= parseInt(opts.style.fontColor.replace('#','0x'));
        }
        if(opts.style.fillColor) {
            fillColor= parseInt(opts.style.fillColor.replace('#','0x'));
        } else {
            alpha = 0;
        }
        if(opts.style.gradientColor) {
            gradientColor = parseInt(opts.style.gradientColor.replace('#','0x'));
        }
   
        if(obj.type === 'image'){
            //var res = curl.get(obj.path);
            //if(res.body){
            //   wally.writeFileSync('/tmp/test.png',res.body);
            //   //gui.loadImage(screen,'/tmp/test.png',X, Y, W/xScale, H/yScale, 255);
            //}
            //gui.loadImage(screen,'/tmp/test.png',X, Y, W, H, 255);
            //continue;
        }
        if(obj.type === 'line'){
            gui.drawLine(screen,X, Y, X + W, Y, 0);
            continue;
	}
	// TODO
        if(obj.type === 'rect'){
	//    if(opts.edge && opts.edge > 0 ){
	//    	log.error(screen, X, Y, W, H, parseInt(opts.edge), fillColor, color, alpha);
	//    	gui.drawFilledBox(screen, X, Y, W, H, parseInt(opts.edge), fillColor, color, alpha);
	//    }
            continue;
	}
        if(obj.type === 'qr'){
	    log.debug('QR not yet ported');
	    continue;
            var qrtext = parseString(opts.value.replace('qr:',''));
            //var res = qr.image(qrtext, { type: 'svg' });
            //log.debug(res);
            //var res = qr.file(qrtext,'/tmp/qr.svg', { type: 'svg' });
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
	}

        if(obj.type === 'text'){
	    if(W === 0 || H === 0 || value === undefined || value.length === 0){
		 continue;
	    }
	    //log.debug(obj);
            var fName = 'font'+H;
            var val = "";
            wally.loadFont(fName, config.basedir+'/etc/wallyd.d/fonts/Lato-Bol.ttf', H);
            if(value.match(/\$_./)){
               var destVal = parseString(value);
               gui.drawText(screen,X, Y, fName, color, destVal);
            } else {
               gui.drawText(screen,X, Y, fName, color, value);
            }
	    continue;
	}
        log.debug('Dont know how to handle type '+obj.type+' yet.');
    }
  
    var end = new Date().getTime()-start;
    log.debug({'time': end /1000 });
//    return end;
//    log.debug('Wallaby Screen has max size '+maxWidth+'x'+maxHeight+' Scaling by '+xScale+'x'+yScale+' Relocating by '+rX+'/'+rY);
}

exports.renderScreen = renderScreen;
