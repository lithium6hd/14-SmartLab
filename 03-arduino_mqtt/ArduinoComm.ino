#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Function prototypes
void subscribeReceive(char* topic, byte* payload, unsigned int length);

// Set your MAC address and IP address here
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x05, 0x1F };
IPAddress ip(196, 168, 178, 71);

// Set yout topic here
const char* topic = "altes/test/arduino"

// Make sure to leave out the http and slashes!
const char* server = "147.142.19.112";

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void setup()
{
  // Useful for debugging purposes
  Serial.begin(9600);

  // Start the ethernet connection
  Ethernet.begin(mac, ip);

  // Ethernet takes some time to boot!
  delay(3000);

  // Set the MQTT server to the server stated above ^
  mqttClient.setServer(server, 1883);

  // Attempt to connect to the server with the ID "myClientID"
  if (mqttClient.connect("Arduino01","smartlab","TtSwjSstVDGB"))
  {
    Serial.println("Connection has been established, well done");

    // Establish the subscribe event
    mqttClient.setCallback(subscribeReceive);
  }
  else
  {
    Serial.println("Looks like the server connection failed...");
  }
}

void loop()
{
  // This is needed at the top of the loop!
  mqttClient.loop();

  // Ensure that we are subscribed to the topic "MakerIOTopic"
  // mqttClient.subscribe("altes/test/arduino");

  // Attempt to publish a value to the topic "MakerIOTopic"
  if(mqttClient.publish(topic, "10"))
  {
    Serial.println("Publish message success");
  }
  else
  {
    Serial.println("Could not send message :(");
  }

  // Dont overload the server!
  delay(4000);
}

void subscribeReceive(char* topic, byte* payload, unsigned int length)
{
  // Print the topic
  Serial.print("Topic: ");
  Serial.println(topic);

  // Print the message
  Serial.print("Message: ");
  for(int i = 0; i < length; i ++)
  {
    Serial.print(char(payload[i]));
  }

  // Print a newline
  Serial.println("");
}
