// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include<stdio.h>
#include<string.h>
#include "DHT.h"
#define DEBUG false

#include <SoftwareSerial.h>

#define BAUD_RATE 115200
#define A9_BAUD_RATE 9600

/***********************************/
#define A9G_PON     16  //ESP12 GPIO16 A9/A9G POWON
#define A9G_POFF    15  //ESP12 GPIO15 A9/A9G POWOFF
#define A9G_WAKE    13  //ESP12 GPIO13 A9/A9G WAKE
#define A9G_LOWP    2  //ESP12 GPIO2 A9/A9G ENTER LOW POWER MODULE

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);


char writeAPIKey[] = "HUINCSGESVB7O13F";    // Change to your channel write API key.
char channelID[] ="859916";

void handleInterrupt();
int A9GPOWERON();
void A9GMQTTCONNECT();
String sendData(String command, const int timeout, boolean debug);
/***********************************/
SoftwareSerial swSer(14, 12, false, 256);
void setup() {
  Serial.begin(BAUD_RATE);
  swSer.begin(A9_BAUD_RATE);

  for (char ch = ' '; ch <= 'z'; ch++) {
    swSer.write(ch);
  }
  swSer.println("");

dht.begin();

/************************************/
  pinMode(A9G_PON, OUTPUT);//LOW LEVEL ACTIVE
  pinMode(A9G_POFF, OUTPUT);//HIGH LEVEL ACTIVE
  pinMode(A9G_LOWP, OUTPUT);//LOW LEVEL ACTIVE

  digitalWrite(A9G_PON, HIGH); 
  digitalWrite(A9G_POFF, LOW); 
  digitalWrite(A9G_LOWP, HIGH); 

  Serial.println("After 2s A9G will POWER ON.");
  delay(2000);
  if(A9GPOWERON()==1)
  {
       Serial.println("A9G POWER ON.");
  }

  delay(5000);

  A9GMQTTCONNECT();
  delay(5000);


}

void loop() {

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  String topic ="channels/" + String( channelID ) + "/publish/"+String(writeAPIKey);
  Serial.println(topic);
  String payload = String("field1=" + String(t) + "&field2=" + String(h));
  Serial.println(payload);
   String staflag ="&status=MQTTPUBLISH";
   Serial.println(staflag);
    String ATCMD="AT+MQTTPUB=";
   String cammand=ATCMD+"\""+ topic + "\""+ ","+"\""+ payload + staflag + "\""+",0,0,0";
   Serial.println(cammand);

// sendData("AT+MQTTPUB=\"channels/859916/publish/HUINCSGESVB7O13F\",\"field1=100&field2=55&status=MQTTPUBLISH\",0,0,0",1000,DEBUG);

 sendData(cammand,1000,DEBUG);

  delay(60000);


  }



void handleInterrupt() {
  Serial.println("An interrupt has occurred.");
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";    
    swSer.println(command); 
    long int time = millis();   
    while( (time+timeout) > millis())
    {
      while(swSer.available())
      {       
        char c = swSer.read(); 
        response+=c;
      }  
    }    
    if(debug)
    {
      Serial.print(response);
    }    
    return response;
}

int A9GPOWERON()
{
      digitalWrite(A9G_PON, LOW);
      delay(3000);
      digitalWrite(A9G_PON, HIGH);
      delay(5000);
      String msg = String("");
      msg=sendData("AT",1000,DEBUG);
      if( msg.indexOf("OK") >= 0 ){
          Serial.println("GET OK");
          return 1;
      }
      else {
         Serial.println("NOT GET OK");
         return 0;
      }
}

int A9GPOWEROFF()
{
      digitalWrite(A9G_POFF, HIGH);
      delay(3000);
      digitalWrite(A9G_POFF, LOW);
      delay(5000);
      String msg = String("");
      msg=sendData("AT",1000,DEBUG);
      if( msg.indexOf("OK") >= 0 ){
          Serial.println("GET OK");
          return 1;
      }
      else {
          Serial.println("NOT GET OK");
          return 0;
      }
}

int A9GENTERLOWPOWER()
{
      String msg = String("");
      msg=sendData("AT+SLEEP=1",1000,DEBUG);
      if( msg.indexOf("OK") >= 0 ){
          digitalWrite(A9G_LOWP, LOW);
          return 1;
      }
      else {
         return 0;
      }
}

void A9GMQTTCONNECT()
{
  sendData("AT+CGATT=1",1000,DEBUG);
  delay(1000);
  sendData("AT+CGDCONT=1,\"IP\",\"CMNET\"",1000,DEBUG);
  delay(1000);
  sendData("AT+CGACT=1,1",1000,DEBUG);
  delay(1000);
  sendData("AT+MQTTDISCONN",1000,DEBUG);
  delay(1000);
  String msg=sendData("AT+MQTTCONN=\"mqtt.thingspeak.com\",1883,\"DHT11\",120,1,\"ESP12SA9G\",\"BQ64USNQPF4GEPSJ\"",1000,DEBUG);
  if( msg.indexOf("OK") >= 0 ){
      Serial.println("A9G CONNECT to the ThingSpeak");
      }
 
 delay(2000);
 }
