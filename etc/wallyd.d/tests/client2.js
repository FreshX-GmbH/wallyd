var p = nucleus.dofile("deps/utils.js").prettyPrint;
var uv = nucleus.uv;

var client = new uv.Tcp();
client.connect("127.0.0.1", 5984, function (err) {
  if (err) throw err;
  p("client connected", client, client.getpeername(), client.getsockname());
  client.readStart(function (err, chunk) {
    if (err) throw err;
    p("client onread", chunk);
    print(chunk);
    if (!chunk) {
      client.close();
    }
  });
  client.write("GET /fxcrm HTTP/1.0\n\n", function (err) {
    if (err) throw err;
    client.shutdown();
  });
});
uv.run();
