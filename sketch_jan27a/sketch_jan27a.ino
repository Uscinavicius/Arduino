#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

int pixel_size = 250;
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

const int led = 2;

void handleRoot();              // function prototypes for HTTP handlers
void handleLED();
void handleNotFound();

Adafruit_NeoPixel pixels(pixel_size, 12, NEO_GRB + NEO_KHZ800);

void setup(void){
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  pixels.begin();
  
  pinMode(led, OUTPUT);
  
  // Connect to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin("mainframe", "itisokay");
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(led,!digitalRead(led));
  }

  digitalWrite(led,HIGH);
  
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/", HTTP_GET, handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
  //server.on("/LED", HTTP_GET, handleGetLED);     // Call the 'handleRoot' function when a client requests URI "/"
  //server.on("/LED", HTTP_POST, handleLED);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  //server.on("/login", HTTP_POST, handleLogin); // Call the 'handleLogin' function when a POST request is made to URI "/login"
  server.on("/json",HTTP_POST,handleJSON);
  server.on("/rgb",HTTP_POST,handleRGB);
  server.on("/color",HTTP_GET,handleColor);
  server.on("/color",HTTP_POST,handleColorPost);
    
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop(void){
  MDNS.update();
  server.handleClient();                    // Listen for HTTP requests from clients
}

void handleRoot() {                          // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<h1>RGB Led Server</h1>");
}

//void handleGetLED() {                         // When URI / is requested, send a web page with a button to toggle the LED
//  server.send(200, "text/html", "<form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Toggle LED\"></form>");
//}
//
//void handleLED() {                          // If a POST request is made to URI /LED
//  digitalWrite(led,!digitalRead(led));      // Change the state of the LED
//  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
//  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
//}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

//void handleLogin() {                         // If a POST request is made to URI /login
//  if( ! server.hasArg("username") || ! server.hasArg("password") 
//      || server.arg("username") == NULL || server.arg("password") == NULL) { // If the POST request doesn't have username and password data
//    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
//    return;
//  }
//  if(server.arg("username") == "John Doe" && server.arg("password") == "password123") { // If both the username and the password are correct
//    server.send(200, "text/html", "<h1>Welcome, " + server.arg("username") + "!</h1><p>Login successful</p>");
//  } else {                                                                              // Username and password don't match
//    server.send(401, "text/plain", "401: Unauthorized");
//  }
//}

void handleColor(){
  server.send(200,"text/html","<html><form action=\"/color\" method=\"POST\"><input type=\"text\" name=\"red\" placeholder=\"red\"><br><input type=\"text\" name=\"green\" placeholder=\"green\"><br><input type=\"text\" name=\"blue\" placeholder=\"blue\"><br><input type=\"submit\" value=\"submit\"></form></html>");
}

void handleColorPost(){
  Serial.println(server.arg("red"));
  Serial.println(server.arg("green"));
  Serial.println(server.arg("blue"));
  int r = server.arg("red").toInt();
  int g = server.arg("green").toInt();
  int b = server.arg("blue").toInt();

  for(int i=0; i<8; i++){
  pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
  server.send(200,"text/html","<html><form action=\"/color\" method=\"POST\"><input type=\"text\" name=\"red\" placeholder=\"red\"><br><input type=\"text\" name=\"green\" placeholder=\"green\"><br><input type=\"text\" name=\"blue\" placeholder=\"blue\"><br><input type=\"submit\" value=\"submit\"></form></html>");
}

void handleJSON(){
  Serial.println(server.arg("plain"));
  String payload = server.arg("plain");
  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);
  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println(F("Parsing failed!"));
    return;
  }
  
  // Decode JSON/Extract values
  Serial.println(F("Response:"));
  Serial.println(root["sensor"].as<char*>());
  Serial.println(root["time"].as<char*>());
  Serial.println(root["data"][0].as<char*>());
  Serial.println(root["data"][1].as<char*>());
  
  server.send(303);
}

void handleRGB(){
  server.send(303);
  Serial.println(server.arg("plain"));
  String payload = server.arg("plain");
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println(F("Parsing failed!"));
    return;
  }
  Serial.println(F("Response:"));
  Serial.println(root["red"].as<char*>());
  Serial.println(root["green"].as<char*>());
  Serial.println(root["blue"].as<char*>());

  String x,y,z;
  x = root["red"].as<char*>();
  y = root["green"].as<char*>();
  z = root["blue"].as<char*>();

  int r,g,b;
  r = x.toInt();
  g = y.toInt();
  b = z.toInt(); 

  for(int i=0; i<pixel_size; i++){
  pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
  
  
}

//void handleTEMPLATE(){
//  Serial.println(server.arg("plain"));
//  String payload = server.arg("plain");
//  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
//  DynamicJsonBuffer jsonBuffer(capacity);
//  JsonObject& root = jsonBuffer.parseObject(payload);
//  if (!root.success()) {
//    Serial.println(F("Parsing failed!"));
//    return;
//  }
//}
//EXAMPLE JSON FORMAT
// Decode JSON/Extract values
//  Serial.println(F("Response:"));
//  Serial.println(root["sensor"].as<char*>());
//  Serial.println(root["time"].as<char*>());
//  Serial.println(root["data"][0].as<char*>());
//  Serial.println(root["data"][1].as<char*>());
//{
//  "sensor": "gps",
//  "time": 1351824120,
//  "data": [
//    48.756080,
//    2.302038
//  ]
//}
