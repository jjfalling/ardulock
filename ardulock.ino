//****************************************************************************
//*   ArduLock                                                               *
//*   ArduLock will take input from a 4x4 digit pad, use a servo or relay to *
//*    open a lock, and will give user feedback with a rgb led. It stores    *
//*    the pin in the eeprom to allow the user to change it without          *
//*    reflashing.                                                           *
//*                                                                          *
//*   Copyright (C) 2013 by Jeremy Falling except where noted.               *
//*                                                                          *
//*   This program is free software: you can redistribute it and/or modify   *
//*   it under the terms of the GNU General Public License as published by   *
//*   the Free Software Foundation, either version 3 of the License, or      *
//*   (at your option) any later version.                                    *
//*                                                                          *
//*   This program is distributed in the hope that it will be useful,        *
//*   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
//*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
//*   GNU General Public License for more details.                           *
//*                                                                          *
//*   You should have received a copy of the GNU General Public License      *
//*   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
//****************************************************************************
// devices used: 4 X 4 Keypad, common anode rgb led, servo

#include <Keypad.h>
#include <Servo.h> 
#include <EEPROM.h>

//The following is meant for a common anode led.
//if yours is common cathode swap the high and low
#define ledOn LOW
#define ledOff HIGH

//define pins going to the led
#define redPin 13
#define greenPin 12
#define bluePin 10

//delay between the lock opening and re-locking
int lockDelay = 5000;

//which pin is the relay or servo on?
int controlPin = 11;

//define the locktype:
// deadbolt - use a servo attached to the deadbolt
// strikeplate - use a no relay on the power to a electric strike plate 
String lockType = "deadbolt";

//what values do you need for your servo to open or close the lock? (ignore these if not using a deadbolt)
int servoClose = 0;
int servoOpen = 180;

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns

// Define the Keymap
char keys[ROWS][COLS] = {
  {
    '1','2','3','A'          }
  ,
  {
    '4','5','6','B'          }
  ,
  {
    '7','8','9','C'          }
  ,
  {
    '*','0','#','D'          }
};

byte rowPins[ROWS] = { 
  8, 7, 6, 9 };
byte colPins[COLS] = { 
  5, 4, 3, 2 };


//end user settings
//****************************************************************************


String pinCode;

String code = "";

Servo myservo;  // create servo object to control a servo 
// a maximum of eight servo objects can be created 


// Create the Keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//Your rgb led type
//  0 - common anode
//  1 - common cathode


void setup(){

  if (lockType == "deadbolt"){
    myservo.attach(controlPin); 
    myservo.write(0); 
  }
  else if (lockType == "strikeplate"){
    pinMode(controlPin, OUTPUT);
  }

  Serial.begin(9600);

  setLocked(true);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  digitalWrite(redPin, ledOff);
  digitalWrite(greenPin, ledOff);
  digitalWrite(bluePin, ledOff);

  Serial.println("starting...");

  digitalWrite(redPin, ledOn);    
  delay(300); 
  digitalWrite(redPin, ledOff);

  digitalWrite(greenPin, ledOn);    
  delay(300); 
  digitalWrite(greenPin, ledOff);

  digitalWrite(bluePin, ledOn);    
  delay(300); 
  digitalWrite(bluePin, ledOff);

  digitalWrite(redPin, ledOn);
  digitalWrite(greenPin, ledOn);
  digitalWrite(bluePin, ledOn);
  delay(300);  

  digitalWrite(redPin, ledOn);
  digitalWrite(greenPin, ledOff);
  digitalWrite(bluePin, ledOff);

  //eprom 0-9 is the first code. each position only stores one number 0-9. 255 is empty 
  byte currentVal;

  boolean valid = true;
  int address = 1;

  currentVal = EEPROM.read(0);
  pinCode = String(currentVal);

  while(valid = true) {

    currentVal = EEPROM.read(address);

    if (currentVal != 255){
      pinCode = pinCode + String(currentVal);
    }
    else {
      valid = false;
      break;
    }
    address++;

    if (address > 9){
      valid = false;
      break;

    }
  }
  Serial.println("finished ");
  //uncomment to print code to serial
  //Serial.println(pinCode);


  //assume 10 0's means the eprom is empty so use a default pin
  if (pinCode == "0000000000") {

    pinCode = "12345";
    Serial.print("invalid code, using default: ");
    Serial.println(pinCode);
  }
}


