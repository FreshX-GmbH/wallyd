(function(){
"use strict";

var ctx;
var curl;
var decodeUrl = require('modules/net').decodeUrl;

var init = function(_ctx){
    ctx = _ctx;
    curl = nucleus.dofile('modules/curl.js');
};

var video = function(command){
    log.info('video() : '+command);
};

var text = function(command){
    log.info('text() : '+command);
    screen.setText('main','black','chalk32',10,10,command.url);
    screen.render('main');

};

var gradient = function(command){
    log.info('gradient() : '+command.url);
    var params = command.url.split(/ /);
    gui.clearTextureNoPaint(params[0]);
    gui.setTargetTexture(params[0]);
    //params[5] = params[5].
    wally.scall('gui::drawGradient '+command.url);
    screen.render(params[0]);
};

var showImage = function(command){
    log.info('Requesting Image URL : ',command.url)
    var url=decodeUrl(command.url);
    var server = url.proto+url.host+':'+url.port;
    if(command.wait > command.delay){
        screen.log('Loading image once from '+command.url);
    } else {
        screen.log('Loading image from '+command.url+' updating every '+command.wait+' seconds');
    }
    try {
      request(command.url,function(err,header,image){
        if(err) {
            return err;
        }
        var TA = new Transaction();
        //var res  = curl.get(command.url);
        //var code = res.code;
        //var headers = res.headers;
        //var image = res.body;
        var headers = header;
        if(image){
           wally.writeFileSync('/tmp/test.png',image);
           log.info('Image saved to /tmp/test.png');
           TA.push(gui.clearTextureNoPaint.bind(null,'main'));
           //TA.push(gui.setImage.bind(null,'main',0,0,'/tmp/test.png'));
           TA.push(gui.loadImage.bind(null,'main','/tmp/test.png',0 , 0, 0, 0, 255));
           TA.push(screen.render.bind(null,'main'));
           TA.commit();
           //gui.setTargetTexture('main');
           //gui.clearTextureNoPaint('main');
           //gui.loadImage('main','/tmp/test.png',0 , 0, 1000,600, 127);
           //screen.render('main');
           //gui.resetTargetTexture();
        } else {
           log.error('Could not download from '+server+': '+command.url,'. Error :',err);
        }
      });
    } catch(e2){
	log.error('Image Request/Curl failed.',e2);
        return e2;
    }
};

return {
    showImage: showImage,
    video: video,
    text: text,
    gradient: gradient,
    init: init
};

})();
