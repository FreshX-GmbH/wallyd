nucleus.dofile('modules/compat.js');
nucleus.dofile('modules/transaction.js');

var myTA = new Transaction();

myTA.push( print.bind(null,"Hello World") );
myTA.push( print.bind(null,myTA.toString()));
myTA.push( print.bind(null,myTA.toString()));
myTA.commit();
