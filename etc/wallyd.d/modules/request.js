(function(){
'use strict';

var httpCodec = require('modules/http-codec');
var createClient = require('modules/net').createClient;
var decodeUrl = require('modules/net').decodeUrl;
var decoder = httpCodec.decoder;

function request(location, callback, optionalHeaders)
{
      optionalHeaders = typeof optionalHeaders !== 'undefined' ? optionalHeaders : [];

      var url = decodeUrl(location);     
      var proto= url.proto;
      var host = url.host;
      var port = url.port;
      var path = url.path;

      log.debug('Proto: '+proto+'  Host : '+host +'  Port : '+port +' Path : '+path);

      var headers = [ 'User-Agent', 'seaduk', 
                       'Date', new Date().toUTCString(),
                       'Connection', 'Close', 
                       'Content-Type', 'text/plain',
                       'Host', host,
                       'Content-Length', 3 ];
      for(var i = 0; i < optionalHeaders.length; i+=2){
           var idx = headers.indexOf(optionalHeaders[i]);
           if(idx === -1){
                 log.debug('Adding ',optionalHeaders[i],' to headers');
                 headers.push(optionalHeaders[i],optionalHeaders[i+1]);
           } else {
                 log.debug('Element ',headers[idx],' already in headers');
                 headers[idx+1]=optionalHeaders[i+1];
           }
      }
      try {
        uv.getaddrinfo({ node: host }, function (err, results) {
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
                 if (err) { 
                       log.error('createClient() error : ',err);
                       return callback(err,null); 
                 }
                 server.write(req, function(err){
                    if (err) { 
                        log.debug('write() error : ',err);
                        server.socket.close();
                        return callback(err,null); 
                    }
                    server.read(function (err, header) {
                        if (err) { 
                            log.debug('initial read() error : ',err);
                            server.socket.close();
                            return callback(err,null); 
                        }
                        server.write('0\r\n',function(err){
                            if (err) { 
                                log.debug('write() error : ',err);
                                server.socket.close();
                                return callback(err,null); 
                            }
                            server.read(function (err, body) {
                                if (err) { 
                                    log.debug('read() error : ',err);
                                    server.socket.close();
                                    return callback(err,null); 
                                }
                                server.socket.close();
                                return callback(null,header,body);
                            });
                        });
                    });
                 });
             });
        });
      } catch(e) {
          callback(e,null);
      }
}

return {
      request: request
}

})();
