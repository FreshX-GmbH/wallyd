
function Texture(name,z,x,y,w,h,color) {
  this.initialize(name,z,x,y,w,h,color);
}

Texture.prototype = {

    initialize: function(name,z,x,y,w,h,color) {
	this.config = {};
	this.wally = new Wally();
	//if(this.cTextureAvailable()){
	//    this.ct = new CTexture();
	//}
	this.set(name,z,x,y,w,h,color);
    },

    set: function (name,z,x,y,w,h,color) {
	if(typeof(name) === 'string') { this.config.name = name; }
	if(typeof(color) === 'object') { this.config.color = color.string; }
	if(typeof(color) === 'string') { this.config.color = color; }
	//if(['string','number'].indexOf(typeof(x))) { 
	this.config.x = String(x);
	//if(['string','number'].indexOf(typeof(y))) {
	this.config.y = String(y);
	//if(['string','number'].indexOf(typeof(z))) { 
	this.config.z = String(z);
	//if(['string','number'].indexOf(typeof(h))) { 
	this.config.h = String(h);
	//if(['string','number'].indexOf(typeof(w))) { 
	this.config.w = String(w);
	//print(JSON.stringify(this.config));
	return this.validate();
    },

    cTextureAvailable: function() {
	//if((typeof(CTexture) === 'undefined') || (typeof(Wally) === 'undefined')){
	if((typeof(Wally) === 'undefined')){
	  //print('CTexture/Wally not available.');
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
//	    return [false,'Texture properties not valid'];
	}
	//if(typeof(this.config.color) !== 'object'){
	//    return [false,'Texture color not of type Color'];
	//}
	return true;
    },

    validNumber: function(num){
	if (typeof(num === 'string') && (/^\d+(\.\d+)?%$/.test(num))){
	  if(parseInt(num) === false){
	    print('String not a valid number : '+num);
	    return false;
	  }
	}
	return true;
    },

    toString: function() {
        return JSON.stringify(this.config);
    },

    create: function(name,z,x,y,w,h,color) {
	print('Create');
	if(this.validate() !== true){
	    print('Create validate');
	    if(this.set(name,z,x,y,w,h,color) !== true){
		print('Create set');
		if(this.validate() === false){
		    return false;
		}
	    }
	}
	print("createTexture(",this.config.name,this.config.z,this.config.x,this.config.y,this.config.w,this.config.h,this.config.color+")");
	this.wally.createTexture(this.config.name,this.config.z,this.config.x,this.config.y,this.config.w,this.config.h,this.config.color);
	this.config.created = true;
	//return this.validate();
    }
};
