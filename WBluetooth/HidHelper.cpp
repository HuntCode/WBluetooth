#include "HidHelper.h"

uint8_t HidHelper::GetHidUsageFromPs2Set1(uint32_t scanCode)
{
    switch (scanCode)
    {
    case 0x29: return 0x35;  // Key location 1  ( ~ ` )
    case 0x02: return 0x1E;  // Key location 2  ( ! 1 )
    case 0x03: return 0x1F;  // Key location 3  ( @ 2 )
    case 0x04: return 0x20;  // Key location 4  ( # 3 )
    case 0x05: return 0x21;  // Key location 5  ( $ 4 )
    case 0x06: return 0x22;  // Key location 6  ( % 5 )
    case 0x07: return 0x23;  // Key location 7  ( ^ 6 )
    case 0x08: return 0x24;  // Key location 8  ( & 7 )
    case 0x09: return 0x25;  // Key location 9  ( * 8 )
    case 0x0A: return 0x26;  // Key location 10 ( ( 9 )
    case 0x0B: return 0x27;  // Key location 11 ( ) 0 )
    case 0x0C: return 0x2D;  // Key location 12 ( _ - )
    case 0x0D: return 0x2E;  // Key location 13 ( + = )
    case 0x0E: return 0x2A;  // Key location 15 ( Backspace )
    case 0x0F: return 0x2B;  // Key location 16 ( Tab )
    case 0x10: return 0x14;  // Key location 17 ( Q )
    case 0x11: return 0x1A;  // Key location 18 ( W )
    case 0x12: return 0x08;  // Key location 19 ( E )
    case 0x13: return 0x15;  // Key location 20 ( R )
    case 0x14: return 0x17;  // Key location 21 ( T )
    case 0x15: return 0x1C;  // Key location 22 ( Y )
    case 0x16: return 0x18;  // Key location 23 ( U )
    case 0x17: return 0x0C;  // Key location 24 ( I )
    case 0x18: return 0x12;  // Key location 25 ( O )
    case 0x19: return 0x13;  // Key location 26 ( P )
    case 0x1A: return 0x2F;  // Key location 27 ( { [ )
    case 0x1B: return 0x30;  // Key location 28 ( } ] )
    case 0x2B: return 0x31;  // Key location 29* ( | \ )
    case 0x3A: return 0x39;  // Key location 30 ( Caps Lock )
    case 0x1E: return 0x04;  // Key location 31 ( A )
    case 0x1F: return 0x16;  // Key location 32 ( S )
    case 0x20: return 0x07;  // Key location 33 ( D )
    case 0x21: return 0x09;  // Key location 34 ( F )
    case 0x22: return 0x0A;  // Key location 35 ( G )
    case 0x23: return 0x0B;  // Key location 36 ( H )
    case 0x24: return 0x0D;  // Key location 37 ( J )
    case 0x25: return 0x0E;  // Key location 38 ( K )
    case 0x26: return 0x0F;  // Key location 39 ( L )
    case 0x27: return 0x33;  // Key location 40 ( : ; )
    case 0x28: return 0x34;  // Key location 41 ( б░ бо )
    case 0x1C: return 0x28;  // Key location 43 ( Enter )
    case 0x2A: return 0xE1;  // Key location 44 ( L SHIFT )
    case 0x56: return 0x64;  // Key location 45 ( NONE ) **
    case 0x2C: return 0x1D;  // Key location 46 ( Z )
    case 0x2D: return 0x1B;  // Key location 47 ( X )
    case 0x2E: return 0x06;  // Key location 48 ( C )
    case 0x2F: return 0x19;  // Key location 49 ( V )
    case 0x30: return 0x05;  // Key location 50 ( B )
    case 0x31: return 0x11;  // Key location 51 ( N )
    case 0x32: return 0x10;  // Key location 52 ( M )
    case 0x33: return 0x36;  // Key location 53 ( < , )
    case 0x34: return 0x37;  // Key location 54 ( > . )
    case 0x35: return 0x38;  // Key location 55 ( ? / )
    case 0x73: return 0x87;  // Key location 56 ( NONE ) ***
    case 0x36: return 0xE5;  // Key location 57 ( R SHIFT )
    case 0x1D: return 0xE0;  // Key location 58 ( L CTRL )
    case 0xE05B: return 0xE3; // Key location 59 ( L WIN )
    case 0x38: return 0xE2; // Key location 60 ( L ALT )
    case 0x39: return 0x2C; // Key location 61 ( Space Bar )
    case 0xE038: return 0xE6; // Key location 62 ( R ALT )
    case 0xE05C: return 0xE7; // Key location 63 ( R WIN )
    case 0xE01D: return 0xE4; // Key location 64 ( R CTRL )
    case 0xE05D: return 0x65; // Key location 65 ( APP )
    default:
        std::cerr << "[HidHelper] Unsupported scan code: 0x" << std::hex << scanCode << std::endl;
        return 0x00;
    }
}

bool HidHelper::IsModifierKey(uint8_t code)
{
    return code >= 0xE0 && code <= 0xE7;
}

uint8_t HidHelper::GetFlagOfModifierKey(uint8_t code)
{
    if (!IsModifierKey(code))
    {
        std::cerr << "[HidHelper] Not a modifier key: 0x" << std::hex << static_cast<int>(code) << std::endl;
        return 0x00;
    }
    return 1 << (code - 0xE0);
}