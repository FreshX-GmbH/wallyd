(function(){
'use strict';

var httpCodec = require('modules/http-codec');
var createClient = require('modules/net').createClient;
var decoder = httpCodec.decoder;

function request(location, callback){
      var temp = location.replace(/^http:\/\/|^https:\/\//,'').replace('/.*','');
      var port = parseInt(temp.split(/:/) ? temp.split(/:/)[1].replace(/\/.*|\&.*|\?.*/,'') : 80);
      var host = temp.replace(/:.*|\/.*|\&.*|\?.*/,'');
      var url = temp.replace(/.*\//,'/');
      log.debug('PATH : '+url);

      try {
        var req = { 
            code: 200, method: 'GET', path: url,
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
          },function(server){
             server.write(req, function(err){
                if (err) { 
                    return callback(err,null); 
                }
                server.read(function (err, header) {
                    if (err) { 
                        return callback(err,null); 
                    }
                    server.write('0\r\n',function(err){
                        if (err) { 
                            return callback(err,null); 
                        }
                        server.read(function (err, body) {
                            if (err) { 
                                return callback(err,null); 
                            }
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
    
return { request: request };

})();
