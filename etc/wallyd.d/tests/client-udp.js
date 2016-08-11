var p = nucleus.dofile("deps/utils.js").prettyPrint;
var uv = nucleus.uv;

var client = new uv.Udp();
client.bind("0.0.0.0", 1900);
client.listen(128, onConnection);
p(client);

function onConnection(err,data){
   p(err,data);
}


//client.bind("239.255.255.250", 1900, function (err) {
//  if (err) throw err;
//  p("client connected", client, client.getpeername(), client.getsockname());
//  client.readStart(function (err, chunk) {
//    if (err) throw err;
//    p("client onread", chunk);
//    print(chunk);
//    if (!chunk) {
//      client.close();
//    }
//  });
//  client.write("M-SEARCH * HTTP/1.1\r\nHOST:239.255.255.250:1900\r\nMAN:\"ssdp:discover\"\r\nST:ssdp:all\r\nNT:freshx:wally\r\nMX:1\r\n\r\n", function (err) {
//    if (err) throw err;
//    //client.shutdown();
//  });
//});
uv.run();
