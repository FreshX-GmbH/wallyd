//  Some core variables
var _config = context.wally.getConfig();
context.tex={};
context.screen = context.wally;

screen = context.screen;

context.config = { 
    debug   : _config.debug,
    wally   : _config,
    logo    : _config.basedir+'/etc/wallyd.d/images/wally1920x1080.png',
    modules : _config.basedir+'/etc/wallyd.d/modules',
    testScreen: false,
    video: _config.basedir+'/etc/wallyd.d/images/WallyStart.mp4',
    startVideo : true,
};

colors = {
    black     : '000000',
    log       : '000000',
    white     : 'FFFFFF',
    stampColor: 'B00000'
};

fonts = {
    logfont  : { file : context.config.wally.basedir+'/etc/wallyd.d/fonts/Lato-Bol.ttf', size : 12},
    stampfont: { file : context.config.wally.basedir+'/etc/wallyd.d/fonts/umbrage2.ttf',       size : 48},
    font16   : { file : context.config.wally.basedir+'/etc/wallyd.d/fonts/Lato-Bol.ttf', size : 16},
    hugefont : { file : context.config.wally.basedir+'/etc/wallyd.d/fonts/Lato-Bol.ttf', size : 96}
};

for (var c in context.colors){
    screen.createColor(c,context.colors[c],'FF');
}

for (var f in context.fonts) {
    screen.loadFont(f,context.fonts[f].file, context.fonts[f].size);
}

context.dump = function dump(obj)
{
  var cache=[];
  print(JSON.stringify(obj,function(key, value) {
    if (typeof value === 'object' && value !== null) {
        if (cache.indexOf(value) !== -1) {
            // Circular reference found, discard key
            //cache.push("["+value+"]");
            return;
        }
        // Store value in our collection
        cache.push(value);
    }
    return value;
  },2));
}
