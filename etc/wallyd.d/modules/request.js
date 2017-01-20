(function(){
'use strict';

var httpCodec = require('modules/http-codec');
var createClient = require('modules/net').createClient;
var decodeUrl = require('modules/net').decodeUrl;
var decoder = httpCodec.decoder;

//function decodeUrl(location){
//      var temp = location.replace(/^http:\/\/|^https:\/\//,'');
//      var proto= location.replace(temp,'');
//      var hostandport = location.replace(proto,'').replace(/\/.*/,'');
//      var host;
//      var port = 80;
//      if(hostandport.match(/:/)) {
//           port = parseInt(hostandport.split(/:/)[1]);
//           host = location.replace(proto,'').replace(/\/.*/,'').split(/:/)[0];
//      } else {
//           host = location.replace(proto,'').replace(/\/.*/,'');
//           if(proto.match(/https:/)) {
//               port = 443;
//           } else {
//               port = 80;
//           }
//      }
//      var path  = location.replace(proto,'').replace(hostandport,'');
//      if(path === ""){
//            path = '/';
//      }
//      return {
//          proto: proto,
//          host: host,  
//          port: port,
//          path: path
//      } 
//}
//

function request(location, callback)
{
      var url = decodeUrl(location);     
      var proto= url.proto;
      var host = url.host;
      var port = url.port;
      var path = url.path;

      log.debug('Proto: ',proto);
      log.debug('Host : ',host);
      log.debug('Port : ',port);
      log.debug('Path : ',path);

      try {
        var req = { 
            code: 200, method: 'GET', path: path,
            headers: [ 'User-Agent', 'seaduk', 
                       'Date', new Date().toUTCString(),
                       'Connection', 'keep-alive', 
                       'Content-Type', 'text/plain',
                       'Content-Length', 3 ]
        };
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
                        log.debug('read() error : ',err);
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
      } catch(e) {
          callback(e,null);
      }
}

return {
      request: request
}

})();
