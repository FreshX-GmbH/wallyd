(function(global){
"use strict";

// for direct test in nucleus
if(typeof(Wally) === 'undefined')
{
        var context = nucleus.dofile('modules/wally/compat.js');
        ssdp(context);
        nucleus.uv.run();
}

function ssdp(context,location)
{
    nucleus.dofile('modules/bootstrap.js');
    var discovery =true;
    var createClient = require('modules/net').createClient;
    var httpCodec = require('modules/http-codec');
    var decoder = httpCodec.decoder;
    var p = context.p;
    var server;
    var searchST = 'freshx:wally';
    var broadcastPort = 1900;
    var broadcastAddress = '239.255.255.250';
    var broadcastString = 'M-SEARCH * HTTP/1.1\r\nHOST:'+broadcastAddress+':'+broadcastPort+'\r\nMAN:\'ssdp:discover\'\r\nST:ssdp:all\r\nNT:'+searchST+'\r\nMX:1\r\n\r\n';
    var count = 0;
    var screen = context.screen;

    log.debug("ssdp location override : ",location)
    if(location){
       registerClient(location);
       return;
    }
    start(context);
    var ssdpTimer = new uv.Timer();
    ssdpTimer.start(1000, 2000, function () {
          if (discovery !== true) {
               ssdpTimer.stop();
               ssdpTimer.close();
               try {
                   registerClient(location);
               } catch(e) {
                   log.error('Registration failed : ',e);
               }
               return;
          }
          log.info('Sending another ssdp package');
          context.screen.log('Scanning for next wallaby server ... ('+count+'s)');
          count++;

          try{
            server.send(broadcastAddress, broadcastPort, broadcastString , function (err) {
                if(err) {return;}
                log.info('Sent another ssdp package');
            });
          } catch(e){
                log.info('UDP send failed : ',e);
          }
    });

    function start(context) {
        var uv = context.uv;
        var port = 8080;
        
        server = new uv.Udp();
    
        server.bind('0.0.0.0',port);
        server.broadcast(true);
        log.info('SSDP Client bound to port '+port)
        
        server.send(broadcastAddress, broadcastPort, broadcastString , function (err) {
          if (err) {return err;}
          context.screen.log('Scanning for next wallaby server ...');
          log.info('Initial SSDP Packet sent. Waiting for response');
          server.readStart(function (err, chunk) {
            if (err) {throw err;}
            var header = {};
            var temp = chunk.toString().split(/\r\n/);
            for(var i in temp){
        	var kv = temp[i].split(/:/);
        	if(kv.length>1){
        	    header[kv[0]] = temp[i].replace(/^.*?:/,'').trim();
        	}
            }
            if(header.ST === searchST && header.LOCATION){
        	  location = header.LOCATION;
        	  log.debug('Found location : '+location+', stopping SSDP discovery');
        	  discovery = false;
        	  server.readStop();
            }
          });
        });
    }
        
    function registerClient(location){
      var featuresB64 = Duktape.enc('base64',JSON.stringify(features));
      var port,url;
      var temp = location.replace(/^http:\/\/|^https:\/\//,'').replace('/.*','');
      var port = parseInt(temp.split(/:/) ? temp.split(/:/)[1].replace(/\/.*|\&.*|\?.*/,'') : 80);
      var host = temp.replace(/:.*|\/.*|\&.*|\?.*/,'');
      var uuid = config.wally.uuid;
      var wifi = config.env.W_WFI ? nucleus.uv.getenv('W_WFI') : 'false';
      var mac= uuid.replace(/(.{2})/g,"$1:").replace(/:$/,"");
      var url = temp.replace(/^.*?\//,'/') + '?' +
      'uuid=' + uuid +
      '&arch='+config.wally.arch  +
      '&platform=WallyTV2-' + config.wally.arch +
      '&fw_version=' + config.wally.release + 
      '&mac=' + mac +
      '&width=' + config.wally.width +
      '&height=' + config.wally.height + 
      '&ip=' + config.network.ip +
      '&wifi=' + wifi + 
      '&features=' + featuresB64;
      log.debug(url);

      // wifi
      log.debug('Connecting to host : '+host+':'+port);
      screen.log('Found wallaby server at : '+host+'. Starting registration process...');
   
      try {
        var client = createClient({
          host: host,
          port: port,
          encode: httpCodec.encoder(),
          decode: httpCodec.decoder()},
          function(server){
            p('connected :', server.socket.getpeername(), server.socket.getsockname());
            server.write({
              code: 200,
              method: 'GET',
              path: url,
              headers: [
                'User-Agent', 'seaduk',
                'Date', new Date().toUTCString(),
                'Connection', 'closed',
                'Content-Type', 'text/plain',
                'Content-Length', '14'
              ]
            },function(err){
              if(err) { throw err;}
              server.read(function (err, data) {
                  if (err) { throw err; }
                  p('Header : ',data); 
              });
            });
            server.write('{ping:true;}\r\n',function(err){
              if(err) { throw err;}
              server.read(function (err, data) {
                  if (err) { throw err; }
                  try{
                    var response = JSON.parse(data);
                  } catch(err){
                    screen.log('Registration failed : '+err);
                  }
                  var demo = 10;
                  response.host = host;
        	      response.url = location;
        	      response.serverport = port;
                  response.configserver = host+":"+port;
        	      response.uuid = uuid;
                  config.connection = response;
                  if(response.configured === false){
                      screen.log('Registered at '+host+'. This client is not yet configured at this wallaby server. Starting demo mode in 10s.');
                      var timer = new uv.Timer();
                      timer.start(0, 1000, function () {
                          demo = demo - 1;
                          if(demo < 0){
                            screen.log('Running icinga2 dashboard demo. Please configure this system on your server at '+host+'.');
                            try {
                              var taName = '/texapps/demo.js';
                              log.error('Running texapp '+taName);
                              wally.evalFile(config.homedir+taName);
                              timer.stop();
                              timer.close();
                            } catch(e) {
                              log.error(e);
                            }
                          } else {
                            screen.log('Registered at '+host+'. This client is not yet configured at this wallaby server. Starting demo mode in '+demo+'s');
                          }
                      });
                  } else {
                      screen.log('Registered at '+host+'. Starting playlist');
                  }
               });
            });
      });
      } catch (e) {
            log.error('Registration process failed :',e);
      }
    }
}
    
return { ssdp: ssdp };
    
})(this);
