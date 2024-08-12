#define BLYNK_TEMPLATE_ID "TMPL6YAS5_ss3"
#define BLYNK_TEMPLATE_NAME "Iot smart exhaust fan"
#define BLYNK_PRINT Serial 
#define BLYNK_AUTH_TOKEN "xHEtcCCVY9iy3R57wPCzaQMJ-Xj_cagZ"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h" 

const char auth[] = "xHEtcCCVY9iy3R57wPCzaQMJ-Xj_cagZ";
const char ssid[] = "Triangulum";
const char pass[] = "27773533";

#define DHTPIN 22
#define DHTTYPE DHT11 

const int gasSensorPin = 34;
const int relayPin = 18;
const int gasThreshold = 20;

DHT dht(DHTPIN, DHTTYPE);

bool manualMode = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Blynk.begin(auth, ssid, pass);

  pinMode(relayPin, OUTPUT);
  pinMode(gasSensorPin, INPUT);

  digitalWrite(relayPin, LOW);  // Start with the relay off
  delay(2000);
  
  Serial.println("Setup complete. Device is ready.");
}

BLYNK_WRITE(V4) {
  int relayControl = param.asInt();  // Get value from Blynk app
  Serial.print("Received manual control value: ");
  Serial.println(relayControl);

  manualMode = (relayControl == 1);  // Set manual mode based on app input

  if (manualMode) {
    Serial.println("Manual mode activated: Turning relay ON.");
    digitalWrite(relayPin, HIGH);  // Turn on relay in manual mode
  } else {
    Serial.println("Manual mode deactivated: Turning relay OFF.");
    digitalWrite(relayPin, LOW);  // Turn off relay when exiting manual mode
  }

  Blynk.virtualWrite(V3, relayControl);  // Update relay status on Blynk
}

void loop() {
  Blynk.run();  // Run Blynk

  int sensorValue = analogRead(gasSensorPin);  // Read gas sensor value
  int gas_percentage = map(sensorValue, 0, 4095, 0, 100);  // Convert to percentage

  float humidity = dht.readHumidity();  // Read humidity from DHT sensor
  float temperature = dht.readTemperature();  // Read temperature from DHT sensor

  // Check if sensor readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Print sensor values to the Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C ");

  Serial.print("Gas sensor value: ");
  Serial.println(sensorValue);

  Serial.print("Gas Percentage: ");
  Serial.print(gas_percentage);
  Serial.println("%");

  Serial.println();

  // Automatic control logic based on gas levels
  if (!manualMode) {
    if (gas_percentage > gasThreshold) {
      Serial.println("Automatic Mode: Gas threshold exceeded. Relay ON");
      digitalWrite(relayPin, LOW);  // Activate relay if gas above threshold
      Blynk.virtualWrite(V3, LOW);  // Update Blynk app
    } else {
      Serial.println("Automatic Mode: Gas below threshold. Relay OFF");
      digitalWrite(relayPin, HIGH);  // Deactivate relay if gas below threshold
      Blynk.virtualWrite(V3, HIGH);  // Update Blynk app
    }
  }

  // Send sensor values to Blynk
  Blynk.virtualWrite(V0, gas_percentage);  // Send gas percentage
  Blynk.virtualWrite(V1, temperature);  // Send temperature
  Blynk.virtualWrite(V2, humidity);  // Send humidity

  delay(1000);  // Wait for a second before next loop iteration
}