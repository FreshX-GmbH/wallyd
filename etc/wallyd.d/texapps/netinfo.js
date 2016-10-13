'use strict';

var wally = new Wally();
var gui = new GUI();
var date = new Date();
var uv = nucleus.uv;

log.info('Started netinfo texapp');

function oninterval() {
    try {
        var name = 'N/A';
        var server = 'N/A';
	var ip;
	if(typeof uv.interface_addresses !== 'function'){
		ip = 'N/A';
	}
        if(typeof(config.conn) !== 'undefined' && typeof(config.conn.name) !== 'undefined' ){
            name=config.conn.name;
        }
        if(typeof(config.conn) !== 'undefined' && typeof(config.conn.host) !== 'undefined' ){
            server=config.conn.host;
	}
	var tstat = "IP: "+config.network.ip+" / Name : "+name+" / Server : "+server;
        gui.clearTexture('netinfo');
	wally.setText('netinfo','black','logfont',0,1,tstat);
	log.error(config);
    } catch(err) {
	log.error('Error in netinfo : '+err);
    }
}

try {
    var nettimer = new uv.Timer();
    nettimer.start( 0, 2000, oninterval);
} catch(e) {
    log.error('Error in netinfo timer : '+e);
}
