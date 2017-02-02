(function(){
'use strict';

var ctx;

var init = function(_ctx){
    ctx = _ctx;
};

var config = function(data,callback){
    log.info('config()');
    return callback(null);
};
var persist = function(data,callback){
    log.info('persist()');
    return callback(null);
};
var requestlogs = function(data,callback){
    log.info('requestlogs()');
    return callback(null);
};
var reboot = function(datamcallback){
    log.info('reboot()');
    return callback(null);
};
var firmwareUpdate = function(data,callback){
    log.info('firmwareUpdate()');
    return callback(null);
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
