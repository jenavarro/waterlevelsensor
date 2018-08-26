#include <ESP8266WiFi.h>
#define echoPin D7 // Echo Pin
#define trigPin D5 // Trigger Pin

 String apiKey = "XXXXXXXXX";
const char* ssid = "XXXXX";
const char* password = "XXXXXXXXXXXXX";
const char* server = "api.thingspeak.com";
int connecttimes = 0;
const bool send = true;

const int sleepSeconds = 60 * 15;

long duration, distance; 

const int qtysamples = 20;
long samples[qtysamples];
long stats[qtysamples][2];
int i,j;
int qtystats=0;
long maxqty=0;
long maxqtyindex=0;
const boolean enablelogging = false;
long dist;
WiFiClient client;

void setup()
{
  
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password); 
  connecttimes = 0;
  while (WiFi.status() != WL_CONNECTED && connecttimes < 120 ) 
  {
    delay(500);
    Serial.print(".");
    connecttimes++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot connect to WiFi, resetting...");
    delay(3000);
    WiFi.disconnect();
    ESP.restart(); 
    delay(3000);
  }
  Serial.println("");
  Serial.println("WiFi connected");

}

void initialize_variables()
{
  for(i=0;i<qtysamples;i++) {
    samples[i]= -1;
  }
  for (j=0; j<qtysamples; j++) {
    stats[j][0]=-1;
    stats[j][1]=0;
  }
  qtystats=0;
  maxqty=-1;
  maxqtyindex=0;
}

long getDistance() {
  if (enablelogging) Serial.println("START LOOP: RAW SAMPLES-------------");

  initialize_variables();

  for (i=0; i<qtysamples;i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) * 0.0343;
    samples[i]=-1;
    if (distance >= 400 || distance <= 2) {
         if (enablelogging) Serial.println("Out of range, duration = ");
      }
      else {
        if (enablelogging) {
          Serial.print(i);
          Serial.print(": ");
          Serial.print("   distance=");
          Serial.print( distance);
          Serial.println(" cm");
        }
        samples[i]=distance;
        delay(100);
      }
  }
  
  if (enablelogging) Serial.println(" START PROCESSING-------------");
  qtystats=0;
  for (i=0; i<qtysamples; i++) {
    if (samples[i]==-1) break;
    for (j=0; j<qtystats;j++) {
      if (stats[j][0]==samples[i]) {
        stats[j][1]++;
        break;
      }
    }
    if (j>=qtystats) {
      stats[qtystats][0]=samples[i];
      stats[qtystats][1]=1;
      qtystats++;
    }
  }
  maxqty=-1;
  maxqtyindex=-1;
  for (j=0; j<qtystats;j++) {
    if (enablelogging) {
       Serial.print(stats[j][0]);
      Serial.print(" qty.: ");
    }
    if (stats[j][1]>maxqty) {
      maxqty=stats[j][1];
      maxqtyindex=j;
      if (enablelogging) Serial.print(" *");
    }
    if (enablelogging) Serial.println(stats[j][1]);
    }
  if (enablelogging) Serial.print(" Most reported distance: ");
  if (maxqty != -1 && maxqty > 1) {   // more than 1 obs for same distance value
      if (enablelogging) {
        Serial.println(stats[maxqtyindex][0]);
        Serial.println(" END LOOP -------------");
      }
     return stats[maxqtyindex][0];
  }
  else {
      if (enablelogging) Serial.println(" END LOOP -------------");
      return -1;  
  }
}
 
void loop()
{
  distance = getDistance();

    if (client.connect(server,80) && distance > -1) {
        String postStr = apiKey;
        postStr +="&field1=";
        postStr += String(distance);
        //postStr += "\r\n\r\n";
        if (send==true)  {
          client.print("POST /update.json HTTP/1.1\n");
          client.print("Host: api.thingspeak.com\n");
          client.print("Connection: close\n");
          client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
          client.print("Content-Type: application/x-www-form-urlencoded\n");
          client.print("Content-Length: ");
          client.print(postStr.length());
          client.print("\n\n");
          client.print(postStr);
        }         
    }
    client.stop();
  
  ESP.deepSleep(sleepSeconds * 1000000);
}
