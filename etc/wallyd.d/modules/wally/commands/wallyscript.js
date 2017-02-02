(function(){
"use strict";

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var wallyscript = function(command,callback){
    log.info('wallyscript() : '+command.url);
    eval(command.url);
    callback(null);
};

return {
    wallyscript: wallyscript,
    init: init
};
})();
