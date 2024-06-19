#include <Adafruit_DPS310.h>
#include <Adafruit_HTU31D.h>
#include <Adafruit_MAX31865.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Resitance for the PT1000 sensor boards
// Used to calculate temperature
#define PT1000_RREF 4300.0
#define PT1000_RNOMINAL 1000.0
// MQTT topics
#define MQTT_TOPIC_DPS_TEMP "Altes/DPS310/Temperature[°C]"
#define MQTT_TOPIC_DPS_PRESSURE "Altes/DPS310/Pressure[hPa]"
#define MQTT_TOPIC_HTU_TEMP "Altes/HTU31D/Temperature[°C]"
#define MQTT_TOPIC_HTU_HUMIDITY "Altes/HTU31D/Rel.Humidity[%]"
// Delay between loop iterations
#define MEASUREMENT_PERIOD 15000 // in milliseconds
// MQTT settings
#define MQTT_HOST "smartlab.physi.uni-heidelberg.de"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "Arduino-old-i2c"
// MAC address of the Arduino
byte mac_address[] = {0x90, 0xA2, 0xDA, 0x0F, 0xC3, 0x7F};

// Sensors
typedef struct SensorPT1000 {
  Adafruit_MAX31865 sensor;
  bool connected;
  char *topic;
} SensorPT1000;
SensorPT1000 sensorsPT1000[] = {
  SensorPT1000 {Adafruit_MAX31865(9, 11, 12, 13), false, "Altes/PT1000-01/Temperature[°C]"},
  SensorPT1000 {Adafruit_MAX31865(8, 11, 12, 13), false, "Altes/PT1000-02/Temperature[°C]"},
};
Adafruit_DPS310 dps;
Adafruit_HTU31D htu;

// Detected automatically
// Do not change these values
bool has_dps = false;
bool has_htu = false;

// Ethernet and MQTT clients
EthernetClient eth_client;
PubSubClient mqtt_client;

// MQTT retry counter
size_t connection_retry_counter = 0;
#define MAX_RETRY_COUNT 5

void display_error_led() {
  while (true) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

void write_value(PubSubClient *client, const char *topic, float value) {
  String str_value(value, 4);
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
  mqtt_client.setKeepAlive(2 * MEASUREMENT_PERIOD);
  // Important: Limit the (input and output) buffer size to twice
  // the size of a random topic. 2x is chosen to have some for
  // headroom.
  // On the Arduino UNO, limited dynamic memory requires small
  // buffers to avoid out-of-memory conditions.
  mqtt_client.setBufferSize(2 * sizeof(MQTT_TOPIC_DPS_PRESSURE));

  Serial.println("Try connecting to MQTT...");
  if (!mqtt_client.connect(MQTT_CLIENT_ID)) {
    Serial.println("Unable to connect to MQTT");
    // Endless loop with blinking led to indicate fault
    display_error_led();
  } else {
    Serial.println("Successfully connected to MQTT");
  }
  delay(500);

  Serial.println("Setup DPS310");
  // Setup DPS (temperature) to use default I2C address
  if (dps.begin_I2C(DPS310_I2CADDR_DEFAULT)) {
    has_dps = true;
    dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
    Serial.println("DPS310 configured");
  }

  Serial.println("Setup HTU31D");
  if (htu.begin(HTU31D_DEFAULT_I2CADDR)) {
    has_htu = true;
    Serial.println("HTU31D configured");
  }

  for (size_t i : sensorsPT1000) {
    SensorPT1000 sensor = sensorsPT1000[i];
  }

  Serial.println("Setup MAX31865");
  pt1000_01.begin(MAX31865_3WIRE);
  has_pt1000_01 = pt1000_01.readFault() == 0x00;
}

void loop() {
  if (!mqtt_client.loop()) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Connection closed or client unavailable");
    Serial.println("Try connecting...");
    connection_retry_counter++;
    if (!mqtt_client.connect(MQTT_CLIENT_ID)) {
      // Failed to reconnect. Either try again in 5 seconds
      // or enter endless loop.
      Serial.println("Unable to connect to MQTT");
      if (connection_retry_counter >= MAX_RETRY_COUNT) {
        Serial.println("Unable to reconnect after multiple attempts");
        // Enter endless loop with blinking led
        display_error_led();
      } else {
        Serial.println("Retry connection in 5 seconds");
        delay(5000);
        return;
      }
    } else {
      Serial.println("Reconnected to MQTT");
    }
  }

  digitalWrite(LED_BUILTIN, HIGH);

  if (has_dps) {
    sensors_event_t temp_event_dps, pressure_event_dps;
    bool _temp_available = dps.temperatureAvailable();
    bool _pressure_available = dps.pressureAvailable();
    dps.getEvents(&temp_event_dps, &pressure_event_dps);
    if (_temp_available) {
      write_value(&mqtt_client, MQTT_TOPIC_DPS_TEMP,
                  temp_event_dps.temperature);
    }
    if (_temp_available) {
      write_value(&mqtt_client, MQTT_TOPIC_DPS_PRESSURE,
                  pressure_event_dps.pressure);
    }
  }

  if (has_htu) {
    sensors_event_t temp_event_htu, humidity_event_htu;
    htu.getEvent(&humidity_event_htu, &temp_event_htu);
    write_value(&mqtt_client, MQTT_TOPIC_HTU_TEMP, temp_event_htu.temperature);
    write_value(&mqtt_client, MQTT_TOPIC_HTU_HUMIDITY,
                humidity_event_htu.relative_humidity);
  }

  if (has_pt1000_01)
    write_value(&mqtt_client, MQTT_TOPIC_PT1000_01_TEMP,
                pt1000_01.temperature(RNOMINAL, PT1000_RREF));
  delay(MEASUREMENT_PERIOD);
}
