#include <Ethernet.h>
#include <PubSubClient.h>

// MQTT topics
#define MQTT_TOPIC_PIEZO_MAX "Altes/Laser Lock/Max. Piezo[V]"
#define MQTT_TOPIC_PIEZO_MIN "Altes/Laser Lock/Min. Piezo[V]"
#define MQTT_TOPIC_PIEZO_CURRENT "Altes/Laser Lock/Piezo[V]"
// Delay between writes
#define WRITE_PERIOD 15000 // in milliseconds
// MQTT settings
#define MQTT_HOST "smartlab.physi.uni-heidelberg.de"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "Arduino-old-laser-lock"
// MAC address of the Arduino
byte mac_address[] = {0x90, 0xA2, 0xDA, 0x0D, 0x63, 0x42};

// Ethernet and MQTT clients
EthernetClient eth_client;
PubSubClient mqtt_client;

int maxValue = 0;
int minValue = 1024;
int value;
unsigned long lastWrite = 0;

void display_error_led() {
  while (true) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

void write_value(PubSubClient *client, const char *topic, int value) {
  String str_value(value);
  const char *c_str_value = str_value.c_str();
  Serial.print("Write ");
  Serial.print(topic);
  Serial.print(" = ");
  Serial.println(c_str_value);
  mqtt_client.publish(topic, c_str_value);
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(9, OUTPUT);

  Serial.println("Setup Ethernet");
  Ethernet.begin(mac_address);
  Serial.print("Assigned IP: ");
  Serial.println(Ethernet.localIP());
  delay(1000);

  Serial.println("Setup MQTT client");
  mqtt_client.setClient(eth_client);
  mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
  // Set keep-alive period to be twice the measurement period
  mqtt_client.setKeepAlive(2 * WRITE_PERIOD);
  // Important: Limit the (input and output) buffer size to twice
  // the size of a random topic. 2x is chosen to have some for
  // headroom.
  // On the Arduino UNO, limited dynamic memory requires small
  // buffers to avoid out-of-memory conditions.
  mqtt_client.setBufferSize(2 * sizeof(MQTT_TOPIC_PIEZO_MAX));

  Serial.println("Try connecting to MQTT...");
  if (!mqtt_client.connect(MQTT_CLIENT_ID)) {
    Serial.println("Unable to connect to MQTT");
    // Endless loop with blinking led to indicate fault
    display_error_led();
  } else {
    Serial.println("Successfully connected to MQTT");
  }
  delay(500);
}

void loop() {
  value = analogRead(A0);
  if (value > maxValue) {
    maxValue = value;
  }
  if (value < minValue) {
    minValue = value;
  }
  if ((millis() - lastWrite) > WRITE_PERIOD) {
    write_value(&mqtt_client, MQTT_TOPIC_PIEZO_MAX, maxValue);
    write_value(&mqtt_client, MQTT_TOPIC_PIEZO_MIN, minValue);
    write_value(&mqtt_client, MQTT_TOPIC_PIEZO_CURRENT, value);
    lastWrite = millis();
    // Reset max and min values
    maxValue = 0;
    minValue = 1024;
  }
}
