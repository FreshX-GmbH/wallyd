(function(){
"use strict";

var extra = nucleus.dofile('modules/extra.js');

function printlog()
{
      var date = new Date();
      var d = date.getFullYear()+'-'+extra.pad(date.getMonth()+1,2)+'-'+extra.pad(date.getDate(),2)+" "
	     +extra.pad(date.getHours(),2)+':'+extra.pad(date.getMinutes(),2)+':'+extra.pad(date.getSeconds(),2);
      var levels = [ 'error', 'warn ', 'info ', 'debug', 'trace', 'screen' ];
      var prefix = "";
      //if(arguments[0] === 5){
      //      prefix = "[SCREEN] "
      //      arguments[0] = 3;
      //}
      if(typeof(utils) !== 'undefined'){
            var lvl = utils.colorize(levels[arguments[0]],levels[arguments[0]]);
	    print("["+lvl+"]["+d+"]"+prefix+" "+Array.prototype.map.call(arguments[1], utils.dump).join(" "));
      } else if(typeof(context) !== 'undefined' && typeof(context.utils) != 'undefined'){
            var lvl = context.utils.colorize(levels[arguments[0]],levels[arguments[0]]);
	    print("["+lvl+"]["+d+"]"+prefix+" "+Array.prototype.map.call(arguments[1], context.utils.dump).join(" "));
      } else {
	    var lvl = levels[arguments[0]];
	    print("["+lvl+"]["+d+"] "+Array.prototype.map.call(arguments[1], JSON.stringify).join(" "));
      }

}

function log_error(){
      printlog(0,arguments);
}
function log_warn(){
      printlog(1,arguments);
}
function log_info(){
      printlog(2,arguments);
}
function log_debug(){
      printlog(3,arguments);
}
function log_fulldebug(){
      printlog(4,arguments);
}
function log_screen(){
      printlog(5,arguments);
}

return {
	error: log_error,
	warn: log_warn,
	info: log_info,
	debug: log_debug,
	fulldebug: log_fulldebug,
	trace: log_fulldebug,
	screen: log_screen
};

})();
