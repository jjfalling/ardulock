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

int position = 0;

String pinCode; 
int redPin = 13;
int greenPin = 12;
int bluePin = 10;

//which pin is the relay or servo on?
int controlPin = 11;

//what values do you need for your servo to open or close the lock? (ignore these if not using a deadbolt)
int servoClose = 0;
int servoOpen = 180;

String code = "";

//define the locktype:
// deadbolt - use a servo
// strikeplate - use a no realy
String lockType = "deadbolt";

//delay between the lock opening and re-locking
int lockDelay = 5000;

Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 


const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns

// Define the Keymap
char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};

byte rowPins[ROWS] = { 8, 7, 6, 9 };
byte colPins[COLS] = { 5, 4, 3, 2 };

// Create the Keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

  

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
  
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
  
  Serial.println("starting...");
  
  digitalWrite(redPin, LOW);    
  delay(300); 
  digitalWrite(redPin, HIGH);
  
  digitalWrite(greenPin, LOW);    
  delay(300); 
  digitalWrite(greenPin, HIGH);
  
  digitalWrite(bluePin, LOW);    
  delay(300); 
  digitalWrite(bluePin, HIGH);
  
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
  delay(300);  

  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
    
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
      valid = false;\
      break;

    }
  }
    Serial.println("finished ");
    //uncomment to print code to serial
//  Serial.println(pinCode);
  
  
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

      
      digitalWrite(redPin, HIGH);    
      delay(50); 
      digitalWrite(redPin, LOW);
      

  
    
      //use # to clear
      if (key == '#') {
        code = "";
        setLocked(true);
            Serial.println("cleared");
            
            digitalWrite(redPin, HIGH);    
            delay(100); 
            digitalWrite(greenPin, LOW);
            delay(100);
            digitalWrite(greenPin, HIGH);    
            delay(100); 
            digitalWrite(redPin, LOW);
      }
      
      //use * as enter
      else if (key == '*') {
          if (code == pinCode)
          {
             setLocked(false);
             code = "";
             Serial.println("correct code");
             setLocked(false);
             delay(lockDelay);
             setLocked(true);

          }
          else{
            
            code = "";
            
            Serial.println("invalid code");
            
            blinkLedError();
          }
      }
      
      //use A to change pin
      else if (key == 'A') {
    
          if (code == pinCode)
          {

             code = "";
             Serial.println("Change code");
             digitalWrite(redPin, HIGH);
             digitalWrite(bluePin, LOW);
           
             boolean cont = true;
             while (cont = true){
               
                char key = keypad.getKey();
                if (key != NO_KEY){
            
                  
                  digitalWrite(bluePin, HIGH);    
                  delay(50); 
                  digitalWrite(bluePin, LOW);
                  
                  //stop with *
                  if (key == '*') {
                      cont = false;
                     
                     
                      //count length of string, set that many addresses in the eprom
                      // and if <10, blank remain with 255. else dont. 
                      
                      int codeLength = code.length();
                      if (codeLength > 10){
                        Serial.println("code too long");
                        code = "";
                        digitalWrite(bluePin, HIGH);  
                        blinkLedError();
                        break;
                      }
                      else {
                        Serial.print("new code accepted ");
                        //uncomment to print new code to serial. 
                        //Serial.println(code);
                        
                        pinCode = code;
                        
                        int i=0;
                        while (i < codeLength){
                           //get the ascii value of the curent digit
                           byte currentDigit = code.charAt(i);
                           
                           //covert from ascii to a normal digit. Given numbers are in a range, just subtract 48
                           currentDigit = currentDigit - 48;
                           
                            EEPROM.write(i, currentDigit);
      
                           i++;
                        } 
                        
                        //put null chars for the remaining addresses
                        if (codeLength < 10){
                         int tempCount = 10 - codeLength;

                        
                         int i=0 ;
                         while (i < tempCount){
                           int foo = codeLength + i;
                           EEPROM.write(foo, 255);
                            
                           i++;
                          }  
                          
                        }
                        
                         digitalWrite(bluePin, HIGH);
                         delay(100);
                         digitalWrite(bluePin, LOW);
                         delay(100);
                         digitalWrite(bluePin, HIGH);
                         delay(100);
                         digitalWrite(bluePin, LOW);
                         delay(100);
                         digitalWrite(bluePin, HIGH);
                         delay(100);
                         digitalWrite(bluePin, LOW);
                         delay(100);
                         digitalWrite(bluePin, HIGH);
                         digitalWrite(redPin, LOW);
                         
                         code = "";
                         
                        break;
                      }
                    }
                    
                  //reject bad keys
                  else if (key == '#' || key == 'A' ||key == 'B' ||key == 'C' || key == 'D' ) {
                      Serial.println("invalid code");
          
                      digitalWrite(bluePin, HIGH);
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
	  digitalWrite(controlPin, LOW);
	}

    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    
  }
  else {

	  if (lockType == "deadbolt"){
		myservo.write(servoOpen);
	  }
	  else if (lockType == "strikeplate"){
		  digitalWrite(controlPin, HIGH);
	  }  

    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW); 
    
  }
}

//function to blink the led red
void blinkLedError(void)
{
    digitalWrite(redPin, HIGH);    
    delay(100); 
    digitalWrite(redPin, LOW);    
    delay(100); 
    digitalWrite(redPin, HIGH);    
    delay(100); 
    digitalWrite(redPin, LOW); 
    delay(100); 
    digitalWrite(redPin, HIGH);    
    delay(100); 
    digitalWrite(redPin, LOW);
    delay(100); 
    digitalWrite(redPin, HIGH);    
    delay(100); 
    digitalWrite(redPin, LOW);
    delay(100); 
    digitalWrite(redPin, HIGH);    
    delay(100); 
    digitalWrite(redPin, LOW);
  
}

