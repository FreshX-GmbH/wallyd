"use strict";

var p = require('./utils.js').prettyPrint;
var fs = require('./fs.js');
var log = require('./log.js');

var curl=CurlPrototype.init();

module.exports = {
  getUrl: getUrl,
  download: download,
  downloadFile: downloadFile
}

function request(url, body, headers) {
  var chunks = [];
  var outHeaders = {};
  var length = 0;
  var end = false;

  curl.setopt("useragent", "DukLuv libcurl bindings");
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
    //body: JSON.parse(chunks.join(""))
    body: chunks.join("")
  };
}

function get(url, headers) {
  curl.reset();
  curl.setopt("httpget", true);
  return request(url, null, headers);
}

function put(url, body, headers) {
  curl.reset();
  curl.setopt("put", true);
  return request(url, body, headers);
}

function post(url, body, headers) {
  curl.reset();
  curl.setopt("post", true);
  return request(url, body, headers);
}

function downloadFile(url,file){
  log.debug("Downloading ",url);
  var headers = [];
  try {
    res = get(url, headers);
    fs.writeFileSync(file,res.body);
    log.debug("Downloaded ",url," to ",file);
  } catch(err){
    log.error(err);
    return false;
  }
  curl.reset();
  return true;
}

function download(url){
  log.debug("Downloading ",url);
  var headers = [];
  try {
    res = get(url, headers);
  } catch(err){
    log.error(err);
  }
  curl.reset();
  return res;
}

function getUrl(url){
  log.debug("Downloading ",url);
  var headers = [];
  try {
    return get(url, headers).body;
  } catch(err){
    log.error(err);
  }
}
