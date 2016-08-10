(function(){
"use strict";

var date = new Date();

function pad(num, size) {
    var s = num+"";
    while (s.length < size) s = "0" + s;
    return s;
}

function hashCode(str) { // java String#hashCode
    var hash = 0;
    for (var i = 0; i < str.length; i++) {
       hash = str.charCodeAt(i) + ((hash << 5) - hash);
    }
    return hash;
} 

function intToRGB(i){
    var c = (i & 0x00FFFFFF)
        .toString(16)
        .toUpperCase();
    return "00000".substring(0, 6 - c.length) + c;
}

var getRO = function (obj, i) {
    if(typeof obj !== 'object' || obj === null) {
        return undefined;
    }
    if (i in obj) {
        return obj[i];
    }
    return undefined;
};

function concatPath(path, add) {
    var components;
    if (Array.isArray(add)) {
        components = add;
    } else if (typeof add === 'string') {
        var seperator = '.';
        if (add.indexOf('|') !== -1) {
            seperator = '|';
        }
        components = add.split(seperator);
    } else {
        throw new Error('[Utils] Can not add path: '+add);
    }
    Array.prototype.push.apply(path, components);
    return path;
};

function getValue(obj, path, def) {
    if (!obj || path === undefined) {
        return def;
    }
    path = exports.concatPath([], path);
    obj = path.slice(0, path.length - 1).reduce(getRO, obj);
    var idx = path[path.length - 1];
    if (obj !== null && typeof obj === 'object' && idx in obj && obj[idx] !== null && obj[idx] !== undefined) {
        return obj[idx];
    }
    return def;
};

function dumpJSON(obj){
  print(parseJSON(obj));
}

function parseJSON(obj)
{
  var cache=[];
  return JSON.stringify(obj,function(key, value) {
    if (typeof value === 'object' && value !== null) {
        if (cache.indexOf(value) !== -1) {
            // Circular reference found, discard key
            //cache.push("["+value+"]");
            return;
        }
        // Store value in our collection
        cache.push(value);
    }
    return value;
  },2);
}

return  {
    pad: pad,
    dumpJSON: dumpJSON,
    parseJSON: parseJSON,
    hashCode: hashCode,
    concatPath: concatPath,
    getValue: getValue
};

})()

