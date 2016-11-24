
function Texture(name,x,y,z,h,w,color) {
  this.initialize(name,x,y,z,h,w,color);
}

Texture.prototype = {

    initialize: function(name,x,y,z,h,w,color) {
	this.config = {};	
	if(this.cTextureAvailable()){
	  this.ct = new CTexture();
	}
	this.set(name,x,y,z,h,w,color);
    },

    set: function (name,x,y,z,h,w,color) {
	if(typeof(name) === 'string') { this.config.name = name; }
	if(typeof(color) === 'object') { this.config.color = color; }
	if(['string','number'].indexOf(typeof(x))) { this.config.x = x; }
	if(['string','number'].indexOf(typeof(y))) { this.config.y = y; }
	if(['string','number'].indexOf(typeof(z))) { this.config.z = z; }
	if(['string','number'].indexOf(typeof(h))) { this.config.h = h; }
	if(['string','number'].indexOf(typeof(w))) { this.config.w = w; }
	return this.validate();
    },

    cTextureAvailable: function() {
	if(typeof(CTexture) === 'undefined'){
	  log.warn('CTexture not available.');
	  return false;
	} else {
	  return true;
	}
    },

    validate: function(){
	if(this.config.name === null){
	  return [false,'Texture name not set'];
	}
	if(this.validNumber(this.config.x) === false || this.validNumber(this.config.y) === false ||
	    this.validNumber(this.config.z) === false ||
	    this.validNumber(this.config.w) === false || this.validNumber(this.config.h) === false){
	    return [false,'Texture properties not valid'];
	}
	if(typeof(this.config.color) !== 'color'){
	    return [false,'Texture color not of type Color'];
	}
	return true;
    },

    validNumber: function(num){
	if (!(typeof(num === 'string') && ((/^\d+(\.\d+)?%$/.test(num)) || (/^\d+(\.\d+)?$/.test(num))) )){
	  print('String not a valid number : '+num);
	  return false;
	}
	return true;
    },

    toString: function() {
        return JSON.stringify(this.config);
    },

    create: function(name,x,y,z,h,w,color) {
	if(this.validate() !== true){
	    if(this.set(name,x,y,z,h,w,color) !== true){
		return this.validate();
	    }
	}
	if(this.cTextureAvailable()){
		return [false,'cTexture not available'];
	}
//	return this.validate();
    }
};
