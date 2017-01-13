(function(){
"use strict";

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var wallyscript = function(command){
    log.info('wallyscript() : '+command.url);
    eval(command.url);
};

return {
    wallyscript: wallyscript,
    init: init
};
})();
