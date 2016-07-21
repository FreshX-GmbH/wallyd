'use strict';

var log = require('./modules/log.js');
var Tcp = require('./modules/classes.js').Tcp;
var server, socket, client;
var wally = new Wally();
var gui = new GUI();
var port = 1337;

function assert(cond, message) {
  if (!cond) {
    throw new Error(message || 'Assertion Failure');
  }
}

function Server(host, port) {
  Tcp.call(this);
  log.info('New server binding on '+host+' port '+ port, this);
  this.bind(host, port);
  this.listen(128, this.onConnection);
}

Server.prototype.__proto__ = Tcp.prototype;
Server.prototype.onConnection = function onConnection(err) {
  assert(this === server);
  if (err) { 
    log.error('Failed to start server : ',err);
  }
  socket = new ClientHandler(this);
  this.accept(socket);
  socket.readStart(socket.onRead);
};

function ClientHandler(server) {
  Tcp.call(this);
  this.server = server;
}

ClientHandler.prototype.__proto__ = Tcp.prototype;
ClientHandler.prototype.onRead = function onRead(err, data) {
  assert(this === socket);
  if (err) { throw err; }
  if (data && data !== 0x4) {
    try {
      var ret = eval(data.toString());
      this.write('{ result : ok, data : '+ ret + '}\n');
    }
    catch (error) {
      this.write('{ result : err, reason : '+ error.toString() + '}\n');
    }
  }
  else {
    this.shutdown();
    this.readStop();
    this.close();
  }
};

server = new Server('127.0.0.1', port);
