##############
## Script listens to serial port and writes contents into a file
##############
## requires pySerial to be installed
import serial  # sudo pip install pyserial should work
from paho.mqtt import client as mqtt_client

"""
::this can be used to desiplay all available ports::

import serial.tools.list_ports


ports = list(serial.tools.list_ports.comports())
for p in ports:
    print(p)
"""

serial_port = ''
baud_rate = 9600  # In arduino, Serial.begin(baud_rate)
ser = serial.Serial(serial_port, baud_rate)

broker = ''
port = 1883
topic = "//"
client_id = ""


def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    #client.username_pw_set("", "")
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def publish(client):

    while True:

        line = ser.readline()
        # ser.readline returns a binary, convert to string
        line = line.decode("utf-8")
        line = line.rstrip()
        Input = line

        result = client.publish(topic, Input)
        status = result[0]
        if status == 0:
            print(f"Send `{Input}` to topic `{topic}`")
        else:
            print(f"Failed to send message to topic `{topic}`")


def run():
    client = connect_mqtt()
    client.loop_start()
    publish(client)


if __name__ == '__main__':
    run()

ser.close()
