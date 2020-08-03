/****************************************************************************************************************************
  SinricProPowerSensor.h - Sinric Pro Library for boards

  Based on and modified from SinricPro libarary (https://github.com/sinricpro/)
  to support other boards such as  SAMD21, SAMD51, Adafruit's nRF52 boards, etc.

  Built by Khoi Hoang https://github.com/khoih-prog/SinricPro_Generic
  Licensed under MIT license
  Version: 2.5.1

  Copyright (c) 2019 Sinric. All rights reserved.
  Licensed under Creative Commons Attribution-Share Alike (CC BY-SA)

  This file is part of the Sinric Pro (https://github.com/sinricpro/)

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  2.4.0   K Hoang      21/05/2020 Initial porting to support SAMD21, SAMD51 nRF52 boards, such as AdaFruit Itsy-Bitsy,
                                  Feather, Gemma, Trinket, Hallowing Metro M0/M4, NRF52840 Feather, Itsy-Bitsy, STM32, etc.
  2.5.1   K Hoang      02/08/2020 Add support to STM32F/L/H/G/WB/MP1. Add debug feature, examples. Restructure examples.
                                  Sync with SinricPro v2.5.1: add Speaker SelectInput, Camera.
 *****************************************************************************************************************************/

#ifndef _SINRICPOWERSENSOR_H_
#define _SINRICPOWERSENSOR_H_

#include "SinricProDevice.h"

/**
   @class SinricProPowerSensor
   @brief Device to report power usage
 *  */
class SinricProPowerSensor :  public SinricProDevice
{
  public:
    SinricProPowerSensor(const char* deviceId, unsigned long eventWaitTime = 100);
    
    // From v2.5.1
    String getProductType() 
    {
      return SinricProDevice::getProductType() + String("POWER_SENSOR"); 
    }
    //////
    
    // event
    bool sendPowerSensorEvent(float voltage, float current, float power = -1.0f, float apparentPower = -1.0f, float reactivePower = -1.0f, float factor = -1.0f, String cause = "PERIODIC_POLL");
  private:
    unsigned long startTime = 0;
    unsigned long lastPower = 0;
  protected:
    float getWattHours(unsigned long currentTimestamp);
};

SinricProPowerSensor::SinricProPowerSensor(const char* deviceId, unsigned long eventWaitTime) : SinricProDevice(deviceId, eventWaitTime) {}

/**
   @brief Send PowerSensor event to SinricPro Server
   @param   voltage       voltage
   @param   current       current
   @param   power         (optional) if not provided, it is calculated automaticly (power = voltage * current)
   @param   apparentPower (optional) if not provided it is set to -1
   @param   reactivePower (optional) if not provided it is set to -1
   @param   factor        (optional) if not provided it is set to -1 \n if apparentPower is provided, factor is calculated automaticly (factor = power / apparentPower)
   @param   cause         (optional) Reason why event is sent (default = `"PERIODIC_POLL"`)
   @return  the success of sending the event
   @retval  true          event has been sent successfully
   @retval  false         event has not been sent, maybe you sent to much events in a short distance of time
 **/
bool SinricProPowerSensor::sendPowerSensorEvent(float voltage, float current, float power, float apparentPower, float reactivePower, float factor, String cause)
{
  DynamicJsonDocument eventMessage = prepareEvent(deviceId, "powerUsage", cause.c_str());
  JsonObject event_value = eventMessage["payload"]["value"];

  if (power == -1)
    power = voltage * current;

  if (apparentPower != -1)
    factor = power / apparentPower;

  unsigned long currentTimestamp = getTimestamp();

  event_value["startTime"]     = startTime;
  event_value["voltage"]       = voltage;
  event_value["current"]       = current;
  event_value["power"]         = power;
  event_value["apparentPower"] = apparentPower;
  event_value["reactivePower"] = reactivePower;
  event_value["factor"]        = factor;
  event_value["wattHours"]      = getWattHours(currentTimestamp);

  startTime = currentTimestamp;
  lastPower = power;
  return sendEvent(eventMessage);
}

float SinricProPowerSensor::getWattHours(unsigned long currentTimestamp)
{
  if (startTime)
    return (currentTimestamp - startTime) * lastPower / 3600.0f;

  return 0;
}

#endif    //_SINRICPOWERSENSOR_H_
