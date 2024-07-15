#include <SPI.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN on Blue Pill
const uint64_t address = 0xF0F0F0F0E1LL;

#define ONE_WIRE_BUS 5

#define RE 8
#define DE 7

const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

byte values[11];
SoftwareSerial mod(2, 3);

const int AirValue = 645;   // Replace with Value_1
const int WaterValue = 254;  // Replace with Value_2
int soilMoistureValue = 0;
int soilmoisturepercent = 0;

float temperature;
int nitrogen;
int phosphorous;
int potassium;
float pHValue;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

struct MyVariable
{
  byte soilmoisturepercent;
  byte nitrogen;
  byte phosphorous;
  byte potassium;
  float temperature;  // Keep as float for Celsius
  float pHValue;      // Add pH value
};
MyVariable variable;

void setup()
{
  Serial.begin(9600);
  mod.begin(9600);
  radio.begin();                  // Starting the Wireless communication
  radio.openWritingPipe(address); // Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  // Set as minimum or maximum depending on distance
  radio.stopListening();          // This sets the module as transmitter
  sensors.begin();
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
}

void loop()
{
  nitrogen = readNitrogen();
  delay(250);
  phosphorous = readPhosphorous();
  delay(250);
  potassium = readPotassium();
  delay(250);

  soilMoistureValue = analogRead(A0);  // Put Sensor into soil
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  
  if (soilmoisturepercent >= 100) {
    soilmoisturepercent = 100;
  } else if (soilmoisturepercent <= 0) {
    soilmoisturepercent = 0;
  }
  delay(1000);

  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);

  // Generate a random pH value between 0 and 14
  pHValue = random(50, 100) / 10.0;

  variable.soilmoisturepercent = soilmoisturepercent;
  variable.nitrogen = nitrogen;
  variable.phosphorous = phosphorous;
  variable.potassium = potassium;
  variable.temperature = temperature;  // Store Celsius temperature
  variable.pHValue = pHValue;  // Store pH value
  delay(1200);

  Serial.print(variable.soilmoisturepercent);
  Serial.print("%, ");
  Serial.print(variable.nitrogen);
  Serial.print(" mg/kg, ");
  Serial.print(variable.phosphorous);
  Serial.print(" mg/kg, ");
  Serial.print(variable.potassium);
  Serial.print(" mg/kg, ");
  Serial.print(variable.temperature);
  Serial.print(" °C, ");
  Serial.print(variable.pHValue);
  Serial.println(" pH");
  
  // Uncomment to send data over RF
  // radio.write(&variable, sizeof(MyVariable));
  // Serial.print("Data Packet Sent");
  // Serial.println("");
  
  delay(1000);
}

byte readNitrogen() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(nitro, sizeof(nitro)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      values[i] = mod.read();
    }
  }
  return values[4];
}

byte readPhosphorous() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(phos, sizeof(phos)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      values[i] = mod.read();
    }
  }
  return values[4];
}

byte readPotassium() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(pota, sizeof(pota)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      values[i] = mod.read();
    }
  }
  return values[4];
}
