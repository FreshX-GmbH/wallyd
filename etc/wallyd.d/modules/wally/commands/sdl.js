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

var showImage = function(command){
    log.info('Requesting Image URL : ',command.url)
    var url=decodeUrl(command.url);
    var server = url.proto+url.host+':'+url.port;
    try {
        var TA = new Transaction();
        var res  = curl.get(command.url);
        var code = res.code;
        var headers = res.headers;
        var image = res.body;
        screen.log('Loading image from '+command.url+' updating every '+command.wait+' seconds');
        if(res.body){
           wally.writeFileSync('/tmp/test.png',res.body);
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
           log.error('Could not download from '+server+': '+imgUrl);
        }
    } catch(e2){
	log.error('Image Curl failed.',e2);
        return e2;
    }
};

return {
    showImage: showImage,
    video: video,
    text: text,
    init: init
};

})();
