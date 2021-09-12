#pragma once
#include <stdlib.h>

enum RequestType {
    HELLO,
    WIFI,
    CALENDAR,
    DATE,
    FORTUNE
};

struct Request {
    uint16_t magic;
    uint16_t reqtype;
    uint32_t fd;
};