//'use strict';

var uv = nucleus.uv;
var playlistWaitTimer = new uv.Timer();
var wally;
var gui;
var modules = {};

var request = function(url, callback){
    callback('failed',null,null);
};

var exec = function(cmd, callback){
   if(typeof(modules[cmd.command]) !== 'undefined'){
      modules[cmd.command](cmd.id);
   } else {
      log.error('No module for command '+cmd.command+' available');
   }
};

var playlist = function(){
    var commands = config.connection.commands;
    if(typeof(commands) !== 'undefined' && commands.length > 0){
        for (var id in commands){
            exec(commands[id],function(err,data){
                  log.info(err);
            });
        }
    } else {
      log.info('No commands, requesting for new ones', typeof(commands));
      // request new commands
    }
};

//	Initialize and run timer
var playlistWait = function() {
    if(typeof(config.connection) === 'undefined'){
        log.error('Could not find config property');
        return;
    }
    if(config.connection.configured === true){
        log.info('System configured, running playlist with currently '+config.connection.commands.length+' commands');
        playlist();
        playlistWaitTimer.stop();
        playlistWaitTimer.close();
    }
};

var restartLoop = function(){
    log.info('Playlist waiting for connection');
    try {
        playlistWaitTimer.start( 0, 6000, playlistWait);
    } catch(e) {
        log.error('Error in playlistWait timer : '+e);
    }
};

var sendSuccess = function(context){
    log.debug('Command executed successfully. Notifying server.');
    var successURL=config.connection.configserver+'/commandsuccess?uuid='+context.config.uuid+'&cmdid='+context.cmdid;
    log.debug('Calling : '+successURL);
    request(successURL, function(error, response, body) {
        if(error){
            log.debug('Could not notify : '+error);
        }
        // setTimeout(function() {
        //     exports.loop(context.opts);
        // }, 0);
    });
};

var sendFailed = function(context, err){
    log.debug('Command executed with error. Notifying server.');
    // TODO : base64 encode error string
    err = err + '';
    var successURL=context.config.configserver+'/commandfailed?uuid='+context.config.uuid+'&cmdid='+context.cmdid+'&err='+err.toString('base64');
    // console.log('Calling : '+successURL);
    request(successURL, function(error, response, body) {
        if(error){
            log.debug('Could not notify : '+error);
        }
        // setTimeout(function() {
        //     exports.loop(context.opts);
        // }, 0);
    });
};

var sendCommand = function(command){
   log.debug('Request command', config.connection.configserver+'/command?uuid='+config.uuid);
   request(config.connection.configserver+'/command?uuid='+config.uuid, function(error, response, body) {
       if(!body){
           log.error('Request failed. Suspending 10 secs...');
           return restartLoop(opts);
       }

       var command = null;

       try {
           command = JSON.parse(body);
       } catch(ex) {

       }
       if(!command || !command.id){
           // Bail out and restart
           log.error('Invalid command structure / returned JSON not valid. Suspending 10 secs...');
           return restartLoop(opts);
       }
       //return exports.exec(opts, command);
   });
};

if(typeof(Wally) === 'undefined')
{
        context = nucleus.dofile('modules/wally/compat.js');
        gui = wally = context.wally;
        config.connection = {
            name: 'Nucleus Client', configured: true,
            mac: '00:27:EB:10:96:00', type: 'WallyTV2-x86_64', arch: 'x86_64',
            firmwareVersion: '1794', lastdetection: '2017-01-05T14:21:02.247Z',
            host: '127.0.0.1', url: 'http://127.0.0.1:8083/register', serverport: 8083,
            configserver: '127.0.0.1:8083', uuid: '0027EB109600',
            lan: { dhcp: true, ip: 'dhcp', subnet: 'auto', gw: 'auto', dns: 'auto', autoIP: '10.111.10.111' },
            commands: [ { command: 'config', id: 'e01a2485-00d3-435a-be63-f4c876869dfa' },
                        { command: 'show', id: '0e5bf569-872e-4d79-a316-8bb954eb1ec6' } ],
            features: {
               modules: [ 'core' ], binaries: [ 'wallyd', 'node' ],
               plugins: [ 'CO2Demo', 'WallyBill', 'Photobooth' ],
               npm: [ 'async', 'noble', 'npm', 'uuid' ]
            }
        };
} else  {
        wally = new Wally();
        gui = new GUI();
}

var loadModules = function(){
   nucleus.scandir('modules/wally/commands', function(file){
      var name = file.split(/\./)[0];
      log.info('Loading wallyTV module '+name);
      var commands = nucleus.dofile('modules/wally/commands/'+file);
      var info = '';
      commands.init(context);
      for(var c in commands){
         if(c === 'init') { 
            continue;
         }
         if(modules[c]){
            log.error('Command '+c+' already defined. Ignoring the command from '+name);
            continue;
         }
         modules[c] = commands[c];
         info = info + ' ' + c;
      }
      log.info('Found the following commands in '+name+' :'+info);
   });
};

loadModules();
restartLoop();

if(typeof(Wally) === 'undefined'){
    nucleus.uv.run();
}
