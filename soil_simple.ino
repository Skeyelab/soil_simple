#include <SdFat.h>
#include <SoftwareSerial.h>
#include <serLCD.h>
#include "RunningAverage.h"

// SD chip select pin
const uint8_t chipSelect = SS;
// file system object
SdFat sd;
// create Serial stream
ArduinoOutStream cout(Serial);
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))

int pin = 2;
serLCD lcd(pin);

RunningAverage tmpAvg(10);
RunningAverage lgtAvg(10);
RunningAverage mstAvg(10);


int lightSensor = 1;
int tempSensor = 0;
int light_val;
int temp_val;
int moisture_val_ac;
int delayTime = 500;

int tempCal = 0;

int mst100 = 900;
int mst0 = 0;

#define moisture_input 4
#define divider_top 7
#define divider_bottom 8


void setup() {
  lcd.setBrightness(30);
  lcd.clear();
  lcd.print("Booting...");
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();
  Serial.begin(9600); //open serial port
  delay(500);
  lcd.print("OK");
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


  lcd.clear();
  lcd.print(tmpAvg.getAverage());



  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);

    char name[] = "APP.TXT";
    ofstream sdout(name, ios::out | ios::app);
    if (!sdout) error("open failed");
    sdout  << "M:" << MoisturePercentage(avgMst,mst100) << " T:" << tmpAvg.getAverage() << endl;
    sdout.close();
    delay(delayTime);

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








