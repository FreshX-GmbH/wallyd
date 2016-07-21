var tex = {};
var textures = {
    main      : { z: 10, x: 0,   y:    0,  w: '100%', h:-16, color : 'FFFFFF' },
    log       : { z: 30, x: 0,   y: '-20', w: '100%', h: 20, color : 'EEEEEE' }, 
    version   : { z: 40, x: -42, y: 0,     w: 42,     h: 16, color : 'FFFFFF' },
    network   : { z: 50, x: -20, y: -20,   w: 20,     h: 20, color : 'FFA500' }
}

for (var t in textures) {
    texture = new Texture(t,textures[t].z,textures[t].x,textures[t].y,textures[t].w,textures[t].h,textures[t].color);
    tex[t] = texture;
}
