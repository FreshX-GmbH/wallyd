/*jshint node:true*/
'use strict';

var uv = require('uv');
var wrapSocket = require('./wrap-socket');

exports.createClient = createClient;

function createClient(options, onClient) {
  var client = new uv.Tcp();

  client.connect(options.host || '127.0.0.1', options.port || 80, onConnection);

  function onConnection(err) {
    if (err) {throw err;}
    log.info('Connected to '+(options.host || '127.0.0.1')+':'+(options.port||80));
    var stream = wrapSocket(client, options.decode, options.encode);
    onClient(stream);
  }
  return client;
}
