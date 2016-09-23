(function(global){
  if(typeof(Wally) !== 'function'){
    var utils = nucleus.dofile('modules/utils.js');
    var log = nucleus.dofile('modules/log.js');
    print('Running in nucleus compat mode.');
    var screen = {
       log: print
    };
    var config= {
      wally : {
        arch:'nucleus',
        release:123
      }
    };
    var context = {
        utils : utils,
        p : utils.prettyPrint,
        log : log,
        screen : screen,
        config : config,
        uv : nucleus.uv
    }
    global.log = log;
    global.uv = nucleus.uv;
    global.utils = utils;
    global.p = utils.p;
    global.config = config;
    return context;
  }
})(this);
