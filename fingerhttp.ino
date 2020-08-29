// -----------------------------------------------
// ONLY RUNNING IN ARDUINO version < 1.0.5
// due to WIFI shield firmware
// -----------------------------------------------
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WiFi.h>
// WIRELESS NETWORK VARIABLES
char ssid[] = "YOUR_SSID";     //  your network SSID (name) 
char pass[] = "PASSWORD";    // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
IPAddress ip(0,0,0,0);
IPAddress myDns(8,8,8,8); // Google DNS 
WiFiClient client;           // initialize the library instance:
IPAddress myServer(192,168,0,219); // Address where your xAMP solution is running
// FINGERPRINT VARIABLES
int getFingerprintIDez();
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
void setup() {
  // initialize serial:
  Serial.begin(9600);

  // attempt to connect using WPA2 encryption:
  Serial.println("Login to wireless network ...");
  status = WiFi.begin(ssid, pass);

  // if you're not connected, stop here:
  if ( status != WL_CONNECTED) { 
    Serial.println("No wifi connection established");
    while(true);
  } 
  // if you are connected, print out info about the connection:
  else {
    Serial.println("Connected to WiFi ...");
    delay(200);
  }
  Serial.println("Check fingerprint sensor");
  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor OK!!!");
  } else {
    Serial.println("No fingerprint sensor found:(");
    while (1);
  }
  printWifiStatus();
  Serial.println("Waiting for a valid fingerprint ...");
}

void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  int printID;
  printID=getFingerprintIDez();
  if (printID != -1) {
    // send the ID to the database for registration
    httpRequest(printID);
    // opnieuw ...
    Serial.println("Waiting for a valid fingerprint ...");
  }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("ID # found"); Serial.print(finger.fingerID); 
  Serial.print(" with probability of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
// this method makes a HTTP connection to the server:
void httpRequest(int printID) {
  // if there's a successful connection:
  if (client.connect(myServer, 80)) {
    Serial.println("Connecting to time server ");
    // send the HTTP PUT request:
    client.print("GET /RegistreerFinger/registreer.php?id=");
    client.print(printID); 
    client.println(" HTTP/1.1");
    client.println("Host: 192.168.0.219"); 
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    // lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("Sorry, no connection ...");
    Serial.println("Shutting down.");
    client.stop();
  }
}
