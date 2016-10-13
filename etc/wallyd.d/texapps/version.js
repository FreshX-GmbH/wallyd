'use strict';

log.info('Started version texapp');

var stat = 'R'+context.config.wally.release/1000+
         ' *** Res: '+context.config.wally.width+'x'+context.config.wally.height+
         ' *** Arch: '+context.config.wally.arch;
screen.setText('version','black','logfont',0,0,stat);
screen.log('Waiting for network to get ready.');
screen.render('version');
