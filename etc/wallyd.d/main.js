var wally = new Wally();
var config = wally.getConfig();
var homedir = config.basedir+'/etc/wallyd.d';

if(nucleus){
	var modules = homedir+'/modules';
	nucleus.dofile('modules/bootstrap.js')
	var utils = nucleus.dofile('modules/utils.js')
	var p     = utils.prettyPrint;
	var log   = nucleus.dofile('modules/log.js');
	var uv    = nucleus.uv;
	var extra = nucleus.dofile('modules/extra.js');
	var fs    = nucleus.dofile('modules/fs.js');
	log.info('Seaduk modules initialized');
        p(nucleus.envkeys());
} else {
	var modules = homedir+'/modules.duv';
	var log = {info:print, error:print, debug:print};
}

context = { 
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
    p:p
};

// This is called after the startVideo 
// or immediately if startVideo==false

context.onVideoFinished = function(){
  wally.destroyTexture('video');
  wally.evalFile(config.homedir+'/texapps/demo.js');
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

context.setup = nucleus.dofile('defaults.js');
//context.ssdp  = nucleus.dofile('ssdp.js');
//context.exec  = nucleus.dofile('execserver.js');
p(context);
