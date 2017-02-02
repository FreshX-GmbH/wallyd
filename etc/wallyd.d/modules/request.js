(function(){
'use strict';

var httpCodec = require('modules/http-codec');
var createClient = require('modules/net').createClient;
var decodeUrl = require('modules/net').decodeUrl;
var decoder = httpCodec.decoder;

if(!log){
      var log = {
            error: print,
            debug: print,
            info: print
      }
}

function request(location, optionalHeaders, callback)
{
      log.error('REQ : ',typeof(optionalHeaders),arguments.length,typeof(callback));
      // second parameter is either the optHeaders or the callback
      // if its optHeaders, the third parameter is the callback
      if(arguments.length === 2 && typeof(optionalHeaders) === 'function'){
            callback = optionalHeaders;
            optionalHeaders = [];
      } else {
            optionalHeaders = typeof optionalHeaders !== 'undefined' ? optionalHeaders : [];
      }

      log.error('REQ : ',typeof(optionalHeaders),arguments.length,typeof(callback));

      var url = decodeUrl(location);     
      var proto= url.proto;
      var host = url.host;
      var port = url.port;
      var path = url.path;
      var keepalive = false;

      log.debug('Proto: '+proto+'  Host : '+host +'  Port : '+port +' Path : '+path);

      var headers = [ 'User-Agent', 'seaduk', 
                       'Date', new Date().toUTCString(),
                       //'Connection', 'keep-alive', 
                       'Connection', 'Close', 
                       'Content-Type', 'text/plain',
                       'Host', host ];
      for(var i = 0; i < optionalHeaders.length; i+=2){
           var idx = headers.indexOf(optionalHeaders[i]);
           if(idx === -1){
                 log.debug('Adding',optionalHeaders[i],'to headers');
                 headers.push(optionalHeaders[i],optionalHeaders[i+1]);
           } else {
                 log.debug('Element',headers[idx],'already in headers');
                 headers[idx+1]=optionalHeaders[i+1];
           }
      }
      for(var i = 0; i < headers.length; i+=2){
           if(headers[i].toLowerCase() === "connection" && headers[i+1].toLowerCase() === "keep-alive") {
               log.debug('Connection keep-alive, we will not close the connection');
               keepalive=true;
           }
      }
      try {
        nucleus.uv.getaddrinfo({ node: host }, function (err, results) {
            if(err) {
                return err;
            }
            if(results[0] && results[0].addr){
                  host = results[0].addr;
                  log.debug("DNS lookup of "+url.host+" resolves to "+host);
            } else {
                  log.warn("Could not resolve ",host);
            }
            var req = { code: 200, method: 'GET', path: path, headers : headers };
            var header = '';
            var client = createClient({
              host: host,
              port: port,
              encode: httpCodec.encoder(),
              decode: httpCodec.decoder()
              },function(err,server){
                 log.debug('create()');
                 if (err) { 
                       log.error('createClient() error : ',err);
                       return callback(err,null); 
                 }
                 server.write(req, function(err,data){
                    log.debug('write() : ',err);
                    if (err) { 
                        log.debug('write() error : ',err);
                        log.error(err,data);
                        server.socket.close();
                        return callback(err,null); 
                    }
                    server.read(function (err, header) {
                        log.debug('readH() : ',err,header);
                        if (err) { 
                            log.debug('initial read() error : ',err);
                            server.socket.close();
                            return callback(err,null); 
                        }
                        server.read(function (err, body) {
                            log.debug('readB() : ',err);
                            if (err) { 
                                log.debug('read() error : ',err);
                                server.socket.close();
                                return callback(err,null); 
                            }
                            if(keepalive === false){
                                server.socket.close();
                            }
                            return callback(null,header,body);
                        });
                    });
                 });
             });
        });
        log.warn("request call exit");
      } catch(e) {
//          log.warn("request call exit");
          return callback(e,null);
      }
}

return {
      request: request
}

})();
