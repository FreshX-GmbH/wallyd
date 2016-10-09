(function(){
"use strict";

if(typeof(CurlPrototype === "undefined")){
	log.error("Curl plugin not available");
	return undefined;
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
  curl.reset();
  curl.setopt("httpget", true);
  log.debug("HTTP GET : "+url);
  return request(url, null, headers);
}

function put(url, body, headers) {
  curl.reset();
  curl.setopt("put", true);
  log.debug("HTTP PUT : "+url);
  return request(url, body, headers);
}

function post(url, body, headers) {
  curl.reset();
  curl.setopt("post", true);
  log.debug("HTTP POST : "+url);
  return request(url, body, headers);
}

return {
           get: get,
           put: put,
           post: post
};

})();
