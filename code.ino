#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <base64.h>

// Replace placeholders with your Firebase credentials
#define FIREBASE_HOST "your-firebase-host.firebaseio.com"
#define FIREBASE_AUTH "your-firebase-auth-token"

#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

#define DHT_PIN 4      // Define the pin to which the DATA pin of the DHT sensor is connected
#define DHT_TYPE DHT11 // Set the type of DHT sensor you are using (DHT11 or DHT22)

// Replace placeholders with your email server and credentials
#define SMTP_SERVER "your-smtp-server.com"
#define SMTP_PORT 465
#define EMAIL_FROM "your-email@example.com"
#define EMAIL_PASSWORD "YourEmailPassword"
#define EMAIL_TO "recipient@example.com"
#define EMAIL_SUBJECT "Temperature Alert"

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure client;

void setup() {
  Serial.begin(9600);
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  delay(1000);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);

  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void sendEmail(float temperature) {
  String emailContent = "Temperature is above 25°C! Current Temperature: " + String(temperature) + " °C";

  client.connect(SMTP_SERVER, SMTP_PORT);
  client.println("EHLO " + WiFi.localIP().toString());
  client.println("AUTH LOGIN");
  client.println(base64::encode(EMAIL_FROM));
  client.println(base64::encode(EMAIL_PASSWORD));
  client.println("MAIL FROM:<" + String(EMAIL_FROM) + ">");
  client.println("RCPT TO:<" + String(EMAIL_TO) + ">");
  client.println("DATA");
  client.println("Subject: " + String(EMAIL_SUBJECT));
  client.println("From: " + String(EMAIL_FROM));
  client.println("To: " + String(EMAIL_TO));
  client.println(emailContent);
  client.println(".");
  client.println("QUIT");
  client.stop();
}

void loop() {
  // Read DHT sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Round the values to the nearest whole number
  int roundedTemperature = round(temperature);
  int roundedHumidity = round(humidity);

  // Send rounded data to Firebase
  Firebase.setInt("values/temperature", roundedTemperature);
  Firebase.setInt("values/humidity", roundedHumidity);

  // Check if temperature is above 25°C
  if (roundedTemperature > 25) {
    // Send email
    sendEmail(temperature);
  }

  // Print rounded data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(roundedTemperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(roundedHumidity);
  Serial.println(" %");

  delay(5000);  // Delay for 5 seconds between Firebase updates
}
