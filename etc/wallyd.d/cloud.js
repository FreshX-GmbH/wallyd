nucleus.dofile('modules/bootstrap.js');
var p = nucleus.dofile('modules/utils.js').prettyPrint;
var createClient = require('modules/net').createClient;
var httpCodec = require('modules/http-codec');
var decoder = httpCodec.decoder;
var uv = require('uv');
var config;

if(typeof(Wally) === 'function'){
  config = context.config;
}
var utils = nucleus.dofile('modules/utils.js')
var log   = nucleus.dofile('modules/log.js');

function registerClient(location){
  var port,url;
  var port = 3333
  var host = location;
  var mac = '00:00:00:00:08:15';
  var uuid= mac.replace(/:/g,'');
  var response = {};
  var url = "/wallyRegister?" +
  'uuid=' + uuid +
  '&mac=' + mac;
  log.debug('Connecting to host : '+host+':'+port);

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
          'x-ssl-client-s-dn', '/OU=FreshX/CN=wallyBlue-01/',
          'x-signature', 'none',
          'x-type', 'wally',
          'Content-Length', '14'
      ]},function(err){
        if(err) { throw err;}
        server.read(function (err, data) {
            if (err) { throw err; }
            p('Header : ',data.toString()); 
        });
      });
      server.write('{ping:true;}\r\n',function(err){
        if(err) { throw err;}
        server.read(function (err, data) {
            if (err) { throw err; }
            try{
              response = JSON.parse(data);
            } catch(err){
              log.debug('Registration failed : '+err);
              log.debug(data.toString());
              return;
            }
	    response.host = host;
  	    response.url = location;
  	    response.serverport = port;
  	    response.uuid = uuid;
            config.connection = response;
            config.connection.client = client;
            config.connection.server = server;
            sendUpdate();
         });
      });
  });
}

function sendUpdate(){
  var port,url;
  if(!context && !context.config && !context.config.conn){
    log.error('Not connected');
  }
  var conn = context.config.connection;
  var url = "/wallyUpdate";
  var server = connection.server;
  var client = createClient({
    host: connection.host,
    port: 3333,
    encode: httpCodec.encoder(),
    decode: httpCodec.decoder()},
    function(server){
     server.write({
       code: 200,
       method: 'GET',
       path: url,
       headers: [
          'User-Agent', 'seaduk',
          'Date', new Date().toUTCString(),
          'Connection', 'closed',
          'Content-Type', 'text/plain',
          'x-ssl-client-s-dn', '/OU=FreshX/CN=wallyBlue-01/',
          'x-signature', 'none',
          'x-type', 'wally',
          'Content-Length', '14'
     ]},function(err){
        if(err) { throw err;}
        server.read(function (err, data) {
            if (err) { throw err; }
            p('WU Resp : ',data); 
        });
     });
     server.write('{ping:true;}\r\n',function(err){
        server.read(function (err, data) {
            if (err) { throw err; }
            p('WU Data : ',data.toString()); 
        });
      });
  });
}

// for direct test in nucleus
if(typeof(Wally) !== 'function'){
  context = {};
  config={ 
    arch:'nucleus', 
    release:123 
  };
  context.config = config;
  context.screen = {};
  context.screen.log = function(a) { print(a);}
  print("Run as seaduk");
  registerClient("127.0.0.1");
  uv.run();
}
