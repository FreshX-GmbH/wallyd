(function(){
"use strict";

var ctx;
var curl;
var wallabyRenderer;
var decodeUrl = require('modules/net').decodeUrl;

var init = function(_ctx){
    ctx = _ctx;
    curl = nucleus.dofile('modules/curl.js');
    wallabyRenderer = require('modules/wally/wallabyRenderer');
};

var wallaby = function(command){
    log.info('WallabyRenderer : '+typeof(wallabyRenderer))
    log.info('Running wallaby() : '+command);
    log.info('Requesting URL : ',command.url)
    wally.log('Loading Wallaby screen from '+command.url);
    var url=decodeUrl(command.url);
    var server = url.proto+url.host+':'+url.port;
    try {
        var res  = curl.get(command.url);
        var code = res.code;
        var headers = res.headers;
        var jsondata = res.body;
	log.info('Wallaby Screen return code : ',code);
	log.info('Wallaby Screen headers : ',headers);
//      TODO : fix request.js buffer bug
//      var json = request(command.url,function(err,header,jsondata){
//	if(err){
//	    log.error('Could not access wallaby json');
//	    return err;
//	}
	try{
	   var json = JSON.parse(jsondata);
	}catch(e){
	    log.error('Wallaby Screen data invalid : ',e);
	    print(jsondata);
	}
	log.info('Wallaby Server returned : ',Object.keys(json));
        if(json.pages){
	    wally.log('Pages in screen found : '+Object.keys(json.pages));
	    log.info('Pages found : ',Object.keys(json.pages));
	    try {
   	        var renderTA = new Transaction();
   	        renderTA.push(gui.clearTextureNoPaint.bind(null,'main'));
   	        wallabyRenderer.renderScreen(renderTA,server,context,context.privates,'main',json.pages[0]);
   	        renderTA.push(wally.render.bind(null,'main'));
   	        renderTA.commit();
   	     } catch(err) {
   	        log.error('ERROR: Show wallaby screen failed : '+err);
   	        wally.log('ERROR: Show wallaby screen failed : '+err);
	     }
   	 }
    } catch(e2){
	log.error('Wallaby Curl failed.',e2);
    }
};

return {
    wallaby: wallaby,
    init: init
};
})();
