
// programma om een rfid te lezen, dan verbinden met internet en uploaden

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

// Wifi info

const char* ssid = "Proximus-Home-2A98";
const char* password = "wh4sc93p2n44u";

// server info
const char* serverAddress = "http://www.strapbadge.stijnally.net";
const char* endpoint = "/api_badge/%7B'UID':";

// MFRC522 - instellen
#define SS_PIN          15         // Pin: D8
#define RST_PIN         0          // Pin: D3

MFRC522 mfrc522(SS_PIN, RST_PIN);

WiFiClient client;

long chipID;

void setup() {
	Serial.begin(115200);
  while (!Serial);

  // SPI-Bus initialiseren
	SPI.begin();

  // MFRC522 initialiseren
	mfrc522.PCD_Init();

  // kleine pauze   
	delay(10);
  Serial.println("");

  // Details van MFRC522 RFID READER / WRITER geven
	mfrc522.PCD_DumpVersionToSerial();	

  Serial.println("RFID-Chip dichter brengen om de UID te lezen...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Check for RFID card
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Read RFID card UID
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      uid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    mfrc522.PICC_HaltA();
  
    // Send UID to remote server
    if (sendDataToServer(uid)) {
      Serial.println("Data sent successfully");
    } else {
      Serial.println("Failed to send data");
    }
  }
  
  delay(2000);  // Wait for a while before checking for the card again
}

bool sendDataToServer(const String& data) {
  if (!client.connect(serverAddress, 80)) {
    Serial.println("Connection failed");
    return false;
  }
  
  // Create the GET request path
  String getRequest = String(endpoint) + "?data=" + data;

  // Send the HTTP GET request to the server
  client.print("GET ");
  client.print(getRequest);
  client.println(" HTTP/1.1");
  client.println("Host: " + String(serverAddress));
  client.println("Connection: close");
  client.println();
  
  delay(100);
  
  // Check for a valid HTTP response
  String line = client.readStringUntil('\n');
  if (line.indexOf("200 OK") == -1) {
    Serial.println("Invalid HTTP response");
    return false;
  }
  
  // Close the connection
  client.stop();
  
  return true;

  }