#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define Heater       13//Heater pin
#define Pump         19//Pump Pin
#define Sensor       4//Sensor Pin
#define Pumptresh    70//treshhold of the pump to power on
#define tfull        74   //temp before it start to regulate
#define tregulate    78//temp to maintain
#define ONE_WIRE_BUS 4   //pin for one wire bus
#define LED          21   //LED Pin

#define freq          3
#define ledChannel    0
#define resolution    8
float t1=0;
float t2=0;
int Increase = 1  ;       //increment value
int Decrease = 15  ;        //decrement value
float pwm      = 256 ;       //pwm value
float DC       =100 ;         //Duty Cycle Value

///MQTT////
const char* ssid = "PLDTHOMEDSL505CA";
const char* password =  "PLDTWIFI505C2";
const char* mqttServer = "m12.cloudmqtt.com";
const int mqttPort = 17013;
const char* mqttUser = "jmdxdybd";
const char* mqttPassword = "5Tj-ehZ_Ieq_";
////////// MQTT Variables
String buff;
String buff2r;
int data = 0;
int state = 1;


char byteRead;
String sub1;
String subres1;
char subres;
char incomingByte;
String incomingByte_str;
////

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); 
LiquidCrystal_I2C lcd(0x3F,16,2); 
WiFiClient espClient;
PubSubClient client(espClient);
/*void ledcAnalogWrite(uint8_t ledChannel, uint32_t value) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t pwm = (255 / 255) * min(value, 255);

  // write duty to LEDC
  ledcWrite(ledChannel, pwm);
}*/
void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    sub1=sub1+((char)payload[i]);
  }
 
  Serial.println();
  sub1=subres1;
  sub1="";
  Serial.println("-----------------------");
 
}

void setup()
{

 // pinMode(Heater, OUTPUT);
 pinMode(Pump, OUTPUT);
 pinMode(Sensor, INPUT);
 pinMode(LED, OUTPUT);
 //pinMode(Heater, OUTPUT); 
///initialization  
//lcd.init(); // initialize the lcd
lcd.init();
Serial.begin(115200);
sensors.begin();
 WiFi.begin(ssid, password);
ledcSetup(ledChannel, freq, resolution);
ledcAttachPin(Heater, ledChannel);

lcd.backlight();
lcd.setCursor(1,0);
lcd.print("Connect... WiFi");
    
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    
  }

Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
   while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
    client.subscribe("esp/test");
  }
  

   Welcome();
}
 
void loop() {

  sensors.requestTemperatures();
  t1=sensors.getTempCByIndex(0);
  t2=sensors.getTempCByIndex(1);
  client.loop();
  
  regulate();
  publishdata();
  view();
  
 
}


void publishdata() {

buff=String (t1);
char CT[5]={buff.charAt(0),buff.charAt(1),buff.charAt(2),buff.charAt(3),buff.charAt(4)};
CT[5]='\0';

buff2r=String (t2);
char KhT[5];
for(int a=0; a<buff2r.length(); a++)
{
   KhT[a] = buff2r.charAt(a);
}
KhT[5]='\0';



//Serial.println(KhT);
 client.publish("esp32/test/kt", KhT);
 client.publish("esp32/test/ct", CT);

// String dc_buffer=String(DC);
//char duty_cycle[3]={dc_buffer.charAt(0),dc_buffer.charAt(1),dc_buffer.charAt(2)};
//duty_cycle[3]='\0';
static char duty[3];
dtostrf(DC,3, 0, duty);
client.publish("esp32/test/dc", duty);

// Serial.println(CT);
  
  delay(400);
}


void Welcome(){
//lcd.backlight();
lcd.setCursor(0,0);
lcd.print("Distiller Box   ");
lcd.setCursor(0,1);
lcd.print("Initializing...");
delay(1000);
}


void regulate(){
  

  if (t1< tfull){
    pwm=256;
  }
  if (t1< tregulate && t1>= tfull){
    pwm=pwm+Increase;
    if (pwm>=256) {
      pwm=256;
    }
  }
  if (t1> tregulate){
    pwm=pwm-Decrease;
    if(pwm<=77){
      pwm=77;
    }
  }

  if (t2>=Pumptresh){
    digitalWrite(Pump, HIGH);
     client.publish("esp32/test/pump", "1");
  }
  else{
    digitalWrite(Pump, LOW);
     client.publish("esp32/test/pump", "0");
  }
  
  ledcWrite(ledChannel, pwm);
  Serial.print("pwm: ");
  Serial.println(pwm);
  Serial.print("dc: ");
  Serial.println(DC);
}


void view(){
  //Serial.println(millis());
  lcd.backlight();
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Steam:");
  lcd.setCursor(0, 1);
  lcd.print("Tank :");
  lcd.setCursor(6, 0);
  lcd.print(sensors.getTempCByIndex(0),0);
  lcd.print("C ");
  lcd.setCursor(6, 1);
  lcd.print(sensors.getTempCByIndex(1),0);
  lcd.print("C ");
  lcd.setCursor(10, 0);
  lcd.print("H:");

  DC=(pwm/256)*100;
  lcd.print(DC,0);
  lcd.print("% ");
  
  lcd.setCursor(10, 1);
  lcd.print("P:");
  if(digitalRead(Pump)==HIGH){
   lcd.print("ON "); 
  }
  else{
    lcd.print("OFF");
  }
  //Serial.print(millis());
  //Serial.println("-");
}
