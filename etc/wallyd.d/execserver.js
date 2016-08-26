(function(){

var uv = nucleus.uv;

p("Starting local TCP echo server...");

var server = new uv.Tcp();
server.bind("127.0.0.1", 1337);
server.listen(128, onConnection);
p("Server is now bound and listening for new connections...");
p(server.getsockname());

function onConnection(err) {
  if (err) throw err;
  var client = new uv.Tcp();
  server.accept(client);
  client.readStart(onRead);

  print("New TCP client accepted");
  p(client.getpeername());

  function onRead(err, chunk) {
    if (err) throw err;
    if (chunk) {
      client.write(wally.eval(ctx,chunk));
    }
    else {
      print("received EOF from client, shutting down...");
      client.shutdown(function () {
        client.close();
       // server.close();
      });
    }
  }
};
})()
