#include <PubSubClient.h>
#include <WiFi.h>                                                                             
#include "DHT.h"   
#include <Servo_ESP32.h>
#include <SPI.h>
#include <MFRC522.h>  
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
                                                                                                    


const int DHTPIN = 4;      
const int DHTTYPE = DHT11;

#define fan_1 14
#define fan_2 21
#define servoPin_4  12 // DOOR

#define LED_PIN_1   15       //LED NEO THANG
#define NUMPIXELS_1   30

// SCK 18, MISO 19, MOSI 21
#define RST_PIN   22         
#define SS_PIN   5 //chân SDA

Servo_ESP32 servo4;
MFRC522 mfrc522(SS_PIN, RST_PIN); 

int UID[4],i;
int ID1[4] = {27, 123, 25, 55};
int ID2[4] = {181, 180, 136, 172}; 

Adafruit_NeoPixel pixels_1(NUMPIXELS_1, LED_PIN_1, NEO_GRB + NEO_KHZ800);

DHT dht(DHTPIN, DHTTYPE);


const char* ssid = "Ken";
const char* password = "88888888"; 
const char* mqtt_server = "192.168.225.66";
const unsigned int mqtt_port = 1883;


long now = millis();
long lastMeasure = 0;


void setup_wifi() 
{
  delay(10);
  Serial.println();
  Serial.print("Đang Kết Nối Tới ");
  Serial.println(ssid);  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

WiFiClient wifi;
PubSubClient client(mqtt_server,mqtt_port,callback,wifi);

void callback(char* topic, byte* payload, unsigned int length)
{
   String string;
   Serial.print("Message from [");
   Serial.print("]:");
   Serial.print(topic);
   Serial.print("] ");

  if(!strcmp(topic,"Fan_1"))
  {
    for (int i = 0; i < length; i++) 
    {
      Serial.print((char)payload[i]);
    } 
      Serial.println();
    if ((char)payload[0] == '2') 
    {  
       digitalWrite(fan_1,HIGH);
    }
    else
    {
      digitalWrite(fan_1,LOW);
    }
            
   }

   if(!strcmp(topic,"Light_1"))
   {
      for (int i = 0; i < length; i++) 
      {
        Serial.print((char)payload[i]);
      } 
      Serial.println();
      
      if ((char)payload[0] == '1') 
      {  
        for(int i=0; i<NUMPIXELS_1; i++) 
        { 
           pixels_1.setPixelColor(i, pixels_1.Color(255, 255, 255));
           pixels_1.show(); 
           delay(50);  
        } 
      }
       
      else 
      {
        for(int i=30; i>=0; i--) 
        { 
          pixels_1.setPixelColor(i, pixels_1.Color(0, 0, 0));
          pixels_1.show();
          delay(50);   
        }
       
      }        
    }

    else if(strstr(topic,"servo_4"))   
    {
      for (int i = 0; i < length; i++) 
      {
        string+=((char)payload[i]);
      }
      Serial.print(" ");
      int resultado_4 = string.toInt();   
      int pos_4 = map(resultado_4, 1, 100, 0, 180);
      Serial.println(pos_4);
      servo4.write(pos_4);
      delay(15);
    }

    else if(!strcmp(topic,"Fan_2"))
    {
      for (int i = 0; i < length; i++) 
      {
        Serial.print((char)payload[i]);
      } 
        Serial.println();
      if ((char)payload[0] == '2') 
      {  
         digitalWrite(fan_2,HIGH);
      }
      else
      {
        digitalWrite(fan_2,LOW);
      }
              
     }


}
  
void setup() 
{
  Serial.begin(115200);
  dht.begin();
  SPI.begin();    
  mfrc522.PCD_Init();
  pinMode(fan_1,OUTPUT);
  pinMode(fan_2,OUTPUT);
  pixels_1.begin();
  servo4.attach(servoPin_4);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);      
}


void reconnect()
{
 while (!client.connected()) 
 {
    Serial.print(" Đang Kết Nối MQTT...");
 
    if (client.connect("ESP32 DEVKIT V1 Client")) 
    {
       Serial.println("Kết Nối Thành Công");
       Serial.println("MQTT Đã Kết Nối");
       client.subscribe("Fan_1");
       client.subscribe("Fan_2");
       client.subscribe("Light_1");
       client.subscribe("servo_4");
       delay(3000);
    }
    else
    {
      Serial.print("Kết Nối MQTT Thất Bại ");
      Serial.println("Thử Lại Trong 5 Giây");
      delay(5000);
 
    }
  }
}


void loop() 
{
      
   float h = dht.readHumidity();    
   float t = dht.readTemperature();
   char Nhiet_do[8];
   dtostrf(t, 6, 2, Nhiet_do);
   char Do_am[8];
   dtostrf(h, 6, 2, Do_am);       
   char string [9];
  

    if (!client.connected())
    {
      reconnect();
    }
    if(!client.loop())
      client.connect("ESP32 DEVKIT V1 Client");

    now = millis();
    if (now - lastMeasure >=  1000)
    {
     lastMeasure = now;
     client.publish("Tem", Nhiet_do);
     client.publish("Hum", Do_am);
     if(t>33)
     {
        digitalWrite (fan_1, HIGH);
     }
     if(t<31)
     {
        digitalWrite (fan_1, LOW);
     }

    }
}
