#ifndef HID_HELPER_H
#define HID_HELPER_H

#include <cstdint>
#include <set>
#include <vector>
#include <array>
#include <iostream>

enum class KeyEvent
{
    KeyMake,
    KeyBreak
};

class HidHelper
{
public:
    static uint8_t GetHidUsageFromPs2Set1(uint32_t scanCode);
    static bool IsModifierKey(uint8_t usageCode);
    static uint8_t GetFlagOfModifierKey(uint8_t usageCode);
    static bool IsFunctionKey(uint8_t usageCode);
};

#endif // HID_HELPER_H
