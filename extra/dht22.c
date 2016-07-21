// For Pi 1 add -DPI1

//#ifdef PI1
//#define BCM2708_PERI_BASE        0x20000000
//#else
#define BCM2708_PERI_BASE        0x3F000000
//#endif

#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
//#define MAXTIMINGS 100
#define MAXTIMINGS 100

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <bcm2835.h>
#include <unistd.h>

int readDHT(int pin);

int main(int argc, char **argv){
  if(getuid() != 0){
    printf("You must be root to execute this program!\n");
    return -1;
  }
  if (!bcm2835_init()) return 1;

  if (argc != 2) {
    printf("usage: %s GPIOpin#\n", argv[0]);
    printf("example: %s 4 - Read from a DHT22 connected to GPIO #4\n", argv[0]);
    return -1;
  }

  int dhtpin = atoi(argv[1]);

  if (dhtpin <= 0){
    printf("Please select a valid GPIO pin #\n");
    return -1;
  }

  printf("Using pin #%d\n", dhtpin);
  readDHT(dhtpin);
  return 0;

}

int readDHT(int pin){
  int bits[250], data[100], bitidx = 0;
  int counter = 0;
  int laststate = HIGH;
  int j=0;

  // Set GPIO pin to output
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);

  bcm2835_gpio_write(pin, HIGH);
  usleep(500000);  // 500 ms
  bcm2835_gpio_write(pin, LOW);
  usleep(20000);

  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // wait for pin to drop?
  while (bcm2835_gpio_lev(pin) == 1){
    usleep(1);
  }

  // read data!
  for (int i=0; i< MAXTIMINGS; i++){
    counter = 0;
    while (bcm2835_gpio_lev(pin) == laststate){
      counter++;
      usleep(1);        // overclocking might change this?
      if (counter == 1000) break;
    }
    laststate = bcm2835_gpio_lev(pin);
    if (counter == 1000) break;
    bits[bitidx++] = counter;

    // ignore first 3 transitions
    if ((i>3) && (i%2 == 0)){
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      if (counter > 200) data[j/8] |= 1;
      j++;
    }
  }

  /* 8b = 8 bits - RH = relative humidity */
  /* DATA = 8b of high RH data + 8b of low RH data + 8b of high temperature data + 8b of low temperature data + 8b CRC */
  printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);

  // check we read 40 bits (8bit x 5) + verify checksum in the last byte
  if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))){
    float temp, hum;
    hum = data[0] * 256 + data[1];
    hum /= 10.0;
    temp = (data[2] & 0x7F)* 256 + data[3];
    temp /= 10.0;

    if (data[2] & 0x80){
      temp *= -1; //questa si avvera con valori di data[2] compresi fra 128 (0x80) e 255 (0xFF). Serve per temp negativa;
    }
    printf("Temp =  %.1f *C, Hum = %.1f \%\n", temp, hum);
    return 0;
  }
  else{
    printf("Data not good, skip\n");
    return -1;
  }
  return 0;
}
