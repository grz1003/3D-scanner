 /* 
 * Pin Layout:
 * SD card:
 * - MOSI - pin 16
 * - MISO - pin 14
 * - SCK - pin 15
 * - CS - pin 10
 * 
 * IR Sensor:
 * - Sensor - A3
 * 
 * Turntable stepper motor driver board:
 * - STEP - pin 2
 * - DIR - pin 3 
 * - MS1 - pin 4
 * - MS2 - pin 5
 * - Enable - pin 6
 * 
 * Sensor stepper motor driver board:
 * - STEP - pin 7
 * - DIR - pin 8
 * - MS1 - pin 9
 * - MS2 - pin A0
 * - ENABLE - pin A1
 */

#include <SPI.h>
#include <SD.h>

/*Create File*/
File scannerValues;
String filename="scanCord.txt";

/*SD Pin*/
int csPin = 10;

/*Sensor Pin*/
int sensorPin = A3;

/*Turntable Motor Pin*/
int tStep = 2;
int tDir = 3;
int tMS1 = 4;
int tMS2 = 5;
int tEnable = 6;

/*Sensor Motor Pin*/
int sStep = 7;
int sDir = 8;
int sMS1 = 9;
int sMS2 = A0;
int sEnable = A1;

/*Error Led*/
int ErrorLed = A2;

void setup() {
	
  // Turn on debug communication with computer
  Serial.begin(9600);
  
  if (!Serial) {
    delay(5000); //wait 5 sec for computer to respond, if not start running the program
  }
  
  Serial.println("Program Started!");

  //Define Pins
  pinMode(tStep, OUTPUT);
  pinMode(tDir, OUTPUT);
  pinMode(tMS1, OUTPUT);
  pinMode(tMS2, OUTPUT);
  pinMode(tEnable, OUTPUT);
  pinMode(sStep, OUTPUT);
  pinMode(sDir, OUTPUT);
  pinMode(sMS1, OUTPUT);
  pinMode(sMS2, OUTPUT);
  pinMode(sEnable, OUTPUT);
  
  pinMode(ErrorLed, OUTPUT);

  Serial.println("Finish Setting PIN!");
  
  //turn led off for no error yet
  digitalWrite(ErrorLed, LOW); 

  //Set microstepping mode for stepper driver boards.
  //MS1 and MS2 as LOW: 1.8 deg turn angle (200 steps/rev)
  //MS1 HIGH and MS2 LOW: 0.9 deg turn angle (400 steps/rev)

  // Turntable motor:
  digitalWrite(tMS1, LOW);
  digitalWrite(tMS2, LOW);
    
  // Sensor motor:  
  //no micro stepping (MS1 Low, MS2 Low) = 1.8 deg/step (200 steps/rev) --> (200 steps/4mm).
  digitalWrite(sMS1, LOW);
  digitalWrite(sMS2, LOW);

  //Enable motor controllers
  digitalWrite(tEnable, LOW);
  digitalWrite(sEnable, HIGH);
  
  digitalWrite(sDir, HIGH); 
  digitalWrite(tDir, LOW); 

  //Debug to Serial
  Serial.print("Checking SD Card ~ Openning ");
  if (!SD.begin(csPin))
  {
    Serial.println("FAILED!");
	digitalWrite(ErrorLed, HIGH); 
    while(1);
    return;
  }
  
  Serial.println("Finished!");
  
}

