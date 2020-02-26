#include <ArduinoOSC.h>

//Settings
#define WIFI_SSID "NETGEAR75"
#define WIFI_PASSWORD "rusticgiant613"
#define LISTEN_PORT 5000

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include <EasyButton.h>


WiFiUDP Udp;

//GPIO pin mappings on the ESP8266. Constants for convenience. 
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

//Absolute
float da = 0;
float ta = 0;
float aa = 0;
float ba = 0;
float ga = 0;

//Relative
float dr = 0;
float tr = 0;
float ar = 0;
float br = 0;
float gr = 0;


//define the button.
EasyButton button(D3);



int idx = 0; //used for chosing the wave and the led port.
float[] waves = {da, ta, aa, ba, ga}; 
float[] pins = null; // to be filled once I have the wired the leds.

void onPressed(){
  Serial.println("Button press detected!");
  idx++; 
  
}


void setup() {
  pinMode(D2, OUTPUT);
  
  setupWifi();

  button.begin(); //start the listener for the button press.
  button.onPressed(onPressed); //set callback method to the button press.
}



float getAveragePSD(OSCMessage &msg) {
  if (msg.size()==1){
    return msg.getFloat(0); //Combined average can be sent by Muse Monitor
  } else {
    return (msg.getFloat(0)+msg.getFloat(1)+msg.getFloat(2)+msg.getFloat(3))/4.0; //TP9, AF7, AF8, TP10
  }
}

void delta(OSCMessage &msg) {
  da = getAveragePSD(msg);
  //Calculate relative value
  dr = (pow(10,da) / (pow(10,da) + pow(10,ta) + pow(10,aa) + pow(10,ba) + pow(10,ga)));
  //map to screen height
}

void theta(OSCMessage &msg) {
  ta = getAveragePSD(msg);
  tr = (pow(10,ta) / (pow(10,da) + pow(10,ta) + pow(10,aa) + pow(10,ba) + pow(10,ga)));
}

void alpha(OSCMessage &msg) {
  aa = getAveragePSD(msg);
  ar = (pow(10,aa) / (pow(10,da) + pow(10,ta) + pow(10,aa) + pow(10,ba) + pow(10,ga)));
}

void beta(OSCMessage &msg) {
  ba = getAveragePSD(msg);
  br = (pow(10,ba) / (pow(10,da) + pow(10,ta) + pow(10,aa) + pow(10,ba) + pow(10,ga)));
}

void gamma(OSCMessage &msg) {
  ga = getAveragePSD(msg);
  gr = (pow(10,ga) / (pow(10,da) + pow(10,ta) + pow(10,aa) + pow(10,ba) + pow(10,ga)));
}

void loop() {

  button.read(); //listen to the button taps.
  
  OSCBundle bundle;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }
    
    if (!bundle.hasError()) {
      bundle.dispatch("/muse/elements/delta_absolute", delta);
      bundle.dispatch("/muse/elements/theta_absolute", theta);
      bundle.dispatch("/muse/elements/alpha_absolute", alpha);
      bundle.dispatch("/muse/elements/beta_absolute", beta);
      bundle.dispatch("/muse/elements/gamma_absolute", gamma);

      //Serial.println(br);
      Serial.println(ar);
      //Serial.println(tr);
      //Serial.println(dr);
      //Serial.println(gr);

    }
  } 
  
    //this is the line that sets the vibration motor speed.
    //analogWrite(D2, (int)mapFloat(dr, 0.0, 1.0, 0.0, 1023.0)); // sets PWM voltage that goes into the motor.

   
    

    //Serial.println((String)mapFloat(dr, 0.0, 1.0, 0.0, 1023.0));
 }




//Configures and initiates the UDP packet flow from the MindMonitor app to the ESP8266 chip.
//WiFi.config(static ip for the esp8266, gateway's local ip, subnet)
void setupWifi(){
  Serial.begin(115200);
  Serial.println("\n\nESP Booted.");
  
  WiFi.config(IPAddress(192,168,0,123),IPAddress(192,168,0,2), IPAddress(255,255,255,0)); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Starting UDP");
    Udp.begin(LISTEN_PORT);
    Serial.print("Local port: ");
    Serial.println(Udp.localPort());
    
}




float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
