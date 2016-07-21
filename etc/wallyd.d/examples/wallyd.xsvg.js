'use strict';

var date = new Date();
var config = wally.getConfig();
var utils = require('./modules/utils.js');
var p = utils.prettyPrint;
var svg = new SVG();

var file = config.basedir+'/etc/wallyd.d/tests/qr.svg';

svg.svgToPng(file,'/tmp/qr.png');
