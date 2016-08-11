/*jshint node:true*/
"use strict";

var uv = require('uv');
var wrapSocket = require('./wrap-socket');

exports.createClient = createClient;

// options.host -- tcp addr to bind to
// options.port -- tcp port to bind to
// options.backlog -- tcp backlog
// options.encode -- encode function
// options.decode -- decode function
// onClient(client)
//   client.read(callback) - read data from stream
//     client.read.update(newDecode) - update decoder
//   client.write(data, callback) - write data to stream
//     client.write.update(newEncode) - update encoder
//   client.socket - uv_tcp_t instance
function createClient(options, onServer) {
  var client = new uv.Tcp();

  client.connect(options.host || "127.0.0.1", options.port || 0, onConnection);

  function onConnection(err) {
    if (err) throw err;
    p("Connected to "+(options.host || "127.0.0.1")+":"+(options.port||80));
    var stream = wrapSocket(client, options.decode, options.encode);
    onServer(stream);
  }
  return client;
}
