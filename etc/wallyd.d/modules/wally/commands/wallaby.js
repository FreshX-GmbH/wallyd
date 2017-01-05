(function(){
"use strict";

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var wallaby = function(){
    log.info('wallaby()');
};

return {
    wallaby: wallaby,
    init: init
};
})();
