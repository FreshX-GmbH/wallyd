nucleus.dofile('modules/wally/compat.js');
nucleus.dofile('modules/wally/color.js');
nucleus.dofile('modules/wally/texture.js');

var myColor =   new Color('green',Color.Green);
var myColor2 =  new Color('red',0xff0000);
var myTexture = new Texture('main');
var myTexture2 = new Texture('version');

print('Green : ',myColor);
print('Red : ',myColor2);
print(myTexture);
print(myTexture2);
print(myTexture.create(null,0,0,10,10,10,myColor));
print(myTexture.toString());
