
#include "application.h"
#include "bsp/board.h"
#include "tusb.h"
#include "hardware/structs/syscfg.h"
#include "hardware/watchdog.h"

void hid_task(uint32_t now);
void led_blinking_task(uint32_t now);

extern bool cdc_connected;
extern uint8_t password[32];
extern uint32_t first_press_time;

int main(void)
{
  // detach SWD pins to prevent reading the password from memory
  syscfg_hw->dbgforce = 0x88; 

  watchdog_enable(100, 0);

  board_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  while (1)
  {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    tud_task(); // tinyusb device task
    hid_task(now);
    led_blinking_task(now);
    watchdog_update();
  }

  return 0;
}


// LED states
void led_blinking_task(uint32_t now)
{
  if (cdc_connected || first_press_time)
  {
    board_led_write((now&0x1ff) > 0xff);
  }
  else if (password[0])
  {
    board_led_write((now&0x1fff) < 0x1f);
  }
  else
  {
    board_led_write(true);
  }
}
