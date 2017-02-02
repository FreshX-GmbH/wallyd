nucleus.dofile('modules/bootstrap.js');
var request = nucleus.dofile('modules/request.js').request;
var log = nucleus.dofile('modules/log.js');

function getImage(url){
    log.debug('Requesting url ',url);
    request(url, function(err,header,body){
        if(err) {
           log.error('Could not connect to server : '+err);
           return(err);
        }
        try {
           log.info('H:',header);
           log.info('B:',body.length);
           //log.info('B:',body);
        } catch(e) {
           log.error('Could not decode server response : ',e,body);
        }
        log.info('request done.');
    });
    log.info('getImage done.');
}

log.info(request);
getImage('http://127.0.0.1:1112/wally1920.png');

nucleus.uv.run();
