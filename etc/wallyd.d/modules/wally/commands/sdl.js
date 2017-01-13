(function(){
"use strict";

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var showImage = function(command){
    log.info('image() : '+command);
};

var video = function(command){
    log.info('video() : '+command);
};

var text = function(command){
    log.info('text() : '+command);
    screen.setText('main','black','chalk32',10,10,command.url);
    screen.render('main');

};
return {
    showImage: showImage,
    video: video,
    text: text,
    init: init
};
})();
