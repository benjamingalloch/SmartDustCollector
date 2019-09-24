

*/
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h> 
 
// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!

// our servo # counter
uint8_t servoCount = 4;
uint8_t servonum = 0;

Servo puerta1;                                         
Servo puerta2;
Servo puerta3;
Servo puerta4;


const int OPEN_ALL = 100;
const int CLOSE_ALL = 99;

boolean powerDetected = 0;
boolean collectorIsOn = 0;
int DC_spindown = 3000;

const int NUMBER_OF_TOOLS = 4;
const int NUMBER_OF_GATES = 4;

String tools[NUMBER_OF_TOOLS] = {"Ingletadora","Herramienta1","Herramienta2","Sierra"};
int voltSensor[NUMBER_OF_TOOLS] = {A1,A2,A3,A4};
long int voltBaseline[NUMBER_OF_TOOLS] = {0,0,0,0};
Servo arregloMotores[NUMBER_OF_TOOLS] = {puerta1,puerta2,puerta3,puerta4};

//DC right, Y, miter, bandsaw, saw Y, tablesaw, floor sweep
//Set the throw of each gate separately, if needed
int gateMinMax[NUMBER_OF_TOOLS][2] = {
  /*open, close*/
  {90,2},//DC right
  {90,2},//Y
  {90,2},//miter
  {90,2},//bandsaw
};

//keep track of gates to be toggled ON/OFF for each tool
int gates[NUMBER_OF_TOOLS] = {1,0,1,0};
 

const int dustCollectionRelayPin = 2;

int mVperAmp = 66; // use 100 for 20A Module and 66 for 30A Module
double ampThreshold = 1;

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled

void setup(){ 
  Serial.begin(9600);
  Serial.println("Hola");
  pinMode(dustCollectionRelayPin,OUTPUT);
  Serial.println("1");
  puerta1.attach(3, 900, 2100);
  puerta2.attach(5, 900, 2100);
  puerta3.attach(6, 900, 2100);
  puerta4.attach(9, 900, 2100);
  
 //record baseline sensor settings
 //currently unused, but could be used for voltage comparison if need be.
  
  for(int i=0;i<NUMBER_OF_TOOLS;i++){
    pinMode(voltSensor[i],INPUT);
    voltBaseline[i] = analogRead(voltSensor[i]); 
  }
  
}

void loop(){
  
   Serial.println("----------");
   //loop through tools and check
      
   int activeTool = 50;// a number that will never happen
   for(int i=0;i<NUMBER_OF_TOOLS;i++){
      if(checkForAmperageChange(i)){
        activeTool = i;
        exit;
      }
      if(i!=0)
        if(checkForAmperageChange(0)){
          activeTool = 0;
          exit;
        }
   }
 if(activeTool != 50){
    // use activeTool for gate processing
    if(collectorIsOn == false){
        openGate(activeTool);
        turnOnDustCollection();     
      }
    else
      if(collectorIsOn == true){
        delay(DC_spindown);
        turnOffDustCollection();  
    }
}
}
boolean checkForAmperageChange(int which){ //Devuelve si esta o no prendida la maquina
   Voltage = getVPP(voltSensor[which]);
   VRMS = (Voltage/2.0) *0.05; 
   AmpsRMS = (VRMS * 1000)/mVperAmp;
   //Serial.print(tools[which]+": ");
  // Serial.print(AmpsRMS);
  // Serial.print(Voltage);
   //Serial.println(" Amps RMS");
   
   
   if(AmpsRMS>ampThreshold){
    return true;
   }
   else{
   return false; 
   }
}
void turnOnDustCollection(){
  Serial.println("turnOnDustCollection");
  digitalWrite(dustCollectionRelayPin,1);
  collectorIsOn = true;
}
void turnOffDustCollection(){
  Serial.println("turnOffDustCollection");
  digitalWrite(dustCollectionRelayPin,0);
  collectorIsOn = false;
}
 
float getVPP(int sensor) //Devuelve en Volt
{
  float result;
  
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 500) //sample for 1 Sec
   {
       readValue = analogRead(sensor);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minim sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = (maxValue - minValue) * (5.0/1023.0);
   Serial.println("max");
   Serial.println(maxValue);
   Serial.println("min");
   Serial.println(minValue);
   
      
   return result;
 }

void closeGate(uint8_t num){
  Serial.print("closeGate ");
  Serial.println(num);
  arregloMotores[num].write(0);
}
void openGate(uint8_t num){
  Serial.print("openGate ");
  Serial.println(num);
  arregloMotores[num].write(90);
  delay(100);
}
