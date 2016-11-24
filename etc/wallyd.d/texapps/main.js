'use strict';

var done = false;
var s = 'WallyTV2 starting ...'.split('');
var c = 0;
var mainTA = new Transaction();
var betaTA = new Transaction();
var writeTA = new Transaction();

mainTA.push(screen.setImageScaled.bind(null,'main',config.logo2));
mainTA.push(screen.render.bind(null,'main'));
//betaTA.push(screen.setText.bind(null,'main','black','chalk32',config.wally.width/2,config.wally.height*0.66,'beta'));
//betaTA.push(screen.render.bind(null,'main'));

try {
     var memTimer = new uv.Timer();
     var writeTimer = new uv.Timer();
     memTimer.start( 1500, 1500, function(){
	  //if(done === false){
	       mainTA.commit();
	       mainTA = betaTA;
	       done = true;
	  //} else {
	       mainTA.commit();
	       memTimer.stop();
	       memTimer.close();
	       writeTimer.start(20,20,function(){
		  var L=s[c];
		  if(L === undefined){
		      writeTimer.stop();
		      writeTimer.close();
		      return ;
		  }
     		  c++;
     		  var y=20*c;
     		  writeTA.push(screen.setTextUTF8.bind(null,'main','black','chalk32',config.wally.width*0.33+y,-80,L));
     		  writeTA.push(screen.render.bind(null,'main'));
		  writeTA.commit();
		  writeTA = new Transaction();
	       });
	  //}
     });
} catch(e) {
       log.error('Error in memdbg timer : '+e);
}

