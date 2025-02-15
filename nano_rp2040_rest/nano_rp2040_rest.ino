#include "arduino_secrets.h"
#include "rest_manager.h"

RestManager restManager(SECRET_SSID, SECRET_PASSWORD, 0, SECRET_PORT, WL_IDLE_STATUS);

void setup()
{
  restManager.set_led_pin_modes();

  Serial.begin(9600); // initialize serial communication at 9600 baud

  restManager.check_for_imu();

  restManager.check_for_wifi_module();

  restManager.check_firmware();

  // Not connected: green LED off, red LED on:
  restManager.control_rgb_led(RGB_ACTION::On, RGB_ACTION::Off, RGB_ACTION::NoChange);

  restManager.connect_to_wifi();

  restManager.server->begin(); // start the web server
  restManager.print_wifi_status();

  // Connected: red LED off, green LED on:
  restManager.control_rgb_led(RGB_ACTION::Off, RGB_ACTION::On, RGB_ACTION::NoChange);
}

void loop()
{
  restManager.listen_for_client();
}
