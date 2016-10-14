(function(){
"use strict";

var currentTA=0;
var ta = new Transaction();
var taFunctions = {};

function start()
{
    currentTA++;
    taFunctions[currentTA] = [];
    log.info(ta.name);
    ta.newTransaction("TA-"+currentTA);
    return currentTA;
}

function push(a,f,p){
    log.error(f);
    taFunctions[a].push([f,p]);
}

function abort(a){
    delete taFunctions[a];
}

function commit(a){
    var commitTA = taFunctions[a];
    ta.startTransaction("TA-"+a);
    log.debug("Commiting transaction "+a+" with "+commitTA.length+" elements");
    for(var i = 0; i < commitTA.length; i++){
 	log.debug(commitTA[i][0]+"("+commitTA[i][1]+")");
	log.debug("Ret : +",commitTA[i][0](commitTA[i][1]));
    }
    ta.commitTransaction("TA-"+a);
}

return  {
    start: start,
    abort: abort,
    commit: commit,
    push: push
};

})()

