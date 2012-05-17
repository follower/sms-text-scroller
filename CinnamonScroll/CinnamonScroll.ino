// This is written for my version 1.x USB_Host_Shield library
// with an NAK_LIMIT edit.

// This is currently a bodge of parts of the DMD demo and a USB_Host_Shield demo.

#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>

#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);


AndroidAccessory acc("Circuits@Home, ltd.",
            "USB Host Shield",
            "Arduino Terminal for Android",
            "1.0",
            "http://www.circuitsathome.com",
            "0000000000000001");


void ScanDMD() {
  // Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  // called at the period set in Timer1.initialize();
  dmd.scanDisplayBySPI();
}


int xpos = 1;

unsigned long timer = 0;

void setup() {

  delay(250);

  acc.powerOn();

   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)

  dmd.selectFont(Arial_Black_16);

  dmd.drawChar(  0,  3, ':', GRAPHICS_NORMAL );
}


void loop() {

  if (millis() > timer) {

    if (acc.isConnected()) {
      while(acc.available() > 0) {
          dmd.drawChar(  (xpos++) * 10,  3, (char) acc.read(), GRAPHICS_NORMAL );
        }
    }

    timer = millis() + 100;
  }

}
