#ifndef WALLY_GPIOPLUGIN
#define WALLY_GPIOPLUGIN

#include "gpio.h"

duk_context *ctx = NULL;
pluginHandler *ph;

typedef struct gpioPluginStructure
{
        const char *name;
        int GPIO;
        int direction;
} myGPIO;

extern const duk_function_list_entry gpioMethods[];

static int GPIOExport(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;
 
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open export for writing!\n");
        return(-1);
    }
 
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
}
 
static int GPIOUnexport(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;
 
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return(-1);
    }
 
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
}
 
static int GPIODirection(int pin, int dir)
{
    static const char s_directions_str[]  = "in\0out";
 
    char path[DIRECTION_MAX];
    int fd;
 
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return(-1);
    }
 
    if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
        fprintf(stderr, "Failed to set direction!\n");
        return(-1);
    }
 
    close(fd);
    return(0);
}
 
static int GPIORead(int pin)
{
    char path[VALUE_MAX];
    char value_str[3];
    int fd;
 
    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        return(-1);
    }
 
    if (-1 == read(fd, value_str, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        return(-1);
    }
 
    close(fd);
 
    return(atoi(value_str));
}
 
static int GPIOWrite(int pin, int value)
{
    static const char s_values_str[] = "01";
 
    char path[VALUE_MAX];
    int fd;
 
    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        return(-1);
    }
 
    if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
        fprintf(stderr, "Failed to write value!\n");
        return(-1);
    }
 
    close(fd);
    return(0);
}
 
int test(int pIn){

    int repeat = 10;
 
    if (-1 == GPIOExport(POUT) || -1 == GPIOExport(PIN)){
        return(1);
    }
 
    if (-1 == GPIODirection(POUT, OUT)){
        return(2);
    }
 
    do {
        if (-1 == GPIOWrite(POUT, repeat % 2)){
            return(3);
        }
        usleep(100 * 1000);
    }
    while (repeat--);
 
    if (-1 == GPIOUnexport(POUT)){
        return(4);
    }
 
    return(0);
}

int c_read(char *in){
    int i = atoi(in);
    if( i ==0 ){
        slog(LVL_QUIET,ERROR,"Usage : read PIN");
        return 0;
    }
    return GPIORead(i);
}


int c_test(char *in){
    int i;
    getNumOrPercent(in,0,&i);
    if( i ==0 ){
        slog(LVL_QUIET,ERROR,"Usage : test PIN");
        return 0;
    }
    test(i);
    return i;
}

// the destructor
duk_ret_t js_gpio_dtor(duk_context *ctx)
{
    // The object to delete is passed as first argument instead
    duk_get_prop_string(ctx, 0, "\xff""\xff""deleted");

    bool deleted = duk_to_boolean(ctx, -1);
    duk_pop(ctx);

    // Get the pointer and free
    if (!deleted) {
        duk_get_prop_string(ctx, 0, "\xff""\xff""data");
        void *freePtr = duk_to_pointer(ctx, -1);
        free(freePtr);
        duk_pop(ctx);

        // Mark as deleted
        duk_push_boolean(ctx, true);
        duk_put_prop_string(ctx, 0, "\xff""\xff""deleted");
    }
    return 0;
}

// Constructor of the JS Object
duk_ret_t js_gpio_ctor(duk_context *ctx)
{
    slog(LVL_NOISY,DEBUG, "Creating new object of "PLUGIN_SCOPE);

    myGPIO *mps = malloc(sizeof(myGPIO));
    mps->name = duk_require_string(ctx, 0);
    
    // Push special this binding to the function being constructed
    duk_push_this(ctx);

    // Store the underlying object
    duk_push_pointer(ctx, mps);
    duk_put_prop_string(ctx, -2, "\xff""\xff""data");

    // TODO : - if not existand create a hash_map
    //        - store structure to a hash_map('name');
    //          so that it can be reached from JS and C
    
    // Store a boolean flag to mark the object as deleted because the destructor may be called several times
    duk_push_boolean(ctx, false);
    duk_put_prop_string(ctx, -2, "\xff""\xff""deleted");

    // Store the function destructor
    duk_push_c_function(ctx, js_gpio_dtor, 1);
    duk_set_finalizer(ctx, -2);

    return 0;
}

int myGPIOInfo(char *i){
   if(i) {
      slog(LVL_NOISY,DEBUG,"Info : %s", i);
      // TODO : Get structure from hashmap('name');
      return true;
   } else {
      slog(LVL_NOISY,DEBUG,"Wrong parameters calling "PLUGIN_SCOPE"::info <name>");
      return false;
   }
}

duk_ret_t js_gpio_toString(duk_context *ctx)
{
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, 0, "\xff""\xff""data");
   myGPIO *mps = duk_to_pointer(ctx, -1);
   duk_pop(ctx);
   duk_push_sprintf(ctx, "%s",mps->name);
   return 1;
}

duk_ret_t js_gpio_info(duk_context *ctx)
{
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, 0, "\xff""\xff""data");
   myGPIO *mps = duk_to_pointer(ctx, -1);
   duk_pop(ctx);
   duk_push_sprintf(ctx, "{ name : %s }",mps->name);
   return 1;
}

const duk_function_list_entry myGPIOMethods[] = {
    { "info",           js_gpio_info,      0   },
    { "toString",       js_gpio_toString,  0   },
    { NULL,    NULL,            0 }
};
const function_list_entry c_myGPIOMethods[] = {
    { PLUGIN_SCOPE"::info", WFUNC_SYNC, myGPIOInfo,  0 },
    { PLUGIN_SCOPE"::test", WFUNC_SYNC, c_test,      0 },
    { NULL, 0,   NULL,  0 }
};


char *initPlugin(pluginHandler *_ph){
    slog(LVL_NOISY,FULLDEBUG,"Plugin "PLUGIN_SCOPE" initializing.");
    ph=_ph;
    ctx = ph->ctx;

    wally_put_function_list(c_myGPIOMethods);

    duk_push_c_function(ctx, js_gpio_ctor, 1 );
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, myGPIOMethods);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "GPIO");  /* -> stack: [ ] */

    return PLUGIN_SCOPE;
}

char *cleanupPlugin(void *p){
    slog(LVL_NOISY,DEBUG,"Plugin "PLUGIN_SCOPE" uninitialized");
    return NULL;
}

#endif
