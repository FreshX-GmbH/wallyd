(function(){
"use strict";

var CurlPrototype;
if(typeof(Duktape.modLoaded.CurlPrototype) === "undefined"){
	log.error("Curl plugin not available");
	return undefined;
} else {
	CurlPrototype = Duktape.modLoaded.CurlPrototype;
}

var curl = CurlPrototype.init();

function request(url, body, headers) {
  var chunks = [];
  var outHeaders = {};
  var length = 0;
  var end = false;

  curl.setopt("useragent", "WallyCurl");
  curl.setopt("url", url);
  curl.setopt("followlocation", true);

  // TODO : configurable
  log.debug('Disabling SSL peer verification');
  curl.setopt("ssl-verifypeer",false);
  curl.setopt("ssl-verifyhost",0);

  curl.setopt("writefunction", function (chunk) {
    chunks.push(chunk);
    length += chunk.length;
    return chunk.length;
  });

  curl.setopt("headerfunction", function (header) {
    var length = header.length;
    header = header.toString();
    if (end) {
      // Only remember headers for last response.
      end = false;
      outHeaders = {};
    }
    if (header === "\r\n") {
      end = true;
    }
    var match = header.toString().match(/^([^:]+): *([^\r]+)/);
    if (match) {
      outHeaders[match[1].toLowerCase()] = match[2];
    }
    return length;
  });

  if (body) {
    if ((typeof body) !== "string") {
      body = JSON.stringify(body) + "\n";
      headers = headers || [];
      headers.push("Content-Type: application/json");
    }
    body = Duktape.Buffer(body);
    headers.push("Content-Length: " + body.length);
    curl.setopt("infilesize", body.length);
    curl.setopt("readfunction", function (size) {
      if (!body) { return ""; }
      var chunk;
      if (body.length < size) {
        chunk = body;
        body = "";
        return chunk;
      }
      throw new Error("TODO: handle large file uploads");
    });
  }

  if (headers) {
    curl.setopt("httpheader", headers);
  }

  curl.perform();

  return {
    code: curl.getinfo("response-code"),
    headers: outHeaders,
    body: chunks.join("")
  };
}

function get(url, headers) {
  try {
      curl.reset();
      curl.setopt("httpget", true);
      log.debug("HTTP GET : "+url);
      log.debug("HTTP HEADER : ",headers);
      return request(url, null, headers);
  }catch(e){
	log.error("Curl get failed : "+e);
  }
}

function put(url, body, headers) {
  try {
      curl.reset();
      curl.setopt("put", true);
      log.debug("HTTP PUT : "+url);
      log.debug("HTTP HEADER : "+headers);
      return request(url, body, headers);
  }catch(e){
	log.error("Curl put failed : "+e);
  }
}

function post(url, body, headers) {
  try {
      curl.reset();
      curl.setopt("post", true);
      log.debug("HTTP POST : "+url);
      log.debug("HTTP HEADER : "+headers);
      return request(url, body, headers);
  }catch(e){
	log.error("Curl post failed : "+e);
  }
}

function basicauth(user, password){
	return ["Authorization: Basic "+Duktape.enc('base64',user+":"+password)];
	//return ["BasicAuth: "+Duktape.enc('base64',user+":"+password)];
}

return { 
           get: get,
           put: put,
           post: post,
	   mkBasicAuth:basicauth,
	   CurlPrototype: CurlPrototype
};
//
})();
