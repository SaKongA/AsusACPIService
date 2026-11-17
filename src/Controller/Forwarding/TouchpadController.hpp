#ifndef TOUCHPAD_CONTROLLER_HPP
#define TOUCHPAD_CONTROLLER_HPP

#include <IOKit/IOReturn.h>

class IOService;

class TouchpadController {
public:
    explicit TouchpadController(IOService *sender);
    IOReturn read(bool &enabled);
    IOReturn set(bool enabled);

private:
    IOReturn findDevice(IOService *&device);
    IOService *sender;
};

#endif
