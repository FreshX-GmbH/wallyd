(function(){

var uv = nucleus.uv;

log.info("Starting local TCP echo server...");
log.info(context);

var server = new uv.Tcp();
server.bind("127.0.0.1", 1337);
server.listen(128, onConnection);
log.debug("Server is now bound and listening for new connections...");
log.debug(server.getsockname());

function onConnection(err) {
  if (err) throw err;
  var client = new uv.Tcp();
  server.accept(client);
  client.readStart(onRead);

  log.info("New TCP client accepted from "+client.getpeername().ip+":"+client.getpeername().port);
  function onRead(err, chunk) {
    if (err) throw err;
    if (chunk) {
      try {
        log.debug("Script : "+chunk);
        res = wally.eval(chunk);
        log.debug("Return : "+res);
        client.write("{ error : false, res : "+res+"}",function(res,err){
          log.debug(res,err);
        });
      } catch(e) {
        client.write("{ error : true, res : "+e+"}",function(res,err){
          log.debug(res,err);
        });
      }
    }
    log.info("received EOF from client, shutting down...");
    client.shutdown(function () {
       client.close();
    });
  }
};
})()
