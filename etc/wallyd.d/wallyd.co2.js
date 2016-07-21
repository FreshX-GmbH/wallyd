'use strict';

var fs = require('./modules/fs.js');
var log = require('./modules/log.js');

var file = "./etc/wallyd.d/tests/temp.json";
var co = [];

return;

var data = fs.readFileSync(file)
var logs = JSON.parse(data);

for(var i = 0; i < logs.rows.length; i++){
  var obj = logs.rows[i].doc;
  var time = obj._id.split(':')[1];
  co.push([ time, obj.temp, obj.co2 ]);
}
co.sort(function(a, b) {return a[0] - b[0]})
for(var i = 0; i < co.length; i++){
  log.debug("x1:",i,"y1: 100%", "x2:",i, "y2:", co[i][2],'0x222222'); 
}
