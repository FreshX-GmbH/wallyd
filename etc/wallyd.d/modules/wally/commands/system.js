(function(){
'use strict';

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var config = function(){
    log.info('config()');
};
var persist = function(){
    log.info('persist()');
};
var requestlogs = function(){
    log.info('requestlogs()');
};
var reboot = function(){
    log.info('reboot()');
};
var firmwareUpdate = function(){
    log.info('firmwareUpdate()');
};

return {
    init: init,
    config: config,
    persist: persist,
    reboot: reboot,
    requestlogs: requestlogs,
    firmwareUpdate: firmwareUpdate
};

})();
