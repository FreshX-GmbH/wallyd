
/* blink.c
 *  *
 *   * Raspberry Pi GPIO example using sysfs interface.
 *    * Guillermo A. Amaral B. <g@maral.me>
 *     *
 *      */

#include "../lib/plugins.h"
#include "../lib/util.h"
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
#define IN  0
#define OUT 1
#define PLUGIN_SCOPE "gpio"

#define LOW  0
#define HIGH 1
 
#define PIN  24 /* P1-18 */
#define POUT 7  /* P1-07 */

#define MIN_INPUT_PIN 1
#define MAX_INPUT_PIN 7
#define MIN_OUTPUT 1
#define MAX_OUTPUT 32
 
#define BUFFER_MAX 3
#define DIRECTION_MAX 35
#define VALUE_MAX 30
