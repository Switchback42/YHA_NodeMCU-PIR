/*
YHA_PIR_Node_template
LED on D6 through 330 Ohm resistor
Pir Motion Sensor on D7 (PIR Sensor / HC-SR501)
*/ 

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FirebaseArduino.h>

#define FIREBASE_HOST "YOUR-FIREBASE-PROJECT.firebaseio.com" //Use dashes, not underscores. ex. my-project-name.firebaseio.com
#define FIREBASE_AUTH "YOUR FIREBASE AUTHORIZATION KEY" //This is the Legacy Credential. Proj Settings/Service Accounts/Database Secret

#define WIFI_SSID "YOUR SSID" //Change as needed
#define WIFI_PASSWORD "YOUR PASSWORD" //Change as needed
#define ESP8266_SSID_PREFIX "YOUR FIRST NAME" //this is the first part of the SSID that your esp8266 will broadcast
                                              //adding your name to it will help you ID it more quickly on a network.

#define MOTION_PIN D7
#define LED_PIN D6

ESP8266WebServer server;
//password must be at least 8 characters
const char WiFiAPPSK[] = "YOUR ESP8266 PASSWORD"; //this allows you to log in to your PLC via a browser

//This stuff is for your web brower 
const char INDEX_HTML[] = 
  "<!DOCTYPE HTML>"
  "<html>"
  "<head>"
  "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
  "<title>ESP8266</title>"
  "<style>"
    "body { background-color: #43efe4; font-family: Arial, Helvetica, Sans-Serif; Color: White; }"
  "</style>"
  "</head>"
  "<body>"
  "<h1>ESP8266 Direct Access Demo</h1>"
  "<button onclick='senseMotion()'>Trigger Motion</button>"
  "<button onclick='toggleLED()'>Toggle LED</button>"
  "<script>"
    "function senseMotion(){"
      "fetch('/senseMotion').then(stream=>stream.text()).then(text=>console.log(text))"
    "}"
    "function toggleLED(){"
      "fetch('/toggleLED').then(stream=>stream.text()).then(text=>console.log(text))"
    "}"
  "</script>"
  "</body>"
  "</html>";

void setup()
{
  Serial.begin(9600); //9600 is the default, but my computer is set to 115200
  setupWiFi();
 
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); //the builtin LED will be turned ON when set to LOW
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(MOTION_PIN, INPUT);
  digitalWrite(MOTION_PIN, LOW); 
 
  server.on("/",sendIndex);
  server.on("/senseMotion", senseMotion);
  server.on("/toggleLED", toggleLED);
  server.begin();
  Serial.println("");
  Serial.println("");
  Serial.print("Server running on http://192.168.4.1/");
  Serial.println("");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setInt("share/motion", LOW); //initializing the value of the Firebase variable "share/motion" to LOW or OFF
  if(Firebase.failed()){
    Serial.print("Firebase connection failed!");
    Serial.println("");
    Serial.print(Firebase.error());
  } else {
    Serial.print("Firebase connection success!");
  }
  Serial.println("");
}

int timePassed (int time) {
  int diff = 0;
  if (millis() <= time) {
    diff = (69666 - time) + millis();
  } else {
    diff = millis() - time;
  }
  return diff;
}

int LED_Pin = LOW;
int motion = LOW;
int checkFirebaseTime = 0;

void loop()
{
  if(timePassed (checkFirebaseTime) >= 1000){  
    int motion = digitalRead(MOTION_PIN); 
    Firebase.setInt("share/motion", motion);
    if(motion == HIGH){
      senseMotion(); //this is the function I created to light the LED, if motion is detected
  }else{
    digitalWrite(LED_PIN, LOW);
    Serial.println("no motion detected");
    }
  checkFirebaseTime = millis();
  }
  server.handleClient();
}

void sendIndex(){
  server.send(200,"text/html",INDEX_HTML);  
}

void toggleLED(){
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  //server.send(204,"");
  server.send(200,"text/plain","Toggle LED!\n");
}

//From the loop:  This function gets called if the value of motion is equal to HIGH
//The value of motion is being updated in the database with this line:  Firebase.setInt("share/motion", motion);
void senseMotion(){
  //server.send(204,"");
  server.send(200,"text/plain","Motion Triggered!\n");
  digitalWrite(LED_PIN, HIGH); 
  Serial.println("Motion detected");
}

void setupWiFi()
{
  WiFi.mode(WIFI_AP_STA);
  //Set up Access Point
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.println();
  Serial.print("Local IP: ");
  Serial.println();
  Serial.println(WiFi.localIP());
  
  // Set up Station
  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = ESP8266_SSID_PREFIX + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);
  //AP_NameChar is the SSID for the Wireless Access Point
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}
