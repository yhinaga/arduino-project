/*
 * This is the programming that collects the data of temperature, humidity, heat index, and moisture levels. 
 * When the moisture level is lower than the certain level, water will be provided to three plants.
 * 
 */


#include <SD.h>
#include <SPI.h>

#include "DHT.h"
#define DHTPIN 2 // pin for temperature and humidity sensor
#define MOISTSENS_1 17 // it connects to moisture sensor; Pin A3 on the arduino 
#define MOISTSENS_2 18
#define MOISTSENS_3 19
#define LED 4 // connects to LED
#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define Valve1_Open 4 // Pin D4 on the arduino -> mint
#define Valve1_Close 5 // Pin D5 on the arduino -> mint
#define Valve2_Open 6 // Pin D6 on the arduino -> cam...
#define Valve2_Close 7 // Pin D7 on the arduino -> cam...
#define Valve3_Open 8 // Pin D8 on the arduino -> lavender
#define Valve3_Close 9 // Pin D9 on the arduino -> lavender

DHT dht(DHTPIN, DHTTYPE);

int CS_PIN = 10; //This is the serial PIN for the SD card reader.
int start = 1;

unsigned int Day=1;
unsigned int Clock=12;
unsigned long timer=0; 

unsigned int moisture_1=0; // Moisture level (when 0, no water)
unsigned int moisture_2=0;
unsigned int moisture_3=0;

unsigned int water_1=0; //water=0 means that there is no neceessity to give water.
unsigned int water_2=0;
unsigned int water_3=0;

unsigned long t_initial=5;
unsigned long t_final=0;
//unsigned int water_pressure_delay=0; // This value is for when the amount of water in the tunk changes and the pressure to provide water changes. 
                                     // We eventually have to open the valve longer when the amount of water in the tunk decreases.
//water_pressure_delay=(v-p)/t; //This equations (v-p)/t changes later. 

unsigned long plant_vol1=0.0005;
unsigned long plant_vol2=0.00025;
unsigned long plant_vol3=0.0005; //values in units of m^3
unsigned long plant_volume;

File file;

void setup()
{
  delay(1000);
  timer = millis();
  pinMode(Valve1_Open, OUTPUT); // this order is output
  pinMode(Valve1_Close, OUTPUT); 
  pinMode(Valve2_Open, OUTPUT); // this order is output
  pinMode(Valve2_Close, OUTPUT); 
  pinMode(Valve3_Open, OUTPUT); // this order is output
  pinMode(Valve3_Close, OUTPUT); 
  pinMode(MOISTSENS_1, INPUT); // moisture sensor is input
  pinMode(MOISTSENS_2, INPUT);
  pinMode(MOISTSENS_3, INPUT);
  Serial.begin(9600);
  Serial.println("DHTxx test!");
  
  
  dht.begin(); // begin temperature and humidity sensor.

  initializeSD(); //writeToFile("This is the data accumilated in the senso 

  digitalWrite(Valve1_Close, HIGH); //All the valves are closed before any measurement starts. 
  delay(8000);
  digitalWrite(Valve1_Close,LOW);
  digitalWrite(Valve2_Close, HIGH);
  delay(8000);
  digitalWrite(Valve2_Close,LOW);
  digitalWrite(Valve3_Close, HIGH);
  delay(8000);
  digitalWrite(Valve3_Close,LOW);

/*
  openFile("prefs.txt");
  Serial.println(readLine());
  Serial.println(readLine());
  closeFile();
*/

  Measurement(timer);

}

