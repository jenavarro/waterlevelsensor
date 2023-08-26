#include <NewPing.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

#define TRIGGER_PIN 5     // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 18       // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400  // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);  // NewPing setup of pins and maximum distance.
WebServer Server;
AutoConnect Portal(Server);

void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}
void statsPage() {
  String strg = String("{\"distance\":" + String(getMeasurement()) + ",\"unit\":\"cm\"}") ;
  Server.send(200, "application/json", strg);
}

int qty_measurements = 0;

void setup() {
  delay(1000);
  Serial.begin(115200);

  Server.on("/", rootPage);
  Server.on("/stats", statsPage);
  Portal.begin();
  Serial.println("Web server started:" + WiFi.localIP().toString());
}

int getMeasurement() {
  int obs1 = getOneMeasurement();
  int obs2 = getOneMeasurement();
  int obs3 = getOneMeasurement();
  int obs4 = getOneMeasurement();
  int obs5 = getOneMeasurement();
  int measurement = (obs1 + obs2 + obs3 + obs4 + obs5) / 5;

  Serial.print("Avg Dist: ");
  Serial.print(measurement);  // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");
  return measurement;
}

int getOneMeasurement() {
  int dist = -1;
  for (; dist < 2 || dist > 300;) {
    dist = sonar.ping_cm();
    delay(100);
  }
  return dist;
}

void loop() {
  Portal.handleClient();
}
