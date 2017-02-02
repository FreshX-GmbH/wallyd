(function(){
"use strict";

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var show = function(command,callback){
    log.info('show() : '+command);
    return callback(null);
};

return {
    show: show,
    init: init
};
})();
