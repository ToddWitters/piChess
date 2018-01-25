// For the 64 reed switches, we need
// FOUR port expanders.  Divided up by
#define GPIO_EXPANDER_87_ADDR 0x20
#define GPIO_EXPANDER_65_ADDR 0x21
#define GPIO_EXPANDER_43_ADDR 0x22
#define GPIO_EXPANDER_21_ADDR 0x23

// GPIO exander for the display and
// button boards.
#define GPIO_EXPANDER_UI_ADDR 0x24

// Eight bit display data on Port A,
// three display control lines on Port B
// five button input lines on Port B
#define E_MASK   0x80  // Bit 7, Port B
#define RS_MASK  0x40  // Bit 6, Port B
#define RW_MASK  0x20  // Bit 5, Port B

#define BUTTON_PORT GPIOB_ADDR

#define B_PRESS_MASK 0x10
#define B_UP_MASK    0x08
#define B_DOWN_MASK  0x04
#define B_LEFT_MASK  0x02
#define B_RIGHT_MASK 0x01

#define B_MASK ( B_PRESS_MASK | B_UP_MASK | B_DOWN_MASK | B_LEFT_MASK | B_RIGHT_MASK )

#define ROW_8_SWITCH_INT_PIN  4
#define ROW_7_SWITCH_INT_PIN 25
#define ROW_6_SWITCH_INT_PIN 15
#define ROW_5_SWITCH_INT_PIN 17
#define ROW_4_SWITCH_INT_PIN 18
#define ROW_3_SWITCH_INT_PIN 27
#define ROW_2_SWITCH_INT_PIN 22
#define ROW_1_SWITCH_INT_PIN 23

#define BUTTON_SWITCH_INT_PIN 24

// The following control register addresses assume BANK = 0 (the default)

// Port pin direction
#define IODIRA_ADDR           0x00
#define IODIRB_ADDR           0x01

// Port Pin Interrupt Enable Registers
#define GPINTENA_ADDR         0x04
#define GPINTENB_ADDR         0x05

#define INTCONA_ADDR          0x08
#define INTCONB_ADDR          0x09

// Port pull up register addresses
#define GPPUA_ADDR            0x0C
#define GPPUB_ADDR            0x0D

// Default values against which to check for pin state change
#define DEFVALA_ADDR          0x06
#define DEFVALB_ADDR          0x07

// Port data addresses
#define GPIOA_ADDR            0x12
#define GPIOB_ADDR            0x13

void gpioInit( void );
void uiBoxInit( void );

