
#include <stdint.h>
#include <stdbool.h>


#define PASSWORD_LENGTH 30

// Pin code length in bits
#define PIN_SIZE 5

// The time interval (ms) for input the pin code
#define PIN_TIMEOUT 600000


uint8_t appGetState(void);

const char * appOnSerialData(uint8_t b);
