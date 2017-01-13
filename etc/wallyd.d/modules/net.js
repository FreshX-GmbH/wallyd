/*jshint node:true*/
'use strict';

var uv = require('uv');
var wrapSocket = require('./wrap-socket');

exports.createClient = createClient;
exports.decodeUrl = decodeUrl;

function createClient(options, callback) {
    var client = new uv.Tcp();
    client.connect(options.host || '127.0.0.1', options.port || 80, onConnection);

    function onConnection(err) {
        if (err) { return callback(err,null);}
        log.info('Connected to '+(options.host || '127.0.0.1')+':'+(options.port||80));
        var stream = wrapSocket(client, options.decode, options.encode);
        return callback(null,stream);
    }
    return client;
}

function decodeUrl(location){
      var temp = location.replace(/^http:\/\/|^https:\/\//,'');
      var proto= location.replace(temp,'');
      var hostandport = location.replace(proto,'').replace(/\/.*/,'');
      var host;
      var port = 80;
      if(hostandport.match(/:/)) {
           port = parseInt(hostandport.split(/:/)[1]);
           host = location.replace(proto,'').replace(/\/.*/,'').split(/:/)[0];
      } else {
           host = location.replace(proto,'').replace(/\/.*/,'');
           if(proto.match(/https:/)) {
               port = 443;
           } else {
               port = 80;
           }
      }
      var path  = location.replace(proto,'').replace(hostandport,'');
      if(path === ""){
            path = '/';
      }
      return {
          proto: proto,
          host: host,  
          port: port,
          path: path
      } 
}


