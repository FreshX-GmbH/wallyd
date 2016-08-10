'use strict';

var Tcp = require('./modules/classes').Tcp;
var server, socket, client;
var port = 1338;

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
  if (err) { throw err; }
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
  if (err) { 
    log.error('Error : '+err); 
  }
  if (!err && data && data !== '\u0004' && !err) {
    var str=data.toString().replace('\n','').replace('\r','');
    try {
      var ret = wally.exec(str);
      //var ret = eval('wally.exec("'+str+'");');
      if(ret){
	this.write(ret+'\n');
      } else {
	this.shutdown();
	this.readStop();
        this.close();
      }
    }
    catch (error) {
      this.write(error.toString()+'\n');
    }
  }
  else {
    this.shutdown();
    this.readStop();
    this.close();
  }
};

server = new Server("127.0.0.1", port);
