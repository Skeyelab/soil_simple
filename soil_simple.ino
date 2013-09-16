#include <SoftwareSerial.h>
#include <serLCD.h>
#include "RunningAverage.h"
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;

int pin = 2;
serLCD lcd(pin);

RunningAverage tmpAvg(20);
//RunningAverage lgtAvg(10);
RunningAverage mstAvg(20);

const int chipSelect = 10;

//int lightSensor = 1;
int tempSensor = 0;
int light_val;
int temp_val;
int moisture_val_ac;
int delayTime = 3000;

int tempCal = 0;

int mst100 = 900;
int mst0 = 0;

#define moisture_input 2
#define divider_top 3
#define divider_bottom 4


void setup() {
  lcd.setBrightness(30);
  lcd.clear();
  lcd.print("Booting...");
  delay(500);
  Serial.begin(9600); //open serial port
  lcd.clear();
  lcd.print("Initializing SD card...");
  delay(500);

  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.print("Card failed, or not present");
    delay(500);

    // don't do anything more:
    return;
  }
  lcd.clear();
  lcd.print("card initialized");
  delay(500);

  lcd.clear();
  lcd.print("OK");
  delay(500);

}

void loop() {


  //collect moisture data
   moisture_val_ac = SoilMoisture();
   mstAvg.addValue( moisture_val_ac);

  Serial.print("Moistrue Reading ");
  Serial.print(moisture_val_ac);
  Serial.print(" ");
  Serial.println(MoisturePercentage(moisture_val_ac,mst100));
  //collect light data  
//  light_val = analogRead(lightSensor); // read the value from the photosensor
//  Serial.print("light sensor reads ");
//  Serial.println( light_val );
  //  lgtAvg.addValue( light_val);


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
  //  int avgLgt = lgtAvg.getAverage();
  int avgTmp = tmpAvg.getAverage();



  lcd.clear();
  
  lcd.print("Tmp:");
  lcd.print(avgTmp);
  lcd.print("  Mst:");
  lcd.print(moisture_val_ac);
 
  // make a string for assembling the data to log:
  //String dataString = "poops!";

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("real.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(avgTmp);
    dataFile.print(",");
    dataFile.println(avgMst);
    dataFile.close();
    // print to the serial port too:
    Serial.println("written:");
    Serial.println(avgTmp);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
  lcd.selectLine(2);

  for(int u = 0; u < 16; u++){
    lcd.print(".");
    delay(delayTime/16);
  }
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














