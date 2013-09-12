#include <SdFat.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// file system object
SdFat sd;

// create Serial stream
ArduinoOutStream cout(Serial);

// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
#include <LiquidCrystal.h>
#include "RunningAverage.h"

RunningAverage tmpAvg(10);
RunningAverage lgtAvg(10);
RunningAverage mstAvg(10);

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int lightSensor = 1;
int tempSensor = 0;
int light_val;
int temp_val;
int moisture_val_ac;
int delayTime = 5000;

int tempCal = 3;

int mst100 = 900;
int mst0 = 0;

#define moisture_input 4
#define divider_top 7
#define divider_bottom 8

int switchState = 0;
int audibleAlarm = 0;

void setup() {

  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();

  Serial.begin(9600); //open serial port
  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, INPUT);
}

void loop() {

  switchState=digitalRead(9);

  if (switchState==HIGH){
    audibleAlarm = !audibleAlarm;
  }
  //collect moisture data
  moisture_val_ac = SoilMoisture();
  mstAvg.addValue( moisture_val_ac);

  Serial.print("Moistrue Reading ");
  Serial.print(moisture_val_ac);
  Serial.print(" ");
  Serial.println(MoisturePercentage(moisture_val_ac,mst100));
  //collect light data  
  light_val = analogRead(lightSensor); // read the value from the photosensor
  Serial.print("light sensor reads ");
  Serial.println( light_val );
  lgtAvg.addValue( light_val);


  //collect temp data  
  temp_val = analogRead(tempSensor);
  Serial.print("temp sensor reads ");
  Serial.print( temp_val );
  // convert the ADC reading to voltage
  float voltage = ((temp_val + tempCal)/1024.0) * 5.0;
  // Send the voltage level out the Serial port
  Serial.print(", Volts: ");
  Serial.print(voltage);
  Serial.print(", degrees C: "); 
  float temperature = (voltage - .5) * 100;
  Serial.print(temperature);
  Serial.print(", degrees F: "); 
  float Ftemperature = (temperature * (9.0/5.0)) + 32.0;
  Serial.println(Ftemperature);

  tmpAvg.addValue( Ftemperature);

  int avgMst = mstAvg.getAverage();
  int avgLgt = lgtAvg.getAverage();



  //output data to LCD
  lcd.begin(16, 2);
  lcd.print(" M% |  L  |  T");
  lcd.setCursor(0, 1);
  lcd.print("    |     |");
  lcd.setCursor(0, 1);
  lcd.print( MoisturePercentage(avgMst,mst100));
  lcd.setCursor(4, 1);
  lcd.print("|");
  lcd.setCursor(6, 1);
  lcd.print(avgLgt);
  lcd.setCursor(11, 1);
  lcd.print(tmpAvg.getAverage());  
  lcd.setCursor(15, 1);
  lcd.print("F"); 
  if (audibleAlarm){
    lcd.setCursor(15, 0);
    lcd.print("*");  
  }
  char name[] = "APP.TXT";
  ofstream sdout(name, ios::out | ios::app);
  if (!sdout) {
    Serial.println("open failed");
  }
  sdout  << "M:" << MoisturePercentage(avgMst,mst100) << " T:" << tmpAvg.getAverage() << endl;
  sdout.close();

  delay(delayTime);

  //sound alarm for low readings
  if (moisture_val_ac < 600){
    if (audibleAlarm){
      //  buzz(speaker, 200, 50);
    }
    //  LEDalarm(LED, 10);  
  }

}

void LEDalarm(int targetPin, int numFlashes){

  for (int Flashes = 0; Flashes < numFlashes; Flashes++) { 
    digitalWrite(targetPin, HIGH);
    delay(1000);
    digitalWrite(targetPin, LOW);
    delay(500);
  }

}

void buzz(int targetPin, long frequency, long length) {
  digitalWrite(13,HIGH);
  long delayValue = 1000000/frequency/2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length/ 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to 
  //// get the total number of cycles to produce
  for (long i=0; i < numCycles; i++){ // for the calculated length of time...
    digitalWrite(targetPin,HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin,LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
  digitalWrite(13,LOW);
}

int SoilMoisture(){
  int reading;
  // set driver pins to outputs
  pinMode(divider_top,OUTPUT);
  pinMode(divider_bottom,OUTPUT);

  // drive a current through the divider in one direction
  digitalWrite(divider_top,LOW);
  digitalWrite(divider_bottom,HIGH);

  // wait a moment for capacitance effects to settle
  delay(1000);

  // take a reading
  reading=analogRead(moisture_input);

  // reverse the current
  digitalWrite(divider_top,HIGH);
  digitalWrite(divider_bottom,LOW);

  // give as much time in 'reverse' as in 'forward'
  delay(1000);

  // stop the current
  digitalWrite(divider_bottom,LOW);

  return reading;
}

float MoisturePercentage(int moisture, int mst100){

  float percent;
  percent = (moisture * 100.0)/mst100;
  Serial.println(percent);
  return (moisture * 100.0)/mst100;
}





