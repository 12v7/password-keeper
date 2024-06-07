
#include "application.h"
#include "class/hid/hid.h" // HID_KEY_*
#include "pico/unique_id.h" // PICO_UNIQUE_BOARD_ID_SIZE_BYTES
#include "hardware/structs/syscfg.h"
#include "sha256.h"


enum {
  S_DEFAULT,
  S_INPUT_PASSWORD,
  S_INPUT_TIMEOUT,
  S_INPUT_TRY_LIMIT,
  S_INPUT_PASSWORD_FORMAT,
};

#define HID_SHIFT 0x80

const uint8_t allowedScanCodes[] = {
  HID_KEY_0, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_6,
  HID_KEY_7, HID_KEY_8, HID_KEY_9,
  HID_KEY_A, HID_KEY_B, HID_KEY_C, HID_KEY_D, /*HID_KEY_E,*/ HID_KEY_F, HID_KEY_G, HID_KEY_H,
  HID_KEY_I, HID_KEY_J, HID_KEY_K, HID_KEY_L, HID_KEY_M, HID_KEY_N, HID_KEY_O, HID_KEY_P,
  /*HID_KEY_Q,*/ HID_KEY_R, HID_KEY_S, HID_KEY_T, HID_KEY_U, HID_KEY_V, /*HID_KEY_W,*/ HID_KEY_X,
  HID_KEY_Y, /*HID_KEY_Z,*/
  HID_SHIFT+HID_KEY_A, HID_SHIFT+HID_KEY_B, HID_SHIFT+HID_KEY_C, HID_SHIFT+HID_KEY_D, /*HID_SHIFT+HID_KEY_E,*/ HID_SHIFT+HID_KEY_F, HID_SHIFT+HID_KEY_G, HID_SHIFT+HID_KEY_H,
  HID_SHIFT+HID_KEY_I, HID_SHIFT+HID_KEY_J, HID_SHIFT+HID_KEY_K, HID_SHIFT+HID_KEY_L, HID_SHIFT+HID_KEY_M, HID_SHIFT+HID_KEY_N, HID_SHIFT+HID_KEY_O, HID_SHIFT+HID_KEY_P,
  /*HID_SHIFT+HID_KEY_Q,*/ HID_SHIFT+HID_KEY_R, HID_SHIFT+HID_KEY_S, HID_SHIFT+HID_KEY_T, HID_SHIFT+HID_KEY_U, HID_SHIFT+HID_KEY_V, /*HID_SHIFT+HID_KEY_W,*/ HID_SHIFT+HID_KEY_X,
  HID_SHIFT+HID_KEY_Y, /*HID_SHIFT+HID_KEY_Z,*/
//  HID_KEY_BRACKET_LEFT, HID_KEY_BRACKET_RIGHT, HID_KEY_SEMICOLON, HID_KEY_APOSTROPHE, HID_KEY_COMMA, HID_KEY_PERIOD, HID_KEY_SLASH
//  Symbols A: ^*%$!&@#
};

uint8_t state = S_DEFAULT;

char buf[260];
char* buf_ptr = NULL;

SHA256_CTX shaCtx;

const unsigned char* send_scan_codes_ptr = NULL;

uint32_t first_press_time = 0;

uint8_t password[SHA256_BLOCK_SIZE] = {0};

uint8_t bPin = 0;


void button_task(uint32_t now)
{
  extern bool get_bootsel_button(void);

  static uint32_t press_time = 0;
  static uint8_t pin = 1;

  if (!password[0]) // if we have no password, will not input the pin
  {
    press_time = 0;
    pin = 1;
    return;
  }

  bool now_pressed = !get_bootsel_button();

  if (now_pressed == !press_time) // if button state changed
  {
    if (now_pressed == false)
    {
      uint32_t duration = now - press_time;
      //printf("%i ", duration);

      if (duration > 20)
      {
        pin = (pin << 1) | (duration >= 500);
        uint8_t pin_mask = (1 << PIN_SIZE) - 1;
        if (pin > pin_mask)
        {
          //printf("pin: %02X %02X\n", pin, bPin);
          if ((pin & pin_mask) == (bPin & pin_mask))
          {
            // password will send only if pin code has been entered during 1 minute
            if (now - first_press_time <= 60000)
              send_scan_codes_ptr = password;
          }
          else // erase the password in case of wrong pin
          {
            pin = 1;
            memset(password, 0, sizeof(password));
          }
          first_press_time = 0;
          //printf("first_press_time 2: %i\n", first_press_time);
          pin = 1; // go next loop
        }
      }
      press_time = 0;
    }
    else
    {
      press_time = now;
      if (!first_press_time)
        first_press_time = now;
    }
  }

  if (first_press_time && (now - first_press_time > PIN_TIMEOUT))
  {
    // clear password due to timeout    
    pin = 1;
    press_time = 0;
    first_press_time = 0;
    memset(password, 0, sizeof(password));
  }
}

