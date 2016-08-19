nucleus.dofile('modules/bootstrap.js');
var p = nucleus.dofile('modules/utils.js').prettyPrint;
var createClient = require('modules/net').createClient;
var httpCodec = require('modules/http-codec');
var decoder = httpCodec.decoder;
var uv = require('uv');

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
  print('SSDP Packet sent. Waiting for response');
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
	  print('Found location : '+location+', stopping SSDP discovery');
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
    print('Sent ssdp package');
  });
});

function registerClient(location){
  var port,url;
  var temp = location.replace(/^http:\/\/|^https:\/\//,'').replace('/.*','');
  var port = parseInt(temp.split(/:/) ? temp.split(/:/)[1].replace(/\/.*|\&.*|\?.*/,'') : 80);
  var host = temp.replace(/:.*|\/.*|\&.*|\?.*/,'');
  var mac = '00:00:00:00:08:15';
  var url = temp.replace(/^.*?\//,'/') + '?' +
  'uuid=' + mac.replace(/:/g,'') +
  '&arch='+config.arch  +
  '&platform=OSE-SD-' + config.arch +
  '&fw_version=' + config.release + 
  '&mac=' + mac;
  // wifi
  // ip
  print('Connecting to host : '+host+':'+port);

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
            p('Payload : ',JSON.parse(data));
            });
      });
  });
}

// for direct test in nucleus
if(typeof(Wally) !== 'function'){
  config={ 
    arch:'nucleus', 
    release:123 
  };
  uv.run();
}
