#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
/*The function is not implement yet, but it would be really
  useful to implement in case of power failure. The device should
  start right where it was or close to.
  To do so, use the EEPROM library and save data
*/
#include <EEPROM.h>

// Set max min value for diffenret
#define MINREPBYMIN 25
#define MAXREPBYMIN 40

//The logic is inverted for the button
#define PRESSED_BUTTONVALUE 0
#define RELEASED_BUTTONVALUE 1

// Set the pinout
#define DOWN_BUTTON_PIN 5
#define UP_BUTTON_PIN 6
//The control button shall be a switch
#define CONTROL_BUTTON_PIN 7
#define VALVE_PIN 8

// Set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Variable use in the main
int breathCounterValue = MINREPBYMIN;
int nextBreath;

int DownButtonCurrentState = 1;
int DownButtonChangeStateCounter = 0;
int DownButtonToggleState = 0;

int UpButtonCurrentState = 1;
int UpButtonChangeStateCounter = 0;
int UpButtonToggleState = 0;

int controlButtonCurrentState = 1;
int controlButtonChangeStateCounter = 0;
int controlButtonToggleState = 0;

// Function
// Debounce button function
void debounceButtonCounter(int button_pin, int *p_currentState, int *p_changeStateCounter);
void changeRepByMin();
void cycleBreather();

void setup() {
  // initialize the LCD
  lcd.begin();

  // initialize the two buttons
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CONTROL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, LOW);

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("--Mission 19--");
  // Col, Row
  lcd.setCursor(0, 1);
  lcd.print("Breathing by minute:");
  lcd.setCursor(0, 2);
  lcd.print(MINREPBYMIN);
}

// Run indefinitely
void loop() {  
  // Read the value of the buttons
  debounceButtonCounter(DOWN_BUTTON_PIN, &DownButtonCurrentState, &DownButtonChangeStateCounter);
  debounceButtonCounter(UP_BUTTON_PIN, &UpButtonCurrentState, &UpButtonChangeStateCounter);
  debounceButtonCounter(CONTROL_BUTTON_PIN, &ControlButtonCurrentState, &ControlButtonChangeStateCounter);

  //Actualize the value of the repetition of the breather
  changeRepByMin();
  
  //Cycle the breather
  cycleBreather();

  // add a delay, so the loop is not too fast
  delay(1);
}

// Use to debounce the button with software
void debounceButtonCounter(int button_pin, int *p_currentState, int *p_changeStateCounter){
  int tempState = digitalRead(button_pin);

  // If the value of the button has change
  if(tempState != *p_currentState){
    //Increase counter
    *p_changeStateCounter++;
  }else{
    //If not, reset the counter
    *p_changeStateCounter = 0;
  }

  // If the counter has increase to a value, than change the state
  // The main loop has a 1ms delay, so 25ms should be fine to debounce.
  if(*p_changeStateUpButtonCounter >= 25){
    p_currentState = tempState;
    p_changeStateCounter = 0;
  }

  return;
}

//Change the value of the rep  by minutes
void changeRepByMin(){
  //Change the value of the breather cycle
  //Make sure the two button or not press at the same time
  if(DownButtonCurrentState != UpButtonCurrentState){
    
    //If Down button is press.
    if(DownButtonCurrentState == PRESSED_BUTTONVALUE && DownButtonToggleState == 0){
      breathCounterValue--;
      DownButtonToggleState = 1;
    }else if(DownButtonCurrentState == RELEASED_BUTTONVALUE){
      DownButtonToggleState = 0;
    }

    //If Up button is press.
    if(UpButtonCurrentState == PRESSED_BUTTONVALUE && UpButtonToggleState == 0){
      breathCounterValue++;
      UpButtonToggleState = 1;
    }else if(UpButtonCurrentState == RELEASED_BUTTONVALUE){
      UpButtonToggleState = 0;
    }


    //Make sure the value respect the min max value
    if(breathCounterValue < MINREPBYMIN){
      breathCounterValue = MINREPBYMIN;
    }

    if(breathCounterValue > MAXREPBYMIN){
      breathCounterValue = MAXREPBYMIN;
    }
    //Print the new value on the LCD
    lcd.cursor(0, 2);
    lcd.print("                    ");
    lcd.cursor(0, 2);
    lcd.print(breathCounterValue);
  }
}

//Cycle the breather
//Variables
enum{
  NOBREATHING_STATE,
  NEWCYCLE_STATE,
  BREATHING_STATE,
  DEPRESSION_STATE
}breathing_state_t

breathing_state_t currentState = NOBREATHING_STATE;
int lastTime;
int cycleLength;

void cycleBreather(){
  //Turn on or off the whole system
  if(controlButtonCurrentState == PRESSED_BUTTONVALUE){
    currentState = NOBREATHING_STATE;
  }else{
    currentState = BREATHING_STATE;
  }

  //What to do!
  switch(currentState){
    case NOBREATHING_STATE:
      //Close the valve by security measure
      digitalWrite(VALVE_PIN, LOW);
      break;
    case NEWCYCLE_STATE:
      //Set the value
      lastTime = millis();
      //Calculate the total cycle length in millisecond
      cycleLength = (int) (60*1000) / (breathCounterValue); 
      
      break;
    case BREATHING_STATE:
      //Turn on the valve
      digitalWrite(VALVE_PIN, HIGH);
      if( millis() - lastTime > (cycleLength/2){
        currentState = DEPRESSION_STATE;
      }
      break;
    case DEPRESSION_STATE:
      //Turn on the valve
      digitalWrite(VALVE_PIN, LOW);
      if(millis() - lastTime > (cycleLength/2){
        currentState = NEWCYCLE_STATE;
      }
      break;
    default:
      //In case of something went wrong
      currentState = NOBREATHING_STATE;
      break;
    
  }
}