void loop()
{
  if (start == 1)
  {
   createFile("Data.txt");
   file.println("Day,Time,Humidity,Temperature,Moisture Level 1,Moisture Level 2,Moisture Level 3,Plant 1 Watered,Plant 2 Watered,Plant 3 Watered");
   closeFile();

   start=0;
  }
  //timer;

 
  //delay(10000);// Wait a few seconds between measurements.

if ((millis() - timer) >= 21600000) //this value should be 21600000 (6 hours)
{
  //Serial.print("millis =");
  //Serial.println(millis());

  //Serial.print("timer =");
  //Serial.println(timer);
  
  timer = millis(); // Compute heat index in Celsius (isFahreheit = false)

  Measurement(timer); //"Measurement" is a function that measures temperature, humidity, and moisture. The detail of the function is at the bottom.

  t_final=(-sqrt(0.20)+sqrt(0.20+5*((0.000052*0.000052)*(t_initial^2)/(0.8)+(0.000052)*sqrt(0.20)*t_initial+plant_volume)))/(0.000052/0.4);
 // t_final=(-sqrt(0.20)+sqrt(0.20+5*(0.000052*0.000065*(t_initial^2)+(0.000052)*sqrt(0.20)*t_initial+plant_volume)))/0.00013;
  Serial.println(t_final);
  t_initial=t_initial+t_final;

}
 Water();
}


void initializeSD()
{
  Serial.println("Initializing SD card...");
  pinMode(CS_PIN, OUTPUT);

  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
}


int createFile(char filename[])
{
  delay(500);
  file = SD.open(filename, FILE_WRITE);

  if (file)
  {
    Serial.println("File created successfully.");
    return 1;
  } 
  else
  {
    Serial.println("Error while creating file.");
    return 0;
  }
}

int writeToFile(char text[])
{
  if (file)
  {
    file.print(text);
    Serial.println("Writing to file: ");
    Serial.println(text);
    return 1;
  } else
  {
    Serial.println("Couldn't write to file");
    return 0;
  }
}

void closeFile()
{
  delay(500);
  
  if (file)
  {
    file.close();
    Serial.println("File closed");
  }
}

int openFile(char filename[])
{
  delay(500);
  
  file = SD.open(filename);
  if (file)
  {
    Serial.println("File opened with success!");
    return 1;
  } else
  {
    Serial.println("Error opening file...");
    return 0;
  }
}

String readLine()
{
  String received = "";
  char ch;
  while (file.available())
  {
    ch = file.read();
    if (ch == '\n')
    {
      return String(received);
    }
    else
    {
      received += ch;
    }
  }
  return "";
}

void Measurement(unsigned long tmr) //tmr means timer
{
  createFile("Data.txt");
  file.print(Day);
  file.print(",");
  file.print(Clock);
  file.print(":00,");
  
// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  
  float h = dht.readHumidity();// Read temperature as Celsius (the default)
  float t = dht.readTemperature(); // Read temperature as Fahrenheit
  float f = dht.readTemperature (true); // This may or may not be necessary.


// Check if any reads failed and exit early (to try again).
// isnan = if humidity/temperature reading is not a number,

if (isnan(h) || isnan(t) || isnan(f)) 
{
  Serial.println("Failed to read from DHT sensor!");
  return;
}
 int moisture_1 = analogRead(MOISTSENS_1); // mesuring the moisture level
 int moisture_2 = analogRead(MOISTSENS_2);
 int moisture_3 = analogRead(MOISTSENS_3);
 

  float hif = dht.computeHeatIndex (f, h);
  float hic = dht.computeHeatIndex(t, h, false); // This is necessary; don't erase!!

  unsigned long hours = tmr; //Change milliseconds into hours

  //Serial.print("Time: ");
  //Serial.println(tmr); //Serial.print(hours/3600000);
  //Serial.print(" hours ");
  
  //Serial.print("Humidity: ");
  //Serial.print(h);
  file.print(h);
  file.print(",");
  //Serial.print(" %\t");
  //Serial.print("Temperature: ");
  //Serial.print(t);
  file.print(t);
  file.print(",");
  //Serial.print(" *C ");
  //Serial.print("Heat index: ");
  //Serial.print(hic);
  //Serial.println(" *C ");
  
  Serial.print( "Moisture_1: ");
  Serial.println(moisture_1);
  file.print(moisture_1);
  file.print(",");

  Serial.print( "Moisture_2: ");
  Serial.println(moisture_2);
  file.print(moisture_2);
  file.print(",");

  Serial.print( "Moisture_3: ");
  Serial.println(moisture_3);
  file.print(moisture_3);
  file.print(",");

  if (moisture_1<450) // When the moisture level is below 300 (changes later), we decide we need to give water (showed as 1).
  {
    water_1=1;
    file.print("Yes,");

    //Serial.println("Plant 1 needs water");
  }
  else
  {
    water_1=0;
    file.print("No,");
    
    //Serial.println("Plant 1 has sufficient water");
  }

  if (moisture_2<350)
  {
    water_2=1;
    file.print("Yes,");

    //Serial.println("Plant 2 needs water");
  }
  else
  {
    water_2=0;
    file.print("No,");

    //Serial.println("Plant 2 has sufficient water");
  }

  if (moisture_3<500)
  {
    water_3=1;
    file.println("Yes");

    //Serial.println("Plant 3 needs water");
  }
  else
  {
    water_3=0;
    file.println("No");

    //Serial.println("Plant 3 has sufficient water");
  }

  
  closeFile();


  if(Clock==22)
  {
    Clock=4;
  }
  else
  {
    Clock=Clock+6;
  }

  if(Clock==4)
  {
    Day=Day++;
  }
}

