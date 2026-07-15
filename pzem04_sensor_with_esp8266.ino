#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"

#define PZEM_RX_PIN D4
#define PZEM_TX_PIN D5

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);


// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);


// Enter network credentials:
const char* ssid     = "Xiaomi13T";
const char* password = "12345678";

// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbylFqq8uddEQq1i7tpOkd4KV2NwidQvJhTj1YsganWZj5MIBgMlxnsPXoSoegNpUrqq";

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;

String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;





void setup() {
  Serial.begin(115200);        
  Serial.println('\n');
  Serial.println("System initialized");
 lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("PZEM Energy");
  lcd.setCursor(0, 1);
  lcd.print("Meter");
  delay(2000);

  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to");
  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("Connected");
         lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected");
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object


  
}


void loop() {
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();
  
  if (isnan(voltage) || isnan(current) || isnan(power) ||
      isnan(energy) || isnan(frequency) || isnan(pf)) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PZEM Error");
    Serial.println("Error reading PZEM");
    delay(2000);
    return;
  }
updatesheet(voltage,current, power ,energy,frequency,pf);
  // Serial Monitor Output
  Serial.print("Voltage: "); Serial.print(voltage); Serial.println(" V");
  Serial.print("Current: "); Serial.print(current); Serial.println(" A");
  Serial.print("Power: "); Serial.print(power); Serial.println(" W");
  Serial.print("Energy: "); Serial.print(energy, 3); Serial.println(" kWh");
  Serial.print("Frequency: "); Serial.print(frequency, 1); Serial.println(" Hz");
  Serial.print("PF: "); Serial.println(pf);
  Serial.println();


 // ===== Screen 1 =====
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("V:");
    lcd.print(voltage,1);
    lcd.print("V");
    lcd.setCursor(9,0);
    lcd.print("I:");
    lcd.print(current,1);
    lcd.print("A");
    lcd.setCursor(0,1);
    lcd.print("P:");
    lcd.print(power,1);
    lcd.print("W");
    lcd.setCursor(9,1);
    lcd.print("PF:");
    lcd.print(pf,2);

    delay(6000);

    // ===== Screen 2 =====
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("E:");
    lcd.print(energy,3);
    lcd.print("kWh");

    lcd.setCursor(0,1);
    lcd.print("f:");
    lcd.print(frequency,1);
    lcd.print("Hz");

    delay(6000);
     
  }



void updatesheet(float voltage,float current, float power ,float energy,float frequency,float pf)

{


  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
  }
  
  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + voltage + "," + current + "," + power+ "," + energy + "," + frequency +","+pf+ "\"}";
  
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }

  // a delay of several seconds is required before publishing again    
  //delay(5000);
  }
  
