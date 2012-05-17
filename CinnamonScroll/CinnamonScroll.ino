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


int offset = 0;

unsigned long timer_UsbPoll = 0;
unsigned long timer_MarqueeUpdate = 0;

#define MAX_MESSAGE_SIZE 160
#define PREFIX_LENGTH 6 // Prepended to messages e.g. "(327) "
char activeMessage[MAX_MESSAGE_SIZE + PREFIX_LENGTH + 1]; // NUL terminated.

void setup() {

  delay(250);

  acc.powerOn();

   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)

  dmd.selectFont(Arial_Black_16);

  // TODO: Do this better
  activeMessage[0] = ':';
  activeMessage[1] = '\0';

  dmd.drawMarquee(activeMessage, strlen(activeMessage), (32*DISPLAYS_ACROSS)-1, 0);
}


void loop() {

  if (millis() > timer_UsbPoll) {

    if (acc.isConnected()) {
      while(acc.available() > 0) {
        char c = (char) acc.read();
        if ((c != '\n') && (c != '\r')) {
          if (offset < (sizeof(activeMessage)/sizeof(char))) { // TODO: Double check.
            activeMessage[offset++] = c;
            activeMessage[offset] = '\0';
          }
        } else if ((c == '\n')) {
          // TODO: Ensure previous message finished first.
          dmd.drawMarquee(activeMessage, strlen(activeMessage), (32*DISPLAYS_ACROSS)-1, 0);
          offset = 0;
        }
      }
    }

    timer_UsbPoll = millis() + 100;
  }

  if (millis() > timer_MarqueeUpdate) {
    dmd.stepMarquee(-1, 0); // By ignoring the result we scroll indefinitely.
    timer_MarqueeUpdate = millis() + 100;
  }

}
