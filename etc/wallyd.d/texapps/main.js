'use strict';

var mainTA = new Transaction();

mainTA.push(screen.setImageScaled.bind(null,'main',config.logo2));
mainTA.push(screen.render.bind(null,'main'));

try {
     var memtimer = new uv.Timer();
     memtimer.start( 1500, 1500, function(){
	 mainTA.commit();
	 memtimer.stop();
	 memtimer.close();
     });
} catch(e) {
       log.error('Error in memdbg timer : '+e);
}