void loop()
{
  char key = keypad.getKey();

  if (key != NO_KEY){


    digitalWrite(redPin, ledOff);    
    delay(50); 
    digitalWrite(redPin, ledOn);




    //use # to clear
    if (key == '#') {
      code = "";
      setLocked(true);
      Serial.println("cleared");

      digitalWrite(redPin, ledOff);    
      delay(100); 
      digitalWrite(greenPin, ledOn);
      delay(100);
      digitalWrite(greenPin, ledOff);    
      delay(100); 
      digitalWrite(redPin, ledOn);
    }

    //use * as enter
    else if (key == '*') {
      //if code matches, open lock
      if (code == pinCode)
      {
        setLocked(false);
        code = "";
        Serial.println("correct code");
        setLocked(false);
        delay(lockDelay);
        setLocked(true);

      }
      //if pin does not match give error
      else{

        code = "";

        Serial.println("invalid code");

        blinkLedError();
      }
    }

    //use A to change pin
    else if (key == 'A') {

      //if pin is valid allow pin change
      if (code == pinCode)
      {

        code = "";
        Serial.println("Change code");
        digitalWrite(redPin, ledOff);
        digitalWrite(bluePin, ledOn);

        boolean cont = true;
        while (cont = true){

          char key = keypad.getKey();
          if (key != NO_KEY){

            //flash blue led with each key press
            digitalWrite(bluePin, ledOff);    
            delay(50); 
            digitalWrite(bluePin, ledOn);

            //stop with *
            if (key == '*') {
              cont = false;
              
              //count length of string, set that many addresses in the eprom
              // and if <10, blank remain with 255. else dont. 
              int pinLength = code.length();
              if (pinLength > 10){
                Serial.println("code too long");
                code = "";
                digitalWrite(bluePin, ledOff);  
                blinkLedError();
                break;
              }
              //check if pin lenght is less then 1 digit
              else if(pinLength < 1){
                Serial.println("code too short");
                code = "";
                digitalWrite(bluePin, ledOff);  
                blinkLedError();
                break;
              }

              //pin is of proper length, continue
              else {
                //flash green led
                digitalWrite(bluePin, ledOff);
                digitalWrite(greenPin, ledOn);
                delay(300);
                digitalWrite(greenPin, ledOff);
                digitalWrite(bluePin, ledOn);

                //verify pin again to prevent mis-typing
                Serial.println("verify pin");
                String code2 = "";
                boolean cont2 = true;

                while (cont2 = true){

                  char key = keypad.getKey();
                  if (key != NO_KEY){


                    digitalWrite(bluePin, ledOff);    
                    delay(50); 
                    digitalWrite(bluePin, ledOn);


                    //stop with *
                    if (key == '*') {
                      cont2 = false;

                      Serial.println(code);
                      Serial.println(code2);

                      //if the pins match, change the current one to the new one
                      if (code == code2) {


                        Serial.println("new code accepted ");
                        //uncomment to print new code to serial. 
                        //Serial.println(code);

                        pinCode = code;

                        int i=0;
                        while (i < pinLength){
                          //get the ascii value of the curent digit
                          byte currentDigit = code.charAt(i);

                          //covert from ascii to a normal digit. Given numbers are in a range, just subtract 48
                          currentDigit = currentDigit - 48;

                          EEPROM.write(i, currentDigit);

                          i++;
                        }
                        digitalWrite(bluePin, ledOff);
                        delay(100);
                        digitalWrite(bluePin, ledOn);
                        delay(100);
                        digitalWrite(bluePin, ledOff);
                        delay(100);
                        digitalWrite(bluePin, ledOn);
                        delay(100);
                        digitalWrite(bluePin, ledOff);
                        delay(100);
                        digitalWrite(bluePin, ledOn);
                        delay(100);
                        digitalWrite(bluePin, ledOff);
                        digitalWrite(redPin, ledOn);

                        code = "";

                        break;

                      }

                      //first and second pin do not match 
                      else {
                        Serial.println("codes do not match");		
                        digitalWrite(bluePin, ledOff);
                        blinkLedError();
                        code = "";

                        break;					

                      }


                    }

                    code2 = code2 + String(key);  

                  }


                } 

                //put null chars for the remaining addresses
                if (pinLength < 10){
                  int tempCount = 10 - pinLength;


                  int i=0 ;
                  while (i < tempCount){
                    int foo = pinLength + i;
                    EEPROM.write(foo, 255);

                    i++;
                  }  

                }


                break;
              }
            }

            //reject bad keys
            else if (key == '#' || key == 'A' ||key == 'B' ||key == 'C' || key == 'D' ) {
              Serial.println("invalid code");

              digitalWrite(bluePin, ledOff);
              blinkLedError();
              cont = false;
              code = "";
              break;
            }

            else {
              code = code + String(key);
            }


          }

        }
      }
      else{

        code = "";

        Serial.println("invalid code");           
        blinkLedError();
      }
    }

    //if no other case matches, store value
    else{
      code = code + String(key);
    }


    delay(50);
  }
}

void setLocked(int locked)
{
  if (locked) {

    if (lockType == "deadbolt"){
      myservo.write(servoClose);
    }
    else if (lockType == "strikeplate"){
      digitalWrite(controlPin, ledOn);
    }

    digitalWrite(redPin, ledOn);
    digitalWrite(greenPin, ledOff);

  }
  else {

    if (lockType == "deadbolt"){
      myservo.write(servoOpen);
    }
    else if (lockType == "strikeplate"){
      digitalWrite(controlPin, ledOff);
    }  

    digitalWrite(redPin, ledOff);
    digitalWrite(greenPin, ledOn); 

  }
}

//function to blink the led red
void blinkLedError(void)
{
  digitalWrite(redPin, ledOff);    
  delay(100); 
  digitalWrite(redPin, ledOn);    
  delay(100); 
  digitalWrite(redPin, ledOff);    
  delay(100); 
  digitalWrite(redPin, ledOn); 
  delay(100); 
  digitalWrite(redPin, ledOff);    
  delay(100); 
  digitalWrite(redPin, ledOn);
  delay(100); 
  digitalWrite(redPin, ledOff);    
  delay(100); 
  digitalWrite(redPin, ledOn);
  delay(100); 
  digitalWrite(redPin, ledOff);    
  delay(100); 
  digitalWrite(redPin, ledOn);

}



