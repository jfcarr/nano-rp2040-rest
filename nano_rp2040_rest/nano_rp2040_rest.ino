#include "arduino_secrets.h"
#include "rest_manager.h"

RestManager RM(SECRET_SSID, SECRET_PASSWORD, 0, SECRET_PORT, WL_IDLE_STATUS);

void setup()
{
  RM.set_led_pin_modes();

  RM.init_serial_communication(9600);

  RM.check_for_imu();

  RM.check_for_wifi_module();

  RM.check_firmware();

  RM.connect_to_wifi();

  RM.print_wifi_status();

  RM.start_web_server();
}

void loop()
{
  RM.listen_for_client();
}
