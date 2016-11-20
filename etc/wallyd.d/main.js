var settings;
var wally = new Wally();
var config = wally.getConfig();
var homedir = config.basedir+'/etc/wallyd.d';
var uv = nucleus.uv;
var gui = new GUI();

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
log.info('Seaduk modules initialized');
var modules = homedir+'/modules.duv';
var curl  = nucleus.dofile('modules/curl.js');
nucleus.dofile('modules/wally/transaction.js');

try {
	settings = JSON.parse(wally.readFile(homedir+'/settings.json'));
} catch(e) {
	log.info('No valid settings.json found in '+homedir+'/settings.json, err : '+e);
	settings = {};
}

try {
	features = JSON.parse(wally.readFile(homedir+'/features.json'));
} catch(e) {
	log.info('No valid features.json found in '+homedir+'/features.json, err : '+e);
	features = {};
}

var context = { 
    wally: wally,
    screen: wally,
    curl: curl,
    settings: settings,
    features: features,
    config: {
        debug   : config.debug,
        wally   : wally.getConfig(),
        modules : modules,
        homedir : homedir,
        fontsdir: homedir+'/fonts',
        logo    : homedir+'/images/wally1920x1080.png',
        logo2   : homedir+'/images/wallyII-1920x1080.png',
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

var myta = new Transaction();
myta.push( wally.destroyTexture.bind(null, "video2") );
myta.commit();

// This is called after the startVideo 
// or immediately if startVideo==false
context.onVideoFinished = function(){
  wally.destroyTexture('video');
  wally.setAutoRender(false);
  try {
    for (var t in textures) {
        var taName = '/texapps/'+t+'.js';
	log.error('Running texapp '+taName);
	wally.evalFile(config.homedir+taName);
    }
  } catch(err) {
	log.error('ERROR in texapps : '+err);
  }
};

try{
    context.setup = nucleus.dofile('defaults.js');
} catch(e1) {
    log.error('ERROR in defaults : '+e1);
}


try{
    if(context.config.network){
      var networktimer = new uv.Timer();
      var count = 0;
      networktimer.start(0, 100, function(){
	screen.log('Waiting for network to get ready ('+count/10+').');
	count++;
        var network = uv.interface_addresses();
        Object.keys(network).forEach(function(ifname){
          Object.keys(network[ifname]).forEach(function(addr){
            if(network[ifname][addr].internal === true)  return;
            if(network[ifname][addr].family === 'INET6') return;
	    log.info('Found a valid IPv4 address : '+network[ifname][addr].ip);
	    {
		if(context.config.network.connected === false){
		    stat = 'Network initialized. Scanning for next wallaby server';
		    wally.log(stat);
		    context.config.network.connected = true;
		    context.config.network.ip = network[ifname][addr].ip;
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

try{
	context.exec  = nucleus.dofile('execserver.js');
} catch(e3) {
	log.error('ERROR in execserver : '+e3);
}
