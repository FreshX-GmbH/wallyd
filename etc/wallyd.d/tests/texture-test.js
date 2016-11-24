nucleus.dofile('modules/wally/compat.js');
nucleus.dofile('modules/wally/color.js');
nucleus.dofile('modules/wally/texture.js');

var myColor = new Color('green','304020');
var myTexture = new Texture('main');

print('Green : ',Color);
print('Color : ',myColor);
print(myTexture.toString());
print(myTexture.create(null,0,0,10,10,10,myColor));
print(myTexture.toString());
