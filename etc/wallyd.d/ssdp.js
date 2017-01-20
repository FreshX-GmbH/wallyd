(function(global){
"use strict";
var context, decodeUrl;

function ssdp(context,iface,location)
{
    nucleus.dofile('modules/bootstrap.js');
    var discovery =true;
    var createClient = require('modules/net').createClient;
    decodeUrl = require('modules/net').decodeUrl;
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
       registerClient(iface,location);
       return;
    }
    start(context);
    var ssdpTimer = new uv.Timer();
    ssdpTimer.start(1000, 2000, function () {
          if (discovery !== true) {
               ssdpTimer.stop();
               ssdpTimer.close();
               try {
                   registerClient(iface,location);
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
        
    function registerClient(iface,location){
      var urlParts = decodeUrl(location);
      var featuresB64 = Duktape.enc('base64',JSON.stringify(features));
      var mac = wally.getMac(iface);
      var uuid = mac.replace(/:/g,'');
      var wifi = config.env.W_WFI ? nucleus.uv.getenv('W_WFI') : 'false';
      var url = location + '?uuid=' + uuid +
      '&arch='+config.wally.arch  +
      '&platform=WallyTV2-' + config.wally.arch +'-'+ config.wally.OS +
      '&fw_version=' + config.wally.release + 
      '&mac=' + mac +
      '&width=' + config.wally.width +
      '&height=' + config.wally.height + 
      '&ip=' + config.network.ip +
      '&wifi=' + wifi + 
      '&features=' + featuresB64;

      log.debug('Registration at ',url);
      screen.log('Found wallaby server at : '+location+'. Starting registration process...');
      try {
        request(url, function(err, header, data){
          print(header);
          print(data);
          if(err){
              screen.log('Registration failed : '+err);
              log.error('Register call failed : '+err);
              return;
          }
          try{
              var response = JSON.parse(data);
          } catch(err){
              screen.log('Registration response invalid.');
              log.error('Registration response invalid : '+err);
              return;
          }
          var demo = 10;
          response.host = urlParts.host;
          response.url = location;
          response.serverport = urlParts.port;
          response.configserver = urlParts.host+":"+urlParts.port;
          response.uuid = uuid;
          config.connection = response;
          if(response.configured === false){
               screen.log('Registered at '+urlParts.host+'. This client is not yet configured at this wallaby server. Starting demo mode in 10s.');
               var timer = new uv.Timer();
               timer.start(0, 1000, function () {
                    demo = demo - 1;
                    if(demo < 0){
                            screen.log('Running dashboard demo. Please configure this system on your server at '+urlParts.host+'.');
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
                        screen.log('Registered at '+urlParts.host+'. This client is not yet configured at this wallaby server. Starting demo mode in '+demo+'s');
                    }
               });
          } else {
               screen.log('Registered at '+urlParts.host+'. Starting playlist');
          }
      });
      } catch(e){
         log.error('Registration request failed : ',e);
      }
   }
}

 // for direct test in nucleus
if(typeof(Wally) === 'undefined')
{
        var playlist  = nucleus.dofile('playlist.js');
        context = nucleus.dofile('modules/wally/compat.js');
        var features = '[]';
        var request = nucleus.dofile('modules/request.js').request;
        ssdp(context,'en0',nucleus.getenv('W_SERVER'));
        try {
            nucleus.uv.run();
        } catch(e) {
            log.error(e);
        }
}

var request = nucleus.dofile('modules/request.js').request;
var decodeUrl = nucleus.dofile('modules/request.js').decodeUrl;
   
return { ssdp: ssdp };
    
})(this);
