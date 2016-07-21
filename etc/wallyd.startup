var wally = new Wally();
var config = wally.getConfig();
var homedir = config.basedir+'/etc/wallyd.d';
var modules = homedir+'/modules';

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

print('Preparing wallyd basic setup');
wally.evalFile(homedir+'/wallyd.setup');

// This is working in the dukluv version only (dwallyd)
//
if( this.main === true ){
    var log = require(modules+'/log.js');
    var utils = require(modules+'/utils.js');
    var fs = require(modules+'/fs.js');
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
    utils.dumpJSON(context);
}