void loop() {
  int Height = 2; //How tall is the object or the Max height possible
  int nsStep = 200; //Step for Height(depand on screw)
  int Rotate = 10; //360/1.8=200
  int sRotate = (Height*200/0.4/nsStep); //number of rotate of nsStep to reach top or bottom
  double AvgMeasured = 0;

  Serial.println("Loop Began!");
  Serial.println("(Point, Average Distance)");

  // Scan object
  digitalWrite(sDir, HIGH); 
  for (int j = 0; j < sRotate; j++) //Rotate Height
  { Serial.println();
    Serial.print("Scanning Revolution: ");
    Serial.print(j);
	  Serial.print(". Number of revolution remain: ");
    Serial.println(sRotate - j);
    
    for (int i = 0; i < Rotate; i++)   //Rotate theta motor for one revolution, read sensor and store
    {
      rotateMotor(tStep, 20); //Rotate theta motor one step
      delay(250);//wait for motor to finish rotating and object to be stable
      AvgMeasured = 0; //Reset;
	  Serial.print("(");
	  Serial.print(i);
	  Serial.print(", ");
      AvgMeasured = readAnalogSensor(); //get Distance reading
      writeToSD(AvgMeasured); //Store
    }
    
    //Increase Height
    digitalWrite(tEnable, HIGH);
	  digitalWrite(sEnable, LOW);
    delay(1000);
    rotateMotor(sStep, nsStep);
    delay(250); //for sensor to stablize
    digitalWrite(sEnable, HIGH);
    digitalWrite(tEnable, LOW);
    delay(10);
	  writeEnd();
  }

  // Scan complete. Return to Base
  digitalWrite(sDir, LOW);
  digitalWrite(sEnable, LOW);
  delay(1000);  
  for (int j = 0; j < sRotate; j++)
  {
    rotateMotor(sStep, nsStep);
  }
  digitalWrite(sDir, HIGH);
  digitalWrite(sEnable, HIGH);

  //Finished Scanning
  Serial.println
  Serial.println("Finished Scanning");
  delay(60000);
  return; 
     
}

void rotateMotor(int pinNo, int steps)
{
  //Serial.println("Turning!"); 
  for (int i = 0; i < steps; i++)
  {
    digitalWrite(pinNo, LOW); //LOW to HIGH changes creates the
    delay(5);
    digitalWrite(pinNo, HIGH); //"Rising Edge" so that the EasyDriver knows when to step.
    delay(5);
  }
  
}

double readAnalogSensor()
{ 
  //Serial.println("Scanning Distance!");
  int Count = 10;
  double sumTemp = 0;

  double temp = 0;
  double AvgMeasured = 0;
    
  for (int i = 0; i < Count; i++)
  {
    temp = loadSensor(); //Reading Value
    sumTemp = sumTemp + temp; //Running sum of sensed distances
    delay(20); //each measurement is 20ms apart
  }
    
  AvgMeasured = sumTemp/Count; //Calculate mean
  //Serial.print("Avg Scanned Distance: ");
  Serial.print(AvgMeasured);
  Serial.print(") ");
  return AvgMeasured;
}

double loadSensor()
{
  int temp;
  double sOutput;
  temp = analogRead(sensorPin);
  sOutput = checkSensor(temp);

  if(sOutput < 2.6 && sOutput > 0.2){
	//Serial.print("Scanned Distance: ");
	//Serial.println(sOutput);
    return sOutput;
  } else {
	delay(1);
    return loadSensor();
  }
}

void writeToSD(double AvgMeasured)
{
  //Serial.println("Writing to SD!");
  // Open file
  scannerValues = SD.open(filename, FILE_WRITE);
    
  //Write
  if (scannerValues) 
  {
    //Write to file
    scannerValues.print(AvgMeasured);
    scannerValues.println();
    
    // close the file:
    scannerValues.close();
  } 
  else 
  {
    Serial.print("error opening ");
    Serial.println(filename);
	  digitalWrite(ErrorLed, HIGH); 
	while(1);
  }
}

void writeEnd()
{
  // Open file
  scannerValues = SD.open(filename, FILE_WRITE);
    
  //Write
  if (scannerValues) 
  {
    //Write to file
    scannerValues.print("EndLine----");
    scannerValues.println();
    scannerValues.println();
    
    // close the file:
    scannerValues.close();
  } 
  else 
  {
    Serial.print("error opening ");
    Serial.println(filename);
	  digitalWrite(ErrorLed, HIGH);
	while(1);
  }
}

double checkSensor(int Distance)
{
	if(Distance > 82 && Distance < 429){
		double tempDis;
		tempDis = Distance * 1.0;
		return tempDis * 5.0 / 1023.0;
	}
	delay(5);
	return(loadSensor());
}
