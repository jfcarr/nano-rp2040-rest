#include "arduino_secrets.h"
#include "rest_manager.h"

RestManager restManager(SECRET_SSID, SECRET_PASSWORD, 0, SECRET_PORT, WL_IDLE_STATUS);

void setup()
{
  restManager.set_led_pin_modes();

  restManager.init_serial_communication(9600);

  restManager.check_for_imu();

  restManager.check_for_wifi_module();

  restManager.check_firmware();

  restManager.connect_to_wifi();

  restManager.print_wifi_status();

  restManager.start_web_server();
}

void loop()
{
  restManager.listen_for_client();
}
