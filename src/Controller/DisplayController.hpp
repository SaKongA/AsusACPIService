#ifndef DISPLAY_CONTROLLER_HPP
#define DISPLAY_CONTROLLER_HPP

#include "../ACPI/Display.hpp"

enum class DisplayFeature : uint32_t {
    Overdrive = AsusACPIDisplay::Overdrive,
    OverdriveSupport = AsusACPIDisplay::OverdriveSupport,
    MiniLed = AsusACPIDisplay::MiniLed,
    MiniLedAlternative = AsusACPIDisplay::MiniLedAlternative,
    FullHD = AsusACPIDisplay::FullHD,
    HDR = AsusACPIDisplay::HDR,
    OptimalBrightness = AsusACPIDisplay::OptimalBrightness,
    Initialize = AsusACPIDisplay::Initialize,
    ScreenPadToggle = AsusACPIDisplay::ScreenPadToggle,
    ScreenPadBrightness = AsusACPIDisplay::ScreenPadBrightness
};

class DisplayController {
public:
    explicit DisplayController(AsusACPI &acpi);
    IOReturn read(DisplayFeature feature, uint32_t &value);
    IOReturn write(DisplayFeature feature, uint32_t value, uint32_t &firmwareResult);
    IOReturn supportsOverdrive(bool &supported);
    IOReturn setMiniLed(uint32_t value, uint32_t &firmwareResult);
    IOReturn setScreenPadBrightness(int32_t percent, uint32_t &firmwareResult);

private:
    AsusACPI &acpi;
};

#endif
