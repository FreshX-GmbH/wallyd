var wally = new Wally();
var config = wally.getConfig();
var homedir = config.basedir+'/etc/wallyd.d';
var uv = nucleus.uv;
var gui = new GUI();

if(nucleus){
	var modules = homedir+'/modules';
	nucleus.dofile('modules/bootstrap.js')
	var utils = nucleus.dofile('modules/utils.js')
	var p     = utils.prettyPrint;
	var log   = nucleus.dofile('modules/log.js');
	var uv    = nucleus.uv;
	var gui   = gui;
	var extra = nucleus.dofile('modules/extra.js');
	var fs    = nucleus.dofile('modules/fs.js');
	var extra = nucleus.dofile('modules/extra.js');
	var curl  = nucleus.dofile('modules/curl.js');
	log.info('Seaduk modules initialized');
	var modules = homedir+'/modules.duv';
}

var context = { 
    wally: wally,
    screen: wally,
    config: {
        debug   : config.debug,
        wally   : wally.getConfig(),
        modules : modules,
        homedir : homedir,
        fontsdir: homedir+'/fonts',
        logo    : homedir+'/images/wally1920x1080.png',
        video   : homedir+'/images/WallyStart.mp4',
        testScreen: false,
        startVideo : false,
    },
    uv:uv,
    p:p
};
context.config.env   = nucleus.envkeys();

if(typeof uv.interface_addresses === 'function'){
    context.config.network = uv.interface_addresses();
    context.config.network.connected = false;
    p(context.config.network);
    context.config.hrstart = uv.hrtime();
    context.config.cpuinfo = uv.cpu_info();
    p(context.config.cpuinfo);
} else {
    log.error('Seaduk misc extension not found : ',typeof uv.interface_addresses);
}

// This is called after the startVideo 
// or immediately if startVideo==false

context.onVideoFinished = function(){
  wally.destroyTexture('video');
  try {
  	wally.evalFile(config.homedir+'/texapps/demo.js');
  } catch(err) {
	log.error('ERROR in demo.js : '+err);
  }
//  screen.log('Initializing texApps ...');
//  for (var t in textures) {
//    log.info('Running texApp : ',t);
//    screen.log('Initializing texApp : '+t);
//    var taName = 'texapps/'+t+'.js';
//    nucleus.dofile(taName);
// //   var timer = new uv.Timer();
// //   timer.start(0, 1, function(){ print(nucleus.dofile(taName));});
//  }
  p(context);
};

try{
    context.setup = nucleus.dofile('defaults.js');
} catch(e1) {
    log.error('ERROR in defaults : '+e1);
}

var stat = 'R'+context.config.wally.release/1000+
         ' *** Res: '+context.config.wally.width+'x'+context.config.wally.height+
         ' *** Arch: '+context.config.wally.arch;
screen.setText('version','black','logfont',0,0,stat);
screen.log('Waiting for network to get ready.');
screen.render('version');

try{
    if(context.config.network){
      var networktimer = new uv.Timer();
      var count = 0;
      networktimer.start(0, 100, function(){
	screen.log('Waiting for network to get ready ('+count/10+').');
	count++;
        var network = uv.interface_addresses();
        //print(Object.keys(network));
        Object.keys(network).forEach(function(ifname){
          Object.keys(network[ifname]).forEach(function(addr){
            if(network[ifname][addr].internal === true)  return;
            if(network[ifname][addr].family === 'INET6') return;
	    log.info('Found a valid IPv4 address : '+network[ifname][addr].ip);
	    {
		if(context.config.network.connected === false){
		    stat = 'Network initialized. Scanning for next wallaby server';
		    wally.log(stat);
		    wally.setText('ip','black','logfont',0,1,"IP:"+network[ifname][addr].ip);
		    wally.render('ip');
		    context.config.network.connected = true;
		    log.info(stat);
		    try {
		      context.ssdp  = nucleus.dofile('ssdp.js').ssdp(context);
		    } catch(e) {
		      log.error('SSDP failed : ',e);
		    }
		    networktimer.stop();
		    networktimer.close();
		}
	    }
          });
        });
      });
    } else {
      context.ssdp  = nucleus.dofile('ssdp.js').ssdp(context);
    }
} catch(e2) {
	log.error('ERROR in ssdp : '+e2);
}

//try{
//	context.exec  = nucleus.dofile('execserver.js');
//} catch(e3) {
//	log.error('ERROR in execserver : '+e3);
//}
