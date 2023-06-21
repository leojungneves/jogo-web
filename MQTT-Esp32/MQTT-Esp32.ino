#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <DHT.h>

#define rainSensorPin 18
#define DHTTYPE DHT11
#define DHTPIN 4
DHT dht(DHTPIN, DHTTYPE);

unsigned long delay1 = 0;

// Update these with values suitable for your network.
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

char LED;

void setup_wifi() {
  Serial.begin(115200);

  WiFiManager wm;

  bool res;
  res = wm.autoConnect("ESP-32_Leo", "leojungneves");

  if (!res) {
    Serial.println("Failed to connect");
  }
  else {
    Serial.println("Connected");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 'L' was received as the first character
  if ((char)payload[0] == 'L') {
    digitalWrite(LED, HIGH);
    snprintf(msg, MSG_BUFFER_SIZE, "O LED está aceso");
    Serial.print("Publishing message: ");
    Serial.println(msg);
    client.publish("leojung/led", msg);
  }
  else if ((char)payload[0] == 'l') {
    digitalWrite(LED, LOW);
    snprintf(msg, MSG_BUFFER_SIZE, "O LED está apagado");
    Serial.print("Publishing message: ");
    Serial.println(msg);
    client.publish("leojung/led", msg);
  }
  // Handle MQTT messages if needed
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "Leo_Esp32_Cliente";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.subscribe("leojung/publisher");
    }
    else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  LED = 5;
  pinMode(rainSensorPin, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" °C");
  snprintf(msg, MSG_BUFFER_SIZE, "%f", t);
  client.publish("leojung/temperatura", msg);

  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.println(" %");
  snprintf(msg, MSG_BUFFER_SIZE, "%f", h);
  client.publish("leojung/umidade", msg);

  int rainSensorValue = digitalRead(rainSensorPin);

  if ((millis() - delay1 ) > 3000){
  if (rainSensorValue == HIGH) {
    Serial.println("Sem chuva");
    snprintf(msg, MSG_BUFFER_SIZE, "Sem Chuva");
    client.publish("leojung/estadochuva", msg);
  }
  else {
    Serial.println("Com Chuva");
    snprintf(msg, MSG_BUFFER_SIZE, "Com Chuva");
    client.publish("leojung/estadochuva", msg);
  }

    delay1 = millis();
  }

  delay(1000);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}