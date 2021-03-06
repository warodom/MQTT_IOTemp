#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 13
#define LightRoom 12
#define Sprinkler 14
#define Touch 2

bool touch_buffer = 0, touch_status = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char* ssid = "Pi_AP";
const char* password = "Raspberry"; 
char* server = "192.168.42.1";

unsigned long temp_count;
char temp[10];

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  int i = 0; 
  int select_color = 0;
  char message_buff[100];
  for(i = 0; i < length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  
  Serial.print("Subscribe ");
  Serial.write(topic);
  Serial.print(" : ");
  //Serial.write(payload,length);
  Serial.println(msgString);
  if(String(topic) == "light") {
    if(msgString == "0") digitalWrite(LightRoom, LOW);
    else digitalWrite(LightRoom, HIGH);
  }
  else if(String(topic) == "sprinkler") {
    if(msgString == "0") digitalWrite(Sprinkler, LOW);
    else digitalWrite(Sprinkler, HIGH);
  }
} 

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);
 
void setup()
{ 
  pinMode(LightRoom, OUTPUT); digitalWrite(LightRoom, LOW);
  pinMode(Sprinkler, OUTPUT); digitalWrite(Sprinkler, LOW);
  pinMode(Touch, INPUT);
  Serial.begin(115200);
  sensors.begin();
  delay(10);
  Serial.println();

  WiFi.begin(ssid, password);
 
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 10)) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
  }
 
  if (client.connect("IOTemp")) {
    client.subscribe("light");
    client.subscribe("sprinkler");
  }
  
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.print("Temp : ");
  Serial.println(String(tempC));
  String(tempC).toCharArray(temp,5);
  client.publish("temp",temp);
  
}
 
void loop()
{
  if(temp_count > 655000) {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    Serial.print("Temp : ");
    Serial.println(String(tempC));
    String(tempC).toCharArray(temp,5);
    client.publish("temp",temp);
    temp_count = 0;
  }
  else temp_count++;
  if((digitalRead(Touch) == 0) & (touch_buffer == 0)) {
    if(touch_status == 0) {
      client.publish("light","1");
      touch_status = 1;
    }
    else if(touch_status == 1) {
      client.publish("light","0");
      touch_status = 0;
    }
    touch_buffer = 1;
  }
  else if((digitalRead(Touch) == 1) & (touch_buffer == 1)) {
    //client.publish("light","0");
    touch_buffer = 0;
  }
  client.loop();
}
