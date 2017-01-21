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
    log.info('Running wallaby() : '+command.url);
    wally.log('Loading Wallaby screen from '+command.url);
    var url=decodeUrl(command.url);
    var server = url.proto+url.host+':'+url.port;
    try {
//        var res  = curl.get(command.url);
//        var code = res.code;
//        var header = res.headers;
//        var jsondata = res.body;
//      TODO : fix request.js buffer bug
      request(command.url,function(err,header,jsondata){
	if(err){
	    log.error('Could not access wallaby json');
	    return err;
	}
	var code = header.code;
	log.info('Wallaby Screen return code : ',code);
	log.info('Wallaby Screen headers : ',header);
	try{
	   var json = JSON.parse(jsondata);
	}catch(e){
	    log.error('Wallaby Screen data invalid : ',e);
	//    print(jsondata);
	    return err;
	}
	//log.info('Wallaby Server returned : ',Object.keys(json));
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
	        return err;
	     }
   	 }
      });
    } catch(e2){
	log.error('Wallaby Request/Curl failed.',e2);
	return err;
    }
};

return {
    wallaby: wallaby,
    init: init
};
})();
