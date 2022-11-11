/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

  Blynk library is licensed under MIT license

 *************************************************************

  This program push DS18B20 temperature sensor values to Blynk app,
  and notify when the temperature is low.

  WARNING :
  For this example you'll need ESP8266Wifi (esp8266 Community 3.0.2),
  OneWire (2.3.6), DallasTemperature (3.9.0), Blynk (1.0.1) libraries.

  App project setup:
    - Value Display widget attached to V0-V(NumberofThermometers-1)
    - Numeric Input attached to V50
    - Button (in whitch mode, 0-1)attached to V51
    - Notifications

  Notofication App: Message Alarm (Ringing when Blynk notification arrives)

  Wireing:
  GND  - Brown, Brown-White, Green-White
  5V   - Blue, Blue-White
  Data - Green
  Free - Red, Red-White

  DS18B20
  Data - Yellow
  Ground - Black
  VCC - Red

  *************************************************************
  Error codes:
    -99 - Could not read temperature data. (not find sensor adress or no connection)
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

// Include the libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMPERATURE_PRECISION 12

// !! Data wire is plugged into pin 2 and pin ? on the Arduino
//#define ONE_WIRE_BUS1 4
//#define ONE_WIRE_BUS2 5

#define D0 16 // LED 2
#define D1 5  // I2C Bus SCL (clock)
#define D2 4  // I2C Bus SDA (data)
#define D3 0
#define D4 2  // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3  // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)
#define SD2 9
#define SD3 10

#define LED_PIN 16

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire1(D4);
OneWire oneWire2(D6);
OneWire oneWire3(D7);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);
DallasTemperature sensors3(&oneWire3);

// Assign address manually. The addresses below will need to be changed
// to valid device addresses on your bus. Device address can be retrieved
// by using either oneWire.search(deviceAddress) or individually via
// sensors.getAddress(deviceAddress, index)

/*
  // Át kell alakítani majd mindent, hogy ne ezeket használják
  DeviceAddress insideThermometer = { 0x28, 0x8C, 0x5C, 0x08, 0x00, 0x00, 0x80, 0xAD };
  DeviceAddress outsideThermometer = { 0x28, 0x59, 0x9B, 0x27, 0x00, 0x00, 0x80, 0x1B };


  unsigned char thermometerAddress[8] = { 0x28, 0x8C, 0x5C, 0x08, 0x00, 0x00, 0x80, 0xAD }; */

const int NumberofThermometers = 5;

unsigned char thermometerAddress[NumberofThermometers][8] = {
    {0x28, 0x8C, 0x5C, 0x08, 0x00, 0x00, 0x80, 0xAD}, // Thermometer01 - 0
    {0x28, 0x59, 0x9B, 0x27, 0x00, 0x00, 0x80, 0x1B}, // Thermometer02 - 1
    {0x28, 0x37, 0xA4, 0x27, 0x00, 0x00, 0x80, 0x20}, // Thermometer03 - 2

    {0x28, 0xC8, 0x5A, 0x08, 0x00, 0x00, 0x80, 0x98}, // Thermometer04 - 3 - oneWire2

    {0x28, 0xFF, 0xFF, 0x02, 0xB4, 0x16, 0x05, 0xDD} // Thermometer05 - 4 - oneWire3
};

char *thermometerName[NumberofThermometers] = {
    "KeletiKozepLent", // Thermometer01 - 0 - D4
    "KeletiKozepFent", // Thermometer02 - 1
    "KeletiVege",      // Thermometer03 - 2

    "Nyugati", // Thermometer04 - 3  - oneWire2 - D6

    "KisFolia" // Thermometer05 - 4  - oneWire3 - D7
};

float temperature[NumberofThermometers];

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "key";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SSWID";
char pass[] = "Passworld";

BlynkTimer timer;

short int warningTemperature;
bool ReadErrorNotification;

