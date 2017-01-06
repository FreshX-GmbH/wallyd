(function(){
"use strict";

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var wallaby = function(command){
    log.info('wallaby() : '+command);
};

return {
    wallaby: wallaby,
    init: init
};
})();
