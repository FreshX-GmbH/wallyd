
function Texture(name,x,y,w,h,color) {
  this.initialize();
}

Texture.prototype = {

    name: 0,

    initialize: function() {
	this.funcs = [];
	if(typeof(CTexture) === 'undefined'){
	  log.warn('CTexture not available.');
	  this.ct = null;
	} else {
	  this.ct = new CTexture();
	}
    },

    toString:   function() {
        return '{ name : '+this.name+' }';
    }
};
