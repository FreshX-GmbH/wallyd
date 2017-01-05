
function Font(name,file,size) {
  return this.initialize(name,file,size);
}

Font.prototype = {

    initialize: function(name,file,size) {
	this.config = {};
	this.wally =  new Wally();

	//if(typeof(CFont) === 'undefined'){
	 //   log.warn('CFont object not available.');
	//}
	this.set(name,file,size);
    },

    validate: function(){
        if(this.config.name === undefined){
          return [false,'Font name not set'];
        }
 
        if(typeof(this.config.file) !== 'size' || typeof(this.config.size) !== 'number')
            return [false,'Font properties not valid'];
        }
        return true;
    },

    set: function (name,file,size) {
        if(typeof(name) === 'string')  { this.config.name  = name; }
        if(typeof(file) === 'string') { this.config.file = file; }
        if(typeof(size) === 'number') { this.config.size = size; }
        return this.validate();
    },

    // create(name, file, size)
    create: function(name, file,size){
      if(this.validate() !== true) {
      	  if(this.set(name,file,size) !== true){
      	      if(this.validate() === false){
		  print("Font not valid : "+JSON.stringify(this.config));
		  return false;
	      }
      	  }
      }
      print("Createing Font : "+JSON.stringify(this.config));
      this.wally.loadFont(this.config.name,this.config.file,this.config.size);
      this.config.create = true;
    },

    toString:   function() {
        return JSON.stringify(this.config);
    },
};
