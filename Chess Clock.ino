/*
Project: Chess clock LCD display with esp32
Creator: Blake Walker
Description:
A chess clock with 9 different pre-programmed time controls.
Includes pausing functionality, special responses to running out of time, lights to indictate
whos turn it is, and easy resetting.
Notes:
- To reset, hold down the "select time control" button.
- To pause on your turn, press the button for the other player.
- pins 2 and 3 are the only interrupt pins, so we are only using interrupts for
the white and black move buttons.
*/

#include <LiquidCrystal_I2C.h>

void printTime(int timeWhiteSec, int timeWhiteMin, int timeWhiteHr, int timeBlackSec, int timeBlackMin, int timeBlackHr);

LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

const int timeControlButtonPin = 25;
const int whiteMoveButtonPin = 26;
const int blackMoveButtonPin = 27;
const int whiteLEDButtonPin = 4;
const int blackLEDButtonPin = 5;

const int numRows = 2;
const int numCols = 16;

unsigned long previousMillis = 0; // will store last time LED was updated
unsigned long currentMillis = millis();
const long interval = 1000; // interval at which to blink (milliseconds), should be 1000

int timeWhiteSec = 0;
int timeWhiteMin = 0;
int timeWhiteHr = 0;

int timeBlackSec = 0;
int timeBlackMin = 0;
int timeBlackHr = 0;

int timeControlButtonState = 0;
volatile bool whiteMoveButtonPressed = false;
volatile bool blackMoveButtonPressed = false;

bool whiteToMove = 0;

int programState = 0; // 0 for setup, 1 for playing
int timeControl = 9; // Variable (0-9) to store the game mode decided in setup, starts with 9 so 
const int numTimeControls = 10;

