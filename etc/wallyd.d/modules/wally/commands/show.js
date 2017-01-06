(function(){
"use strict";

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var show = function(command){
    log.info('show() : '+command);
};

return {
    show: show,
    init: init
};
})();