// This function will be called every time Slider Widget
// in Blynk app writes values to the Virtual Pin V50
BLYNK_WRITE(V50)
{
  warningTemperature = param.asInt(); // assigning incoming value from pin V50 to a variable
  // process received value
}
BLYNK_WRITE(V51)
{
  ReadErrorNotification = param.asInt(); // assigning incoming value from pin V51 to a variable
  // process received value
}
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V50); // will cause BLYNK_WRITE(V50) to be executed
  Blynk.syncVirtual(V51); // will cause BLYNK_WRITE(V51) to be executed
}

//-------------------------------------------------------------------------------
// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void temperatureSensor()
{
  Serial.println("   .   ");
  sensors1.requestTemperatures();
  sensors2.requestTemperatures();
  sensors3.requestTemperatures();
  delay(1200);

  /* Old solution
    Serial.print(thermometerName[0]);
    sendTemperature(thermometerAddress[0], 1);

    Serial.print(thermometerName[1]);
    sendTemperature(thermometerAddress[1], 2); */

  for (size_t i = 0; i < NumberofThermometers; i++)
  {
    if ((i != 0) && (i % 4 == 0))
    {
      delay(1200);
    }

    Serial.print(thermometerName[i]);
    Serial.print(": ");
    sendTemperature(thermometerAddress[i], i);
  }
}

void temperatureWarning()
{
  Serial.println("   .   ");
  Serial.print("Low temperature limit: ");
  Serial.println(warningTemperature);

  String notificationLowTemperature;
  String notificationReadError;

  for (size_t i = 0; i < NumberofThermometers; i++)
  {
    if (temperature[i] <= -99)
    {
      Serial.print("Warning: ");
      Serial.print(thermometerName[i]);
      Serial.print(" - TempC: ");
      Serial.println(temperature[i]);

      notificationReadError = notificationReadError + String(thermometerName[i]) + ", ";
    }
    else if (temperature[i] <= warningTemperature && temperature[i] > -99)
    {
      Serial.print("Warning: ");
      Serial.print(thermometerName[i]);
      Serial.print(" - TempC: ");
      Serial.println(temperature[i]);

      notificationLowTemperature = notificationLowTemperature + String(thermometerName[i]) + ", ";
    }
  }

  Serial.print(" Notifocation status: ");
  Serial.print(ReadErrorNotification);
  if (notificationReadError != NULL && ReadErrorNotification < 2 && 0 < ReadErrorNotification)
  {
    notificationReadError = "Nem sikerült mérni ezekkel a hőmérőkkel: " + notificationReadError;
    Serial.print(", Delay... ");
    delay(6000);
    Blynk.notify(notificationReadError);
    Serial.println("Read Error Notification sent");
  }

  if (notificationLowTemperature != NULL)
  {
    notificationLowTemperature = "Alacsony hőméréséklet az alábbi hőmérőknél: " + notificationLowTemperature;
    Serial.print(", Delay... ");
    delay(6000);
    Blynk.notify(notificationLowTemperature);
    Serial.println("Low Temperature Notification sent");
  }
}

// Function to messure and send data
void sendTemperature(DeviceAddress DeviceAddress, int pn)
{
  float t = TemperatureRead(DeviceAddress, pn);
  temperature[pn] = t;
  if (t > -99)
  {
    Serial.print("Sending data... ");
    Blynk.virtualWrite(pn, t);
    Serial.println("End");
  }
}

// function to read the temperature for a device
float TemperatureRead(DeviceAddress deviceAddress, int pn)
{
  float tempC = -99;

  if (pn == 3)
  {
    tempC = sensors2.getTempC(deviceAddress);
  }
  else if (pn == 4)
  {
    tempC = sensors3.getTempC(deviceAddress);
  }
  else
  {
    tempC = sensors1.getTempC(deviceAddress);
  }

  if (tempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: Could not read temperature data");
    tempC = -99;

    return tempC;
  }
  Serial.print(" ");
  Serial.print(tempC);
  Serial.print(" °C, ");
  return tempC;
}

