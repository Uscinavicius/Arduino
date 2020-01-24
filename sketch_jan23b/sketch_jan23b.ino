#include <ESP8266WiFi.h>

const char* ssid="Bone";
const char* password = "123456789";
WiFiServer server(80);

int ledPin = 13;

//NeoPixel Setup
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN            12
#define NUMPIXELS      1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  //Pin Setup
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);

  //Serial Monitor setup
  Serial.begin(115200);
  delay(10);

  //Wifi Prompt
  Serial.println();
  Serial.print("Wifi connecting to ");
  Serial.println( ssid );

  WiFi.begin(ssid,password);

  Serial.println();
  Serial.print("Connecting");

  while( WiFi.status() != WL_CONNECTED ){
      delay(500);
      Serial.print(".");        
  }

  digitalWrite( ledPin , HIGH);
  Serial.println();

  Serial.println("Wifi Connected Success!");
  Serial.print("NodeMCU IP Address : ");
  Serial.println(WiFi.localIP() );

  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  //NeoPixel setup 
  pixels.begin();
}

void loop() {
    // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

    // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
 
  // Match the request
  if (request.indexOf("/LED=") != -1)  {
  String x = request.substring(request.indexOf("=") + 1);
  Serial.println(x);
  int y = x.toInt();
  String j = 
  pixels.setPixelColor(0, pixels.Color(0,y,0)); // Moderately bright green color. 
  pixels.show();
  }
  
  int value = LOW;
  if (request.indexOf("/LED=ON") != -1)  {
//    Serial.print("/LED=ON -- ");
//    Serial.print(request);
//    Serial.println(request.indexOf("/LED=ON"));
//    pixels.setPixelColor(0, pixels.Color(0,100,0)); // Moderately bright green color.
//    pixels.show();
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1)  {
//    Serial.print("/LED=OFF -- ");
//    Serial.print(request);
//    Serial.println(request.indexOf("/LED=ON"));
//
//    pixels.setPixelColor(0, pixels.Color(0,0,0)); // Moderately bright green color.
//    pixels.show();
    value = LOW;
  }

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("Led is now: ");
 
  if(value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>On </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Off </button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");

  if(WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid,password);
    Serial.println();
    Serial.print("Connecting");
  }
  while( WiFi.status() != WL_CONNECTED ){
      delay(500);
      Serial.print(".");        
  }

}
