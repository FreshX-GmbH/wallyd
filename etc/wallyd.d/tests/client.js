nucleus.dofile("bootstrap.js");

var createClient = require("deps/net").createClient;
var httpCodec = require('deps/http-codec');
var uv = require('uv');

var request = createClient({
    host: '127.0.0.1',
    port: 5984,
    decode: httpCodec.encoder(),
    encode: httpCodec.decoder()
}, function(stream) {
    p(stream);
    stream.write('GET /fxcrm HTTP/1.0\n\n',function(err){
	if(err) throw err;
	p("Client write done",err);
    });
    stream.onRead = function(err,data){
	p(err,data);
    };
    stream.onData = function(err,data){
	p(err,data);
    };
});

p("New client : ", request);

request.readStart(onRead);

function onRead(err,res){
    p("Client read : ",err,res);
}


// Start the libuv event loop
uv.run();

print("Event loop exiting...");
