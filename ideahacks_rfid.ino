#include <HTMLSerialMonitor.h>
#include <SPI.h>
#include <MFRC522.h>
HTMLSerialMonitor SerialM;

int frameNr = 0;

int red_light_pin = A0;
int green_light_pin = A1;
int blue_light_pin = A2;

const int lifted = 1;
const int sedentary = 0;

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

const int trigPin = 5; 
const int echoPin = 6;

const float THRESHOLD = 5;
float prev_vals[5] = {0, 0, 0, 0, 0};
int i = 0;

int state = 0;
int lifted_count = -1;
unsigned long dockedTime;
unsigned long prevMillis = 0;
unsigned long cummedTime = 0;
unsigned long startLift = 0;
unsigned long totalLift = 0;
unsigned long startTotalTime = 0;

float duration, distance; 


void RGB_color(int red_light_value, int green_light_value, int blue_light_value){
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}

//*****************************************************************************************//
void setup() {

  // RFID Stuff
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                           // Init MFRC522 card
  
  // Ultrasound Setup
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);

  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);
  
  SerialM.begin(9600);
  
  //We setup the gui and start by clearing the page with [cls] 
  SerialM.println("[cls]");
  
  //project title
  SerialM.print("<h1 style='display:flex; justify-content: center; font-family:Helvetica; font-weight:bold;'>LVFTD</h1><hr>");

  // lift state
  SerialM.print("<h3 style='display:flex; justify-content: center; font-family:Helvetica; font-weight:bold; font-size:300%; line-height: 150%;'>LIFT STATE:  <span id='liftState' >" + String(" LIFTED") + "</span></h3>");

  // # of times lifted
  SerialM.print("<h3 style='display:flex; justify-content: center; font-family:Helvetica; font-weight:bold; font-size:300%; line-height: 150%;'>TIMES LIFTED: <span id='liftNum'>" + String(lifted_count) + "</span></h3>");

  // time since last lift
  SerialM.print("<h3 style='display:flex; justify-content: center; font-family:Helvetica; font-weight:bold; font-size:150%; line-height: 150%;'>TIME SINCE LAST LIFT: <span id='dockTime'>" + String(dockedTime) + "</span></h3>");

  // C U M  time
  SerialM.print("<h3 style='display:flex; justify-content: center; font-family:Helvetica; font-weight:bold; font-size:150%; line-height: 150%;'>TOTAL TIME: <span id='cumTime'>" + String(cummedTime) + "</span></h3>");

  SerialM.print("<div style='opacity: 0.6; height: 1000px; width: 2500px; left: 0; bottom: 0; position: fixed; z-index: 0; background-image: linear-gradient(#de0707, #c9c118);'></div>");

}

//*****************************************************************************************//
void loop() {

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    digitalWrite(trigPin, LOW); 
    delayMicroseconds(2); 
    digitalWrite(trigPin, HIGH); 
    delayMicroseconds(10); 
    digitalWrite(trigPin, LOW); 
  
    duration = pulseIn(echoPin, HIGH); 
    distance = (duration*.0343)/2;
  
    prev_vals[i] = distance;
    i = (i + 1) % 5;
  
    int count = 0;
    for (int itr = 0; itr < 5; itr++) {
      if (prev_vals[itr] > THRESHOLD) {
        count += 1;
      }
    }
  
    if (count > 3) { //if lifted
      if (state == sedentary) { //only if changing from docked to lifted
        RGB_color(0, 255,0);
        startLift = (millis())/1000;
        lifted_count += 1;
        state = lifted;
      }
      prevMillis = millis();
      String liftState="[liftState:"+String("LIFTED")+"]";
      SerialM.print(liftState);
    }
    else { //if on dock
      if (state == lifted) { //only if changing from lifted to docked
        RGB_color(0,0,255);
        totalLift += (millis())/1000-startLift;
        state = sedentary;
      }
      String liftState="[liftState:"+String("DOCKED")+"]";
      SerialM.print(liftState);
    }
    String liftNum="[liftNum:"+ String(lifted_count)+"]";
    SerialM.print(liftNum);
  
    dockedTime = millis() - prevMillis;
    dockedTime/=1000;    

    cummedTime = (millis())/1000 - startTotalTime;

    String dockTime ="[dockTime:"+String(dockedTime)+"]";
    SerialM.print(dockTime);

    String cumTime ="[cumTime:"+String(cummedTime)+"]";
    SerialM.print(cumTime);
  
    //close print string to let monitor proces it    
    SerialM.println("");
  
    //we call hndlRemoteCmd to enable the control buttons (play/reset) at the top right of the monitor
    SerialM.hndlRemoteCmd(); //or call serialEvent if you need the input yourself
     
    delay(100); 
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Card deteted, reset count
  lifted_count = 0;
  startTotalTime = (millis()) / 1000;
  
  
  
  delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  
  
}
//*****************************************************************************************//
