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
    }
};

config = context.config;

log.info('Preparing wallyd basic setup');

//p(this);

if(nucleus){
    log.info('Executing all js files');
    files = nucleus.scandir('.',function(f,type){
        if(type === 'file' && f.match('\.js$') && !f.match('^main.js$')){
      //     try {
      //       log.debug('Running ',f);
             this.f = nucleus.dofile(f);
      //     } catch(err) {
      //       log.error('Failed to run ',f,' : ',err);
      //     }
	}
    });
} else {
    wally.evalFile(modules+'/wally.setup.js');
    files = fs.readdir(homedir,function(err,files){
        if(err){
            log.error('Error reading files from ',homedir);
            return;
        }
        utils.dumpJSON(files);
        for(var i in files){
            var f = files[i];
            if(!f.type || (f.type === 'file' && !f.name.match('\.js$'))){
              continue;
            }
            try {
              log.info('Running ',f.name);
              var x = require(homedir+'/'+f.name);
            } catch(err) {
              log.error('Failed to run ',f.name,' : ',err);
            }
          }
    });
    p(context);
}	