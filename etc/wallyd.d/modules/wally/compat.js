(function(global){
  if(typeof(Wally) !== 'function'){
    nucleus.dofile('modules/bootstrap.js');
    var utils = nucleus.dofile('modules/utils.js');
    var log = nucleus.dofile('modules/log.js');
    print('Running in nucleus compat mode.');
    var screen = {
       log: log.screen
    };
    var wally = {
       log: log.screen
    };
    var config= {
      wally : {
        arch:'nucleus',
        release:123
      },
      network : {
        lo0: [
          { internal: true, ip: "127.0.0.1", family: "INET" },
          { internal: true, ip: "::1", family: "INET6" },
        ],
        en0: [
          { internal: false, ip: "10.111.10.111", family: "INET" },
          { internal: false, ip: "2a01:1e8:e100:8428:1414:846:2e0a:3acf", family: "INET6" },
          { internal: false, ip: "2a01:1e8:e100:8428:7dc3:5280:ff51:462", family: "INET6" }
        ]
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
    global.compatMode = true;
    global.log = log;
    global.uv = nucleus.uv;
    global.utils = utils;
    global.p = utils.p;
    global.config = config;
    wally.getMac = function(iface){ return '00:00:00:00:08:15';};
    global.wally = wally;
    global.Wally = wally;
    global.config.env = nucleus.envkeys();
    global.gui = {
        drawText: log.screen,
        drawLine: log.screen
    }


    return context;
  }
})(this);
