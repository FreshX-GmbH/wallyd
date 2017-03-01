/*jshint node:true*/
"use strict";

module.exports = wrapSocket;

function wrapSocket(socket, decode, encode) {
  var readBuffer;
  function read(){
    socket.readStart(onRead);
  }

  function onRead(err, chunk) {
    log.debug("onRawRead", err, chunk);
    log.debug("onRawRead", err, (''+chunk).length);
    readBuffer = decode.concat(readBuffer, chunk);
    if (!chunk) {
      checkClose();
    }
  }

  function checkClose() {
      socket.close();
  }

  return {
    read: read,
    socket: socket
  };
}