void Water_Pressure_Delay(unsigned long plant_volume)
{
  t_final=1000*(-sqrt(0.20)+sqrt(0.20+5*(0.000052*0.000065*(t_initial^2)+(0.000052)*sqrt(0.20)*t_initial+plant_volume)))/0.00013;
  Serial.println(t_final);
  t_initial=t_initial+t_final;
}

void Water()
{
  if (water_1==1)  // Open the valve, give water, close the valve, and reset the necessity of providing water.
  {
    Water_Pressure_Delay(plant_vol1);
    digitalWrite(Valve1_Open, HIGH);
    //Serial.println("Start opening Valve_1");
    delay(10000);  // should be 8,500
    digitalWrite(Valve1_Open,LOW);
    //Serial.println("Stop opening Valve_1");
    delay(t_final);
    digitalWrite(Valve1_Close, HIGH);
    //Serial.println("Start closing Valve_1");
    delay(8000);
    digitalWrite(Valve1_Close,LOW);
    //Serial.println("Stop closing Valve_1");
    water_1=0;
  }
  
  if (water_2==1)
  {
    Water_Pressure_Delay(plant_vol2);
    digitalWrite(Valve2_Open, HIGH);
    //Serial.println("Start opening Valve_2");
    delay(10000);
    digitalWrite(Valve2_Open,LOW);
    //Serial.println("Stop opening Valve_2");
    delay(t_final);
    digitalWrite(Valve2_Close, HIGH);
    //Serial.println("Start closing Valve_2");
    delay(8000);
    digitalWrite(Valve2_Close,LOW);
    //Serial.println("Stop closing Valve_2");
    water_2=0;
  }
  
  if (water_3==1)
  {
    Water_Pressure_Delay(plant_vol3);
    digitalWrite(Valve3_Open, HIGH); 
    //Serial.println("Start opening Valve_3");
    delay(10000);
    digitalWrite(Valve3_Open,LOW);
    //Serial.println("Stop opening Valve_3");
    delay(t_final);
    //Serial.print("t_final= "); Serial.println(t_final);
    digitalWrite(Valve3_Close, HIGH);
    //Serial.println("Start closing Valve_3");
    delay(8000);
    digitalWrite(Valve3_Close,LOW);
    //Serial.println("Stop closing Valve_3");
    water_3=0;
  }
  
}


// This programming: 
// 1. measures temperature humidity, and moisture of all plants 3 times a day (every 8 hours) -> roop "time=milis();”// returns numbers of millisconds since start
// 2. takes photos once per day
// 3. waters plants based on moisture levels
// 4. records the data, pictures, and the action on the SD card.


