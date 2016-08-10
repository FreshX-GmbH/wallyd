dukluv-qr
========

This is yet another QR Code generator for duktape JS engine running under dukluv

Overview
--------

  * No dependecies;
  * generate image in `svg`, `eps` and `pdf` formats;
  * png not yet supported

Example:
```dukluv
var qr = require('./qr/qr.js');

var qr_svg = qr.image('I love QR!', { type: 'svg' });
print(qr_svg);

qr.file('I love QR!','./qr.svg', { type: 'svg' });
```

