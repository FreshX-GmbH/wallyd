
var uv = nucleus.uv;
var playlistWaitTimer = new uv.Timer();
var wally;
var gui;
var modules = {};
var request;

var exec = function(cmd){
   if(typeof(modules[cmd.command]) !== 'undefined'){
      var ret = modules[cmd.command](cmd);
      if(ret === undefined){
         sendSuccess(cmd.id);
      } else {
         log.error('Command failed : ',ret);
         sendFailed(cmd.id,ret);
      }
   } else {
      log.error('No module for command '+cmd.command+' available');
   }
};

var playlist = function(){
    var commands = config.connection.commands;
    if(typeof(commands) !== 'undefined' && commands.length > 0){
        for (var id in commands){
           if(commands[id].active === true){
               log.debug(commands[id]);
               exec(commands[id]);
           }
        }
        config.connection.commands=[];
    }
    log.info('No more commands, requesting configserver for new ones');
    request('http://'+config.connection.configserver+'/command?uuid='+config.connection.uuid, function(err,header,body){
         if(err) {
            log.error('Could not connect to config server : '+err);
            return restartLoop();
         }
         log.info(header);
         try {
            var response = JSON.parse(body);
            config.connection.commands.push(response);
         } catch(e) {
            log.error('Could not decode server response : ',e,body);
            return restartLoop();
         }
         return playlist();
    });
};

//	Initialize and run timer
var playlistWait = function() {
    if(typeof(config.connection) === 'undefined'){
        log.warn('Playlist : Could not find a valid connection config property');
        return;
    }
    if(!config.connection.commands) {
        config.connection.commands = [];
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

var sendSuccess = function(id){
    log.debug('Command executed successfully. Notifying server.');
    var successURL=config.connection.configserver+'/commandsuccess?uuid='+config.connection.uuid+'&cmdid='+id;
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

var sendFailed = function(id, err){
    log.debug('Command executed with error. Notifying server.');
    err = err + '';
    var successURL=config.connection.configserver+'/commandfailed?uuid='+config.connection.uuid+'&cmdid='+id+'&err='+Duktape.enc('base64',err);
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
        nucleus.dofile('modules/bootstrap.js');
        context = nucleus.dofile('modules/wally/compat.js');
        gui = wally = context.wally;
        config.connection = {
            name: 'Nucleus Client', configured: true,
            mac: '00:27:EB:10:96:00', type: 'WallyTV2-x86_64', arch: 'x86_64',
            firmwareVersion: '1794', lastdetection: '2017-01-05T14:21:02.247Z',
            host: '127.0.0.1', url: 'http://127.0.0.1:8083/register', serverport: 8083,
            configserver: '127.0.0.1:8083', uuid: '0027EB109600',
            lan: { dhcp: true, ip: 'dhcp', subnet: 'auto', gw: 'auto', dns: 'auto', autoIP: '10.111.10.111' },
            //commands: [ { command: 'config', id: 'e01a2485-00d3-435a-be63-f4c876869dfa' },
            //            { command: 'show', id: '0e5bf569-872e-4d79-a316-8bb954eb1ec6' } ],
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

request = nucleus.dofile('modules/request.js').request;

loadModules();
restartLoop();

if(typeof(Wally) === 'undefined'){
    nucleus.uv.run();
}
