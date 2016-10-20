context.tex={};
var wally = screen = context.screen = context.wally;
var config  = context.config;
var homedir = context.config.homedir;
var fontsdir= context.config.fontsdir;
var defTA = new Transaction();

var textures = {
    main      : { z: 10, x: 0,   y:    0,  w: '100%', h:-16, color : 'FFFFFF' },
//    main2     : { z: 10, x: '50%', y:  0,  w: '50%', h:-16, color : 'FFFFFF' },
    log       : { z: 30, x: 0,   y: '-20', w: '100%', h: 20, color : 'FEFEFE' }, 
    version   : { z: 40, x: -260, y: 0,     w: 260,     h: 16, color : 'FFFFFF' },
    memdbg    : { z: 41, x: -260, y: 16,     w: 260,     h: 16, color : 'FFFFFF' },
    netinfo   : { z: 42, x: -260, y: 32,     w: 260,     h: 16, color : 'FFFFFF' },
    stat      : { z: 43, x: -260, y: 48,     w: 260,     h: 16, color : 'FFFFFF' },
    up        : { z: 50, x: -120, y: -20,   w: 100,     h: 20, color : 'FEFEFE' },
    netcolor  : { z: 50, x: -20, y: -20,   w: 20,     h: 20, color : '2BFF00' }
};

if(config.startVideo === true && textures.video === undefined){
    textures.video = { z: 51, x: 0, y: 0, w: '100%',    h: -20,color : 'FFA500' };
}

var colors = {
    black     : '000000',
    log       : '000000',
    white     : 'FFFFFF',
    stampColor: 'B00000'
};

fonts = {
    logfont  : { file : fontsdir+'/Lato-Bol.ttf', size : 12},
    stampfont: { file : fontsdir+'/umbrage2.ttf',  size : 48},
    font16   : { file : fontsdir+'/Lato-Bol.ttf', size : 16},
    hugefont : { file : fontsdir+'/Lato-Bol.ttf', size : 96}
};

for (var c in colors){
    defTA.push( screen.createColor.bind(null, c,colors[c],'FF'));
}
for (var f in fonts) {
    defTA.push( screen.loadFont.bind(null, f,fonts[f].file, fonts[f].size));
}
for (var t in textures) {
    defTA.push( screen.createTexture.bind(null, t,textures[t].z,textures[t].x,textures[t].y,textures[t].w,textures[t].h,textures[t].color));
}


//   Display the test screen
if(config.testScreen === true){
    defTA.push(screen.showTextureTestScreen);
} else {
    defTA.push(screen.log.bind(null,'WallyTV starting...'));
    defTA.push(screen.setImageScaled.bind(null,'main',config.logo));
//    screen.setText('version','black','logfont',0,0,'R'+config.wally.release/1000);
}

defTA.commit();

if(config.startVideo === true){
    p(context.onVideoFinished);
    try{
        context.startVideo = new FFVideo();
        context.startVideo.play('video', config.video,context.onVideoFinished);
    } catch(err) {
        print('Error in video play :',err);
    }
} else {
    context.onVideoFinished();
}
