## Description

Simple REST service for the Arduino Nano RP2040 Connect

## Recommended Toolset

* [Nano RP2040 board with a micro USB cable](https://www.amazon.com/dp/B095J4KFVT)
* [Arduino CLI](https://docs.arduino.cc/arduino-cli/)
* [Visual Studio Code](https://code.visualstudio.com/) with the [Arduino Community Edition extension](https://marketplace.visualstudio.com/items?itemName=vscode-arduino.vscode-arduino-community). This is a community fork of Microsoft's ([deprecated](https://github.com/microsoft/vscode-arduino/issues/1757)) Arduino extension.

## Board Setup

Plug in the Nano RP2040 board and issue the following command:

```bash
arduino-cli board list
```

You should see something like this:

```txt
Port         Protocol Type              Board Name                  FQBN                                Core
/dev/ttyACM0 serial   Serial Port (USB) Arduino Nano RP2040 Connect arduino:mbed_nano:nanorp2040connect arduino:mbed_nano
```

You will probably need to install the core:

```bash
arduino-cli core install arduino:mbed_nano
```

## Library Dependencies

Name | Description
---- | -----------
[Arduino_LSM6DSOX](https://docs.arduino.cc/libraries/arduino_lsm6dsox/) | Access the IMU for accelerometer, gyroscope, and embedded temperature sensor.
[ArduinoJson](https://docs.arduino.cc/libraries/arduinojson/) | A simple and efficient JSON library for embedded C++.
[WiFiNINA](https://docs.arduino.cc/libraries/wifinina/) | With this library you can instantiate Servers, Clients and send/receive UDP packets through WiFi.

Install libraries:

```bash
arduino-cli lib install Arduino_LSM6DSOX

arduino-cli lib install ArduinoJson

arduino-cli lib install WiFiNINA
```

## Compile and Upload

Make sure the board is connected, then open a terminal.

Compile:

```bash
make compile
```

or

```bash
arduino-cli compile --fqbn arduino:mbed_nano:nanorp2040connect nano_rp2040_rest
```

You should see something similar to this:

```txt
Sketch uses 112214 bytes (0%) of program storage space. Maximum is 16777216 bytes.
Global variables use 44552 bytes (16%) of dynamic memory, leaving 225784 bytes for local variables. Maximum is 270336 bytes.

Used library     Version
Arduino_LSM6DSOX 1.1.2
Wire
SPI
ArduinoJson      7.3.0
WiFiNINA         1.9.0

Used platform     Version
arduino:mbed_nano 4.2.1 
```

Upload:

```bash
make upload
```

or

```bash
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:mbed_nano:nanorp2040connect nano_rp2040_rest
```

You should see something similar to this:

```txt
...
New upload port: /dev/ttyACM0 (serial)
```

Start the monitor to check the status of the running server:

```bash
make monitor
```

or

```bash
arduino-cli monitor -p /dev/ttyACM0
```

Results will be similar to this:

```txt
Using default monitor configuration for board: arduino:mbed_nano:nanorp2040connect
Monitor port settings:
  baudrate=9600
  bits=8
  dtr=on
  parity=none
  rts=on
  stop_bits=1

Connecting to /dev/ttyACM0. Press CTRL-C to exit.
SSID: (network name)
IP Address: (server ip address)
signal strength (RSSI): -40 dBm
```

## Call the Service

Now that we've completed our code, flashed the device, and our server is running, we're ready to test it.

First, note the server address from the monitor above.  I'll use an example of 192.168.0.186.

There are many options for calling the service.  You could use cURL:

```bash
curl --request GET --url http://192.168.0.186:8090/Temperature/Current/F
```

If you're using a REST runner that recognizes .http files, you can use test/get_temp.http

You could also use a REST client like Postman or Insomnia.  Since these are simple GET requests, you can even put the URL directly into a web browser.  Regardless of how you call the service, though, you should see a response similar to this:

```json
HTTP/1.1 200 OK
Content-type: application/json
Connection: close

{
  "message": "Current temperature is 70 °F",
  "value": 70,
  "status": "success"
}
```

If you call the service with an endpoint it doesn't recognize, you'll see this:

```json
HTTP/1.1 200 OK
Content-type: application/json
Connection: close

{
  "message": "Hello from Arduino RP2040! Valid endpoints are /Temperature/Current/F and /Temperature/Current/C",
  "value": -99,
  "status": "invalid"
}
```
