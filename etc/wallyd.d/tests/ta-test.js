nucleus.dofile('modules/wally/compat.js');
nucleus.dofile('modules/wally/transaction.js');

var myTA = new Transaction();

myTA.push( print.bind(null,"Hello World") );
myTA.push( print.bind(null,myTA.toString()));
myTA.push( print.bind(null,myTA.toString()));
myTA.commit();