void setup() {
  Serial.begin(9600);
  //using external pull-up resistors in Tinkercad because the internal ones dont work
  pinMode(timeControlButtonPin, INPUT_PULLUP); // Enable internal pull-up resistor 
  pinMode(whiteMoveButtonPin, INPUT_PULLUP);
  pinMode(blackMoveButtonPin, INPUT_PULLUP);
  pinMode(whiteLEDButtonPin, OUTPUT);
  pinMode(blackLEDButtonPin, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  
  // Attach interrupts for button presses
  attachInterrupt(digitalPinToInterrupt(whiteMoveButtonPin), whiteMoveButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(blackMoveButtonPin), blackMoveButtonISR, FALLING);
  //attachInterrupt(digitalPinToInterrupt(timeControlButtonPin), timeControlButtonISR, FALLING);
}

void loop() {
  timeControlButtonState = digitalRead(timeControlButtonPin);
  switch(programState) {
    // Selecting time control
    case 0:
      if (timeControlButtonState == LOW) { // Check if button press was detected
        delay(200);
        timeControl = ++timeControl % numTimeControls;
        lcd.clear();

        switch (timeControl) {
          case 0: lcd.print("Rapid 15|10"); timeWhiteMin = timeBlackMin = 15; break;
          case 1: lcd.print("Rapid 10|0"); timeWhiteMin = timeBlackMin = 10; break;
          case 2: lcd.print("Blitz 5|5"); timeWhiteMin = timeBlackMin = 5; break;
          case 3: lcd.print("Blitz 5|0"); timeWhiteMin = timeBlackMin = 5; break;
          case 4: lcd.print("Blitz 3|2"); timeWhiteMin = timeBlackMin = 3; break;
          case 5: lcd.print("Blitz 3|0"); timeWhiteMin = timeBlackMin = 3; break;
          case 6: lcd.print("Bullet 2|1"); timeWhiteMin = timeBlackMin = 2; break;
          case 7: lcd.print("Bullet 1|0"); timeWhiteMin = timeBlackMin = 1; break;
          case 8: lcd.print("Rapid 30|0"); timeWhiteMin = timeBlackMin = 30; break;
        }
      }

      if (blackMoveButtonPressed) {
        blackMoveButtonPressed = false; // Reset flag
        programState = 1;
        whiteToMove = 1;
      }
    break;

  // normal clock mode
    case 1: 
      currentMillis = millis();
      timeControlButtonState = digitalRead(timeControlButtonPin);
    
      // reset all values and return to default state
      if (timeControlButtonState == LOW){ // reusing this button for the reset
        reset();
        delay(500);
        return;
      }
    
      // pause
      if ((whiteToMove && blackMoveButtonPressed) || (!whiteToMove && whiteMoveButtonPressed)){
        whiteMoveButtonPressed = blackMoveButtonPressed = false;
        programState = 4;
        return;
      }
      // Check for moves based on button presses
      if (whiteToMove && whiteMoveButtonPressed) {
        whiteMoveButtonPressed = false; // Reset flag
        whiteToMove = 0; // white hit the button

        //Handle increments
        //for 15|10
        if (timeControl == 0){
          if (timeWhiteSec < 50)
            timeWhiteSec += 10;
          else{
            timeWhiteSec = (timeWhiteSec + 10)%60;
            timeWhiteMin++;
          } 
        }
        //for 5|5
        if (timeControl == 2){
          if (timeWhiteSec < 55)
            timeWhiteSec += 5;
          else{
            timeWhiteSec = (timeWhiteSec + 5)%60;
            timeWhiteMin++;
          } 
        }
        //for 3|2
        if (timeControl == 4){
          if (timeWhiteSec < 58)
            timeWhiteSec += 2;
          else{
            timeWhiteSec = (timeWhiteSec + 2)%60;
            timeWhiteMin++;
          } 
        }
        //for 2|1
        if (timeControl == 6){
          if (timeWhiteSec < 59)
            timeWhiteSec++;
          else{
            timeWhiteSec = (timeWhiteSec++)%60;
            timeWhiteMin++;
          } 
        }

      }
    
      if (!whiteToMove && blackMoveButtonPressed) {
        blackMoveButtonPressed = false; // Reset flag
        whiteToMove = 1; // black hit the button

        //Handle increments
        //for 15|10
        if (timeControl == 0){
          if (timeBlackSec < 50)
            timeBlackSec += 10;
          else{
            timeBlackSec = (timeBlackSec + 10)%60;
            timeBlackMin++;
          } 
        }
        //for 5|5
        if (timeControl == 2){
          if (timeBlackSec < 55)
            timeBlackSec += 5;
          else{
            timeBlackSec = (timeBlackSec + 5)%60;
            timeBlackMin++;
          } 
        }
        //for 3|2
        if (timeControl == 4){
          if (timeBlackSec < 58)
            timeBlackSec += 2;
          else{
            timeBlackSec = (timeBlackSec + 2)%60;
            timeBlackMin++;
          } 
        }
        //for 2|1
        if (timeControl == 6){
          if (timeBlackSec < 59)
            timeBlackSec++;
          else{
            timeBlackSec = (timeBlackSec++)%60;
            timeBlackMin++;
          } 
        }

      }

      if (currentMillis - previousMillis >= interval) {
        // Update time (white)
        if (whiteToMove) {
          if (timeWhiteSec == 0 && timeWhiteMin == 0 && timeWhiteHr == 0){
            Serial.println("black won on time");
            programState = 2;
            lcd.clear();
            return;
          }
          if (timeWhiteSec > 0)
            timeWhiteSec--;
          else {
            if (timeWhiteMin > 0) {
              timeWhiteMin--;
              timeWhiteSec = 59;
            }
            else if (timeWhiteHr > 0) {
              timeWhiteHr--;
              timeWhiteMin = 59;
              timeWhiteSec = 59;
            }
          }
        }

        // Update time (black)
        else {
          if (timeBlackSec == 0 && timeBlackMin == 0 && timeBlackHr == 0){
            Serial.println("white won on time");
            programState = 3;
            lcd.clear();
            return;
          }
          if (timeBlackSec > 0)
            timeBlackSec--;
          else {
            if (timeBlackMin > 0) {
              timeBlackMin--;
              timeBlackSec = 59;
            } else if (timeBlackHr > 0) {
              timeBlackHr--;
              timeBlackMin = 59;
              timeBlackSec = 59;
            }
          }
        }
        printTime(timeWhiteSec, timeWhiteMin, timeWhiteHr, timeBlackSec, timeBlackMin, timeBlackHr);
        previousMillis = currentMillis;
      }
    break;
    
  // white ran out of time
  case 2:
  	lcd.print("Black won");
    lcd.setCursor(0,1);
    lcd.print("on time");
    // flash the LEDs
    digitalWrite(whiteLEDButtonPin, HIGH);
    delay(250);
    digitalWrite(whiteLEDButtonPin, LOW);
    delay(250);
  	digitalWrite(blackLEDButtonPin, HIGH);
    delay(250);
    digitalWrite(whiteLEDButtonPin, LOW);
    delay(250);
    lcd.clear();
    // reset all values and return to default state
    if (timeControlButtonState == LOW){ // reusing this button for the reset
      reset();
      delay(500);
      return;
    }
    break;
    
  // black ran out of time
  case 3:
  	lcd.print("White won");
    lcd.setCursor(0,1);
    lcd.print("on time");
    // flash the LEDs
    digitalWrite(whiteLEDButtonPin, HIGH);
    delay(250);
    digitalWrite(whiteLEDButtonPin, LOW);
    delay(250);
  	digitalWrite(blackLEDButtonPin, HIGH);
    delay(250);
    digitalWrite(whiteLEDButtonPin, LOW);
    delay(250);
    lcd.clear();
    // reset all values and return to default state
    if (timeControlButtonState == LOW){ // reusing this button for the reset
      reset();
      delay(500);
      return;
    }
    break;
    
  //paused:
  case 4:
    delay(50);
    if ((whiteToMove && blackMoveButtonPressed) || (!whiteToMove && whiteMoveButtonPressed)){
      whiteMoveButtonPressed = blackMoveButtonPressed = false;
      programState = 1;
      return;
    }
    
  }//end switch
}//end loop

void printTime(int timeWhiteSec, int timeWhiteMin, int timeWhiteHr, int timeBlackSec, int timeBlackMin, int timeBlackHr) {
  lcd.clear();
  // format: x:xx:xx
  char topLine[18];
  char timeWhite[8]; // 7 characters + 1 for the null-terminator
  char timeBlack[8]; // 7 characters + 1 for the null-terminator
  timeWhite[1] = timeBlack[1] = ':';
  timeWhite[4] = timeBlack[4] = ':';
  timeWhite[7] = timeBlack[7] = '\0'; // Null-terminate the string

  // Hours
  timeWhite[0] = '0' + (timeWhiteHr % 10);
  timeBlack[0] = '0' + (timeBlackHr % 10);

  // Minutes
  timeWhite[2] = '0' + (timeWhiteMin / 10);
  timeWhite[3] = '0' + (timeWhiteMin % 10);
  timeBlack[2] = '0' + (timeBlackMin / 10);
  timeBlack[3] = '0' + (timeBlackMin % 10);

  // Seconds
  timeWhite[5] = '0' + (timeWhiteSec / 10);
  timeWhite[6] = '0' + (timeWhiteSec % 10);
  timeBlack[5] = '0' + (timeBlackSec / 10);
  timeBlack[6] = '0' + (timeBlackSec % 10);
  
  //combine strings and print
  snprintf(topLine, sizeof(topLine), "%s||%s", timeWhite, timeBlack);
  lcd.print(topLine);
  lcd.setCursor(0,1);
  lcd.print("White  ||  Black");
}

void whiteMoveButtonISR() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) { // 200 ms debounce delay
    whiteMoveButtonPressed = true;
    digitalWrite(blackLEDButtonPin, HIGH);
    digitalWrite(whiteLEDButtonPin, LOW);
    lastInterruptTime = interruptTime;
  }
}

void blackMoveButtonISR() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) {
    blackMoveButtonPressed = true;
    digitalWrite(whiteLEDButtonPin, HIGH);
    digitalWrite(blackLEDButtonPin, LOW);
    lastInterruptTime = interruptTime;
  }
}

void reset() {
  programState = 0;
  previousMillis = 0; // will store last time LED was updated
  timeWhiteSec = 0;
  timeWhiteMin = 0;
  timeWhiteHr = 0;
  timeBlackSec = 0;
  timeBlackMin = 0;
  timeBlackHr = 0;
  whiteToMove = 0;
  timeControl = 9;
}
