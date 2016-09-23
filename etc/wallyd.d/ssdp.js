nucleus.dofile('modules/bootstrap.js');
var p = nucleus.dofile('modules/utils.js').prettyPrint;
var createClient = require('modules/net').createClient;
var httpCodec = require('modules/http-codec');
var decoder = httpCodec.decoder;
var uv = require('uv');

var context = nucleus.dofile('modules/compat.js');

var discovery =true;

var searchST = 'freshx:wally';
var broadcastPort = 1900;
var broadcastAddress = '239.255.255.250';
var broadcastString = 'M-SEARCH * HTTP/1.1\r\nHOST:'+broadcastAddress+':'+broadcastPort+'\r\nMAN:\'ssdp:discover\'\r\nST:ssdp:all\r\nNT:'+searchST+'\r\nMX:1\r\n\r\n';

var server = new uv.Udp();
var location = '';

server.bind('0.0.0.0',8080);
server.broadcast(true);

server.send(broadcastAddress, broadcastPort, broadcastString , function (err) {
  if (err) {throw err;}
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

var timer1 = new uv.Timer();
timer1.start(1000, 2000, function () {
  if (discovery !== true) {
       timer1.stop();
       timer1.close();
       registerClient(location);
       return;
  }
  server.send(broadcastAddress, broadcastPort, broadcastString , function (err) {
    if(err) {throw err;}
    log.info('Sent another ssdp package');
  });
});

function registerClient(location){
  var port,url;
  var temp = location.replace(/^http:\/\/|^https:\/\//,'').replace('/.*','');
  var port = parseInt(temp.split(/:/) ? temp.split(/:/)[1].replace(/\/.*|\&.*|\?.*/,'') : 80);
  var host = temp.replace(/:.*|\/.*|\&.*|\?.*/,'');
  var mac = '00:00:00:00:08:15';
  var uuid= mac.replace(/:/g,'');
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

      //var client = new uv.Tcp();
      //client.connect(host, parseInt(port), function (err) {
      //if (err) {throw err;}
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
              response = JSON.parse(data);
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

// for direct test in nucleus
if(typeof(Wally) !== 'function'){
  var utils = nucleus.dofile('modules/utils.js')
  log=context.log;
  uv.run();
}
