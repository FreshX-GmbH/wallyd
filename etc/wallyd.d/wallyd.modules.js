//var utils = require('./modules/utils.js');

var client = {}, video = {}, v2 = {}, wallaby = {};

try {
    client = new CloudClient(function(){
	print('Cloud object created');
    });
} catch(err) {
    print('Could not acquire cloud client');
}
try {
    video = new FFVideo();
    v2 = new FFVideo();
} catch(err) {
    print('Could not acquire video client');
}

//try {
//    //wallaby = require(config.modules+'/wallaby.js');
////    wallaby = require(config.modules+'/modules/wallaby.js');
//    //utils.prettyPrint(wallaby);
//    //wallaby = new Wallaby('AAA'); 
//    wallaby.renderWallabyData('BBB');
//} catch(err) {
//    print('Could not acquire wallaby client : '+err);
//}
