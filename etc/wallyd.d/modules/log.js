(function(){
"use strict";

var date = new Date();

function printlog()
{
      var d = date.getFullYear()+'-'+date.getMonth()+'-'+date.getDate()+" "+date.getHours()+':'+date.getMinutes();
      var levels = [ 'error', 'warn ', 'info ', 'debug', 'trace' ];
      var lvl = utils.colorize(levels[arguments[0]],levels[arguments[0]]);
      print("["+lvl+"]["+d+"] "+Array.prototype.map.call(arguments[1], utils.dump).join(" "));
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

return {
	error: log_error,
	warn: log_warn,
	info: log_info,
	debug: log_debug,
	fulldebug: log_fulldebug,
	trace: log_fulldebug
};

})();
