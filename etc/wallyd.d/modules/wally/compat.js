(function(global){
  if(typeof(Wally) !== 'function'){
    nucleus.dofile('modules/bootstrap.js');
    var utils = nucleus.dofile('modules/utils.js');
    var log = nucleus.dofile('modules/log.js');
    var transaction = nucleus.dofile('modules/wally/transaction.js');
    print('Running in nucleus compat mode.');
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
    global.compatMode = true;
    global.log = log;
    global.uv = nucleus.uv;
    global.utils = utils;
    global.p = utils.p;
    global.config = config;
    global.config.env = nucleus.envkeys();
    global.SVG = {
    };
    global.wally = {
       log: log.screen,
       getMac:     function(iface){ return '00:00:00:00:08:15';},
   	   render:     function(){ log.screen('render : '+Array.prototype.slice.call(arguments).join(" "));},
   	   drawText:   function(){ log.screen('drawText : '+Array.prototype.slice.call(arguments).join(" "));},
   	   drawLine:   function(){ log.screen('drawLine : '+Array.prototype.slice.call(arguments).join(" "));},
   	   loadFont:   function(){ log.screen('loadFont : '+Array.prototype.slice.call(arguments).join(" "));},
   	   loadImage:  function(){ log.screen('loadImage: '+Array.prototype.slice.call(arguments).join(" "));},
       setAutoRender: log.screen,
       writeFileSync: log.screen,
       // GUI
       clearTextureNoPaint: log.screen,
       setTargetTexture: log.screen,
       drawFilledBox: log.screen
    };
    global.screen = global.wally;
    global.Wally = global.wally;
    var context = {
        compatMode: true,
        utils : utils,
        p : utils.prettyPrint,
        log : log,
        screen : global.screen,
        wally : global.wally,
        config : config,
        uv : nucleus.uv
    }
    return context;
  }
})(this);
