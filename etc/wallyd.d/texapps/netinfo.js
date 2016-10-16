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
	var TA = new Transaction();
	TA.push(gui.clearTexture.bind(null,'netinfo'));
	TA.push(wally.setText.bind(null,'netinfo','black','logfont',0,1,tstat));
	TA.push(wally.render.bind(null,'netinfo'));
	TA.commit();
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
