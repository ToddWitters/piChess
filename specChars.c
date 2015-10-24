#include "specChars.h"


const uint8_t charRightFilledArrowArray[8] = { 0x08, 0x0C, 0x0E, 0x0F, 0x0E, 0x0C, 0x08, 0x00};
const uint8_t charLeftFilledArrowArray[8]  = { 0x02, 0x06, 0x0E, 0x1E, 0x0E, 0x06, 0x02, 0x00};
const uint8_t charBackslashArray[8]        = { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00};

specCharDefn charRightFilledArrow = &charRightFilledArrowArray;
specCharDefn charLeftFilledArrow = &charLeftFilledArrowArray;
specCharDefn charBackslash = &charBackslashArray;
