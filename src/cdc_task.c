
#include "tusb.h"
#include "application.h"

bool cdc_connected;

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;
  (void) rts;

  if ( dtr )
  {
    // Terminal connected
    cdc_connected = true;
  }
  else
  {
    // Terminal disconnected
    cdc_connected = false;
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;

  while (tud_cdc_available())
  {
    const char * answer = appOnSerialData(tud_cdc_read_char());
    if (answer)
    {
      tud_cdc_write_str(answer);
      tud_cdc_write_flush();
    }
  }
}