void createPassword()
{
  sha256_final(&shaCtx, password);

  pico_unique_board_id_t pico_id;
  pico_get_unique_board_id(&pico_id);

  // Make XOR with salt (pico board identifier) and sha256 1000 times
  for (int i = 0; i < 1000; i++) // 1000 steps = 62 ms
  {
    for (int j = 0; j < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; j++)
      password[j] = password[j] ^ pico_id.id[j];

    sha256_init(&shaCtx);
    sha256_update(&shaCtx, password, sizeof(password));
    sha256_final(&shaCtx, password);
  }
  memset(&shaCtx, 0, sizeof(SHA256_CTX));

  //for (int i = 0; i<sizeof(password); i++)
  //  printf("%02x", password[i]);
  //printf("\n");

  // The PIN code is the last byte of the hash
  bPin = password[sizeof(password) - 1];

  // Convert hash into scan codes
  for (uint8_t i = 0; i < sizeof(password); i++)
    password[i] = allowedScanCodes[password[i] % sizeof(allowedScanCodes)];

  password[sizeof(password) - 2] = HID_KEY_ENTER;
  password[sizeof(password) - 1] = 0;
}


const char * appOnSerialData(uint8_t b)
{
  const char* answer_ptr = NULL;

  switch (state)
  {
  case S_DEFAULT:
    if (b > ' ') // in this mode we should ignore system symbols like <CR><LF>
    {
      switch (b)
      {
      case '?':
      case 'h':
      case 'H':
        answer_ptr = "'A'bout\r\n'P'assword\r\n'S'alt'\r\n"; //github.com/12v7/password-keeper\r\n";
        break;
      case 'a':
      case 'A':
      case 'v':
      case 'V':
        answer_ptr = "Password keeper device v0.3\r\n"; //github.com/12v7/password-keeper\r\n";
        break;
      case 'p':
      case 'P':
        answer_ptr = "Type password phrase then press <Enter>\r\n";
        state = S_INPUT_PASSWORD;
        sha256_init(&shaCtx);
        break;
//      case 't':
//      case 'T':
//        answer_ptr = "Enter timeout\n";
//        state = S_INPUT_TIMEOUT;
//        buf_ptr = buf;
//        break;
//      case 'l':
//      case 'L':
//        answer_ptr = "Enter limit\n";
//        state = S_INPUT_TRY_LIMIT;
//        buf_ptr = buf;
//        break;
//      case 'f':
//      case 'F':
//        answer_ptr = "Enter password format\n";
//        state = S_INPUT_PASSWORD_FORMAT;
//        buf_ptr = buf;
//        break;
      case 's':
      case 'S':
        {
          char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
          pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
          strcpy(buf, "Salt (serial): ");
          strcat(buf, usbd_serial_str);
          strcat(buf, "\r\n");
          answer_ptr = buf;
        }
        break;
      default:
        answer_ptr = "Press 'H' for help\r\n";
        break;
      }
    }
    break;
  case S_INPUT_TIMEOUT:
  case S_INPUT_TRY_LIMIT:
  case S_INPUT_PASSWORD_FORMAT:
    if (b >= ' ')
    {
      if (buf_ptr < buf+sizeof(buf)-1)
      {
        *buf_ptr = b;
        buf_ptr++;
        *buf_ptr = 0;
        answer_ptr = buf_ptr-1; // send echo char
      }
    }
    else
    {
      strcat(buf, "\n");
      answer_ptr = buf;
      state = S_DEFAULT;
    }
    break;
  case S_INPUT_PASSWORD:
    if (b >= ' ')
    {
      sha256_update(&shaCtx, &b, 1);
      answer_ptr = "*";
    }
    else
    {
      state = S_DEFAULT;
      createPassword(&shaCtx);
      strcpy(buf, "\r\nPassword created\r\nPin: ");
      for (int i = 0; i < PIN_SIZE; i++)
        strcat(buf, (bPin & (1 << (PIN_SIZE - 1 - i))) ? "-" : ".");
      strcat(buf, "\r\n");
      syscfg_hw->dbgforce = 0x88;
      answer_ptr = buf;
    }
    break;
  }

  return answer_ptr;
}
