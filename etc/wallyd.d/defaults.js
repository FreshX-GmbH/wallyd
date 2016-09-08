context.tex={};
var wally = screen = context.screen = context.wally;
var config  = context.config;
var homedir = context.config.homedir;
var fontsdir= context.config.fontsdir;

var textures = {
    main      : { z: 10, x: 0,   y:    0,  w: '50%', h:-16, color : 'FFFFFF' },
    main2     : { z: 10, x: '50%', y:  0,  w: '50%', h:-16, color : 'FFFFFF' },
    log       : { z: 30, x: 0,   y: '-20', w: '100%', h: 20, color : 'FEFEFE' }, 
    version   : { z: 40, x: -42, y: 0,     w: 42,     h: 16, color : 'FFFFFF' },
    network   : { z: 50, x: -20, y: -20,   w: 20,     h: 20, color : 'FFA500' }
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

screen.startTransaction();

for (var c in colors){
    screen.createColor(c,colors[c],'FF');
}
for (var f in fonts) {
    screen.loadFont(f,fonts[f].file, fonts[f].size);
}
for (var t in textures) {
    screen.createTexture(t,textures[t].z,textures[t].x,textures[t].y,textures[t].w,textures[t].h,textures[t].color);
}


//   Display the test screen
if(config.testScreen === true){
    screen.showTextureTestScreen();
} else {
    screen.log('WallyTV starting...');
    screen.setImageScaled('main',config.logo);
    screen.setText('version','black','logfont',0,0,'R'+config.wally.release/1000);
}

screen.commitTransaction();

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
