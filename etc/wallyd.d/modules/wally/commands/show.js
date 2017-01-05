(function(){
"use strict";

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var show = function(){
    log.info('show()');
};

return {
    show: show,
    init: init
};
})();
