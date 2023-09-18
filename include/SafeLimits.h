// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "Configuration.h"

class SafeLimits {
public:
    void init();
    void loop();
    void updated(uint64_t inverterSerial);

private:
    struct Fallback {
      Fallback(): _inverterSerial(0) {}
      uint64_t _inverterSerial;
      int32_t _timeoutMillis;
    };
    Fallback _fallback[INV_MAX_COUNT];
};

extern SafeLimits SafeLimitsInstance;