// Function to blynk the microcontroller's LED
void blynkLife()
{
  
  digitalWrite(LED_PIN, LOW); // Turn ON
  delay(600);
  digitalWrite(LED_PIN, HIGH); // Turn OFF
  
}

/*------------------------------------------------Not used------------------------------------------------------

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)  {
  float tempC = sensors1.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}


// function to print a device address
void printAddress(DeviceAddress deviceAddress)  {
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}


// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.print(sensors1.getResolution(deviceAddress));
  Serial.println();
}

------------------------------------------------Not used------------------------------------------------------*/

void setup()
{
  // Set CPU clock to 160MHz
  system_update_cpu_freq(160);

  // Debug console
  Serial.begin(115200);

   pinMode(LED_PIN, OUTPUT);    // Initialize the LED_PIN pin as an output
   digitalWrite(LED_PIN, HIGH); // Turn OFF

  delay(1200);
  Serial.println(" ");
  Serial.println("   .   ");
  Serial.println("NodeMCU Temperature sensor's message: I am alive!!!!");

  Blynk.begin(auth, ssid, pass);

  // Setup a function to be called every second
  timer.setInterval(60000L, temperatureSensor);

  timer.setInterval(60000L, temperatureWarning);

  timer.setInterval(5000L, blynkLife); // Mérési időköz milliszekundumban
                                       // 1000L    = 1 másodperc
                                       // 10000L   = 10 másodperc
                                       // 60000L   = 1 perc
                                       // 600000L  = 10 perc
                                       // 3600000L = 1 óra

  // Start up the Blynk library
  sensors1.begin();
  sensors2.begin();
  sensors3.begin();

  Serial.println();
  /*
    // locate devices on the bus
    Serial.print("Locating devices...");
    Serial.print("Found ");
    Serial.print(sensors1.getDeviceCount(), DEC);
    Serial.println(" devices.");

    // report parasite power requirements
    Serial.print("Parasite power is: ");
    if (sensors1.isParasitePowerMode()) Serial.println("ON");
    else Serial.println("OFF");


    // search() looks for the next device. Returns 1 if a new address has been
    // returned. A zero might mean that the bus is shorted, there are no devices,
    // or you have already retrieved all of them. It might be a good idea to
    // check the CRC to make sure you didn't get garbage. The order is
    // deterministic. You will always get the same devices in the same order
    //
    // Must be called before search()
    oneWire.reset_search();

    for (size_t i = 0; i < NumberofThermometers; i++)
    {
    if (!oneWire.search(thermometerAddress[i]))
    {
      Serial.print("Unable to find address for ");
      Serial.println(thermometerName[i]);
    }
    }
  */

 /*
  Serial.println("   .   ");
  for (size_t i = 0; i < NumberofThermometers; i++)
  {
    if (sensors1.getTempC(thermometerAddress[i]) == DEVICE_DISCONNECTED_C)
    {
      Serial.print("Unable to find address for ");
      Serial.println(thermometerName[i]);
    }
  }
  */
  Serial.println(" ");
  Serial.println("*************************************************************");

  /*
    // show the addresses we found on the bus
    Serial.print("Device 0 Address: ");
    printAddress(insideThermometer);
    Serial.println();

    Serial.print("Device 1 Address: ");
    printAddress(outsideThermometer);
    Serial.println();
  */

  // set the resolution
  // Serial.println("Device Resolutions: ");
  for (size_t i = 0; i < NumberofThermometers; i++)
  {
    sensors1.setResolution(thermometerAddress[i], TEMPERATURE_PRECISION);

    /*
    Serial.print(thermometerName[i]);
    Serial.print(": ");
    Serial.println(sensors1.getResolution(thermometerAddress[i]), DEC); */
  }
  // Serial.println();

  temperatureSensor();
}

void loop()
{
  Blynk.run();
  timer.run();
}
