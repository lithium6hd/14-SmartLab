# Smartlab monitoring solution with MQTT and InfluxDB

This project is thought to implement a monitoring tool for many different clients.
The principle setup is consisting of a MQTT client and a InfluxDB database.
Individual devices in the lab can communicate with the server using the widely supported MQTT standard.

## Subfolders

The first three subfolders are prepared for your *server*:
- `00-docker`: Docker compose configuration for MQTT, InfluxDB
- `01-mosquitto`: Docker container configuration files for Eclipse [Mosquitto](https://mosquitto.org)
- `02-bridge`: Python script that receives MQTT data and stores them to InfluxDB

The subfolders 03 and 04 give example programs for the communication of monitoring devices with the server using MQTT.
- `03-arduino_mqtt`
- `04-Python_mqtt`

# Setup

## Installing docker and docker compose

Install
[docker](https://docs.docker.com) and
[docker-compose](https://docs.docker.com/compose/), if you don't have
them installed in your *server* yet.
If you don't know if you have them installed or not, simply try to run the
commands `docker` and `docker-compose` on the command line.

For Windows or Mac OS X, the easiest way is to install
[Docker for Desktop](https://www.docker.com/products/docker-desktop).
Alternatively, for Mac OS X you can use [Homebrew](https://brew.sh),
with `brew install docker docker-compose`.

For Linux (Ubuntu or other Debian based), just use `apt install`:

```sh
sudo apt install docker.io
sudo apt install docker-compose
```
For anything else, follow the [official instructions](https://docs.docker.com/install/).

## Setup your server
Given your desired storage location `{DataDir}` you first need to create a number of folders for data storage. The folders that need to be created are given here:
```sh
mkdir {DataDir}/mosquitto
mkdir {DataDir}/mosquitto/data
mkdir {DataDir}/mosquitto/log
mkdir {DataDir}/influxdb
mkdir {DataDir}/influxdb/data
mkdir {DataDir}/influxdb/config
```
Now replace the given location within the file `00-docker/docker-compose.yml` and update them to match the chosen `{DataDir}`. Remember, that if you are using windows the file system uses `\` instead of `/`. Thus the `{DataDir}` has to be created using `\`. This does not apply to the folders the file system is mapped to using docker (everything behind `:`).

Next the enviromental files have to be created. Fist start by movin into the folder `cd 00-docker` and start setting up Influxdb:
```sh
nano influxdb.env
```
In this file add:
```sh
DOCKER_INFLUXDB_INIT_USERNAME=
DOCKER_INFLUXDB_INIT_PASSWORD=
DOCKER_INFLUXDB_INIT_ORG=
DOCKER_INFLUXDB_INIT_BUCKET=
```
Next we will set up Telegraf:
```sh
nano telegraf.env
```
With the content:
```sh
INFLUX_TOKEN=(needs to be added later)
INFLUX_URL=http://:8086
INFLUX_ORG=
INFLUX_BUCKET=
MQTT_SERVER=tcp://:1883
```

Within the file `02-telegraf/telegraf.conf` you can edit the names of the tags associated to the MQTT input. In this case they are chosen to be `Experiment/Device/Measurement`.

Create an empty password file inside the folder `cd ../01-mosquitto` by
```sh
nano passwords
```
and saving it without entering anything.

You can now start the Smartlab installation using:
```sh
cd ../00-docker
docker-compose up -d
```
To check weather all containers are runnign you can use:
```sh
docker ps
```
Logs for a given container can be found using:
```sh
docker logs {Container Name}
```

To finish the installation we now need to access influxdb at the given url at port 8086. Within the Data menu you can add and edit API Tokens. You will need to generate one token and add it to the `telegraf.env` file.
Now restart the Smartlab installation using:
```sh
docker-compose up -d --force-recreate
```
The Installation should now be able to receive data to MQTT and store it within InfluxDB.

You can quickly test your setup by using the python script provided in `04-Python_mqtt` to send some data to your smartlab installation. If all works well you shoud be able to browse throug your assigned tags and finally display data within the Explore tab of influxdb.
