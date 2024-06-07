
#include "tusb.h"

extern void button_task(uint32_t);
extern const unsigned char* send_scan_codes_ptr;

void hid_task(uint32_t now)
{
  // avoid sending multiple zero reports
  static bool send_empty = false;

  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;
  if (now - start_ms < interval_ms)
    return; // not enough time
  start_ms += interval_ms;

  if (send_empty && tud_hid_ready())
  {
    send_empty = false;
    tud_hid_keyboard_report(0, 0, NULL);
  }
  else if (!send_scan_codes_ptr)
  {
    button_task(now);
  }
  else
  {
    if (tud_hid_ready()) // skip if hid is not ready yet
    {
      uint8_t keycodes[6] = { 0x7f & *send_scan_codes_ptr };
      uint8_t modifier = (0x80 & *send_scan_codes_ptr) ? KEYBOARD_MODIFIER_LEFTSHIFT : 0;
      send_scan_codes_ptr++;
      if (!*send_scan_codes_ptr)
        send_scan_codes_ptr = NULL;

      tud_hid_keyboard_report(0, modifier, keycodes);
      send_empty = true;
    }
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;
}
