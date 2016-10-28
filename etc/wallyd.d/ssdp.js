(function(global){
"use strict";

// for direct test in nucleus
if(typeof(Wally) === 'undefined')
{
        var context = nucleus.dofile('modules/compat.js');
        ssdp(context);
        nucleus.uv.run();
}

function ssdp(context)
{
    nucleus.dofile('modules/bootstrap.js');
    var discovery =true;
    var location = '';
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
        log.info('SSDP Server bound to port '+port)
        
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
      var port,url;
      var temp = location.replace(/^http:\/\/|^https:\/\//,'').replace('/.*','');
      var port = parseInt(temp.split(/:/) ? temp.split(/:/)[1].replace(/\/.*|\&.*|\?.*/,'') : 80);
      var host = temp.replace(/:.*|\/.*|\&.*|\?.*/,'');
      var uuid = config.wally.uuid;
      var mac= uuid.replace(/(.{2})/g,"$1:").replace(/:$/,"");
      var url = temp.replace(/^.*?\//,'/') + '?' +
      'uuid=' + uuid +
      '&arch='+config.wally.arch  +
      '&platform=OSE-SD-' + config.wally.arch +
      '&fw_version=' + config.wally.release + 
      '&mac=' + mac;
      // wifi
      // ip
      log.debug('Connecting to host : '+host+':'+port);
      screen.log('Found wallaby server at : '+host+'. Starting registration process...');
    
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
                p('Payload : ',response);
                var demo = 10;
                config.conn = response;
    	    response.host = host;
      	    response.url = location;
      	    response.serverport = port;
      	    response.uuid = uuid;
                p('Config : ',config);
                if(response.configured === false){
                    screen.log('Registered at '+host+'. This client is not yet configured at this wallaby server. Starting demo mode in 60s.');
                    var timer = new uv.Timer();
                    timer.start(0, 1000, function () {
                        demo = demo - 1;
                        if(demo < 0){
                          timer.stop();
                          timer.close();
                        } else {
                          screen.log('Registered at '+host+'. This client is not yet configured at this wallaby server. Starting demo mode in '+demo+'s');
                        }
                    });
                } else {
                    screen.log('Registered at '+host+'. Starting playlist');
                }
                p(context);
                });
          });
      });
    }
}
    
return { ssdp: ssdp };
    
})(this);
