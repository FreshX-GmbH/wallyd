
// Register uv as a require-able module.
Duktape.modLoaded.uv = {exports:nucleus.uv};
Duktape.modLoaded.log = nucleus.dofile('modules/log.js');
Duktape.modLoaded.utils = nucleus.dofile('modules/utils.js');
Duktape.modLoaded.console = {log:print};

if(typeof(CurlPrototype !== "undefined")){
    Duktape.modLoaded.CurlPrototype=CurlPrototype;
}

// Bootstrap require by reusing Duktape's default behavior
// It's mostly node.js like.
Duktape.modSearch = function (id) {
  "use strict";
  var filename = id + ".js";
  var js = nucleus.readfile(filename);
  if (typeof js !== "string") {
    throw new Error("No such file in bundle: " + filename);
  }
  return js;

};
