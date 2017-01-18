(function(){
"use strict";

var ctx;
var curl;
var decodeUrl = require('modules/net').decodeUrl;

var init = function(_ctx){
    ctx = _ctx;
    curl = nucleus.dofile('modules/curl.js');
};

var image = function(command){
    log.info('Requesting Image URL : ',command.url)
    var url=decodeUrl(command.url);
    var server = url.proto+url.host+':'+url.port;
    try {
        var res  = curl.get(command.url);
        var code = res.code;
        var headers = res.headers;
        var image = res.body;
        if(res.body){
           wally.writeFileSync('/tmp/test.png',res.body);
           TA.push(gui.loadImage.bind(null,screen,'/tmp/test.png',X, Y, W/xScale, H/yScale, 255));
        } else {
           log.error('Could not download from '+server+': '+imgUrl);
        }
    } catch(e2){
	log.error('Image Curl failed.',e2);
    }
};

return {
    image: image,
    init: init
};
})();
