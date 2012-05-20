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

#define USE_SMS // By which I mean the current "BackgroundUsbService" Android project.

#if !defined(USE_SMS)
AndroidAccessory acc("Circuits@Home, ltd.",
            "USB Host Shield",
            "Arduino Terminal for Android",
            "1.0",
            "http://www.circuitsathome.com",
            "0000000000000001");
#else
AndroidAccessory acc("rancidbacon.com",
		     "BackgroundUsbDemo",
		     "Background USB Demo accessory",
		     "0.1",
		     "http://rancidbacon.com/",
		     "0000000000000001");
#endif



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
#define DEFAULT_MESSAGE_STRING ":: Cinnamon Scroll 0.1 ::"
char activeMessage[MAX_MESSAGE_SIZE + PREFIX_LENGTH + 1] = DEFAULT_MESSAGE_STRING; // NUL terminated.

boolean newMessageAvailable = true;
boolean scrollCycleComplete = true;

boolean readingSms = false;

#define NEW_CODE

#ifdef NEW_CODE
#define NUM_STORED_MSGS 5
char storedMessages[NUM_STORED_MSGS][MAX_MESSAGE_SIZE + PREFIX_LENGTH + 1] = {"Txt us yr msg: XXX XXX XXXX",".","..","...","...."};
unsigned int currentMsgStoreIndex = 1;
unsigned int nextMsgToShow = 0;
#endif


void setup() {

  delay(250);

  acc.powerOn();

   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 2500 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)

  dmd.selectFont(Arial_Black_16);

  Serial.begin(9600);
  Serial.println("start");
}


void loop() {

  if (millis() > timer_UsbPoll) {

    if (acc.isConnected()) {
#ifndef NEW_CODE
      if (!newMessageAvailable) { // Wait for current message to be pushed to display.
#endif
        while(acc.available() > 0) {
          char c = (char) acc.read();
          
          switch (c) {
           
#ifdef USE_SMS            
            case 0x01:
              if (readingSms) {
                // End of message
#ifdef NEW_CODE
                currentMsgStoreIndex++;
                if (currentMsgStoreIndex >= NUM_STORED_MSGS) {
                  currentMsgStoreIndex = 1; // Skip 0 as it's permanent.
                }
#else
                newMessageAvailable = true;
#endif

                offset = 0;
                
                readingSms = false;
              }
              break;
              
            case 0x02:
              readingSms = true;
              offset = 0;
              break;

            case '\n':
            case '\r':
              // Ignore 
              break;
#else                      
            case '\n':
              newMessageAvailable = true;
              offset = 0;
              break;

            case '\r':
              // Ignore 
              break;
#endif              
              
            default:
#ifdef USE_SMS            
              if (readingSms) {
#endif                
#ifdef NEW_CODE
                Serial.print(c);
                if (offset < (sizeof(activeMessage)/sizeof(char))) { // TODO: Double check.
                  storedMessages[currentMsgStoreIndex][offset++] = c;
                  storedMessages[currentMsgStoreIndex][offset] = '\0';
                }

#else
                if (offset < (sizeof(activeMessage)/sizeof(char))) { // TODO: Double check.
                  activeMessage[offset++] = c;
                  activeMessage[offset] = '\0';
                }
#endif
#ifdef USE_SMS                            
              }
#endif              
          }
        }
#ifndef NEW_CODE
      }
#endif
    }

    timer_UsbPoll = millis() + 100;
  }

  if (millis() > timer_MarqueeUpdate) {

    scrollCycleComplete = dmd.stepMarquee(-1, 0); // By ignoring the result we scroll indefinitely.

#ifdef NEW_CODE
    if (scrollCycleComplete) {
      dmd.drawMarquee(storedMessages[nextMsgToShow], strlen(storedMessages[nextMsgToShow]), (32*DISPLAYS_ACROSS)-1, 0);

      nextMsgToShow++;
      if (nextMsgToShow > NUM_STORED_MSGS) {
        nextMsgToShow = 0;
      }
    }
#else
    // TODO: Don't have full blank screen before new message?
    if (scrollCycleComplete && newMessageAvailable) {
      dmd.drawMarquee(activeMessage, strlen(activeMessage), (32*DISPLAYS_ACROSS)-1, 0);
      newMessageAvailable = false;
    }
#endif

    timer_MarqueeUpdate = millis() + 30; // TODO: Make scroll speed adjustable.
  }

}
