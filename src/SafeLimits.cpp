// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "SafeLimits.h"
#include "Configuration.h"
#include "MessageOutput.h"
#include <Hoymiles.h>
#include <Arduino.h>

static void setLimit(uint64_t inverterSerial, uint16_t watts)
{
    auto inv = Hoymiles.getInverterBySerial(inverterSerial);
    if (inv != 0) {
	MessageOutput.print("Inverter ");
	MessageOutput.print(inverterSerial, HEX);
	MessageOutput.printf(" falls back to Safe Limit: %uW\r\n", watts);
	inv->sendActivePowerControlRequest(watts, PowerLimitControlType::AbsolutNonPersistent);
    } else {
        MessageOutput.print("Ignored safe limit for mising inverter ");
	MessageOutput.print(inverterSerial, HEX);
	MessageOutput.print("\r\n");
    }
}

void SafeLimits::init()
{
    for (auto &conf : Configuration.get().Inverter) {
        if (conf.SafeLimitMillis != 0) {
            setLimit(conf.Serial, conf.SafeLimitWatts);
        }
    }
}

void SafeLimits::loop()
{
    int32_t millisNow = (int32_t)millis();
    for (auto &fallback : _fallback) {
        if (fallback._inverterSerial != 0 && millisNow - fallback._timeoutMillis > 0) {
            if (auto conf = Configuration.getInverterConfig(fallback._inverterSerial)) {
                setLimit(fallback._inverterSerial, conf->SafeLimitWatts);
            }
            fallback._inverterSerial = 0;
        }
    }
}

void SafeLimits::updated(uint64_t inverterSerial) {
    auto conf = Configuration.getInverterConfig(inverterSerial);
    if (conf != 0 && conf->SafeLimitMillis > 0) {
        for (auto &fallback : _fallback) {
            if (fallback._inverterSerial == inverterSerial) {
                fallback._timeoutMillis = (int32_t)(millis() + conf->SafeLimitMillis);
                MessageOutput.printf("Updated inverter %llX safe limit timeout (%u millis)\r\n", inverterSerial, conf->SafeLimitMillis);
                return;
            }
        }
        for (auto &fallback : _fallback) {
            if (fallback._inverterSerial == 0) {
                fallback._inverterSerial = inverterSerial;
                fallback._timeoutMillis = (int32_t)(millis() + conf->SafeLimitMillis);
                MessageOutput.printf("Created inverter %llX safe limit timeout (%u millis)\r\n", inverterSerial, conf->SafeLimitMillis);
                return;
            }
        }
    }
}

SafeLimits SafeLimitsInstance;
