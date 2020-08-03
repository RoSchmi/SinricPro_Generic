/****************************************************************************************************************************
  SinricProDimSwitch.h - Sinric Pro Library for boards

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

#ifndef _SINRICDIMSWITCH_H_
#define _SINRICDIMSWITCH_H_

#include "SinricProDevice.h"

#define DIM_SWITCH_MIN_POWER_LEVEL      0
#define DIM_SWITCH_MAX_POWER_LEVEL      100

/**
   @class SinricProDimSwitch
   @brief Device which supports on / off and dimming commands
 **/
class SinricProDimSwitch :  public SinricProDevice
{
  public:
    SinricProDimSwitch(const char* deviceId, unsigned long eventWaitTime = 100);
    
    // From v2.5.1
    String getProductType() 
    {
      return SinricProDevice::getProductType() + String("DIMMABLE_SWITCH"); 
    }
    //////
    
    // callbacks
    /**
       @brief Definition for setPowerLevel callback

       Gets called when device receive a `setPowerLevel` reuqest \n
       @param[in]     deviceId        String which contains the ID of device
       @param[in]     powerLevel      integer `0..100` new powerlevel the device should be set to
       @param[out]    powerLevel      integer `0..100` report the powerlevel that the device have been set to
       @return        the success of the request
       @retval        true            request handled properly
       @retval        false           request was not handled properly because of some error

       @section SetPowerLevelCallback Example-Code
       @snippet callbacks.cpp onPowerLevel
     **/
    typedef std::function<bool(const String&, int&)> SetPowerLevelCallback;

    /**
       @brief Definition for onAdjustPowerLevel callback

       Gets called when device receive a `adjustPowerLevel` reuqest \n
       @param[in]     deviceId      String which contains the ID of device
       @param[in]     powerLevel    integer `-100..100` delta value which power level have to be changed
       @param[out]    powerLevel    integer `0..100` report the absolute powerlevel that the device have been set to
       @return        the success of the request
       @retval true   request handled properly
       @retval false  request can`t be handled properly because of some error

       @section AdjustPowerLevelCallback Example-Code
       @snippet callbacks.cpp onAdjustPowerLevel
     **/
    typedef std::function<bool(const String&, int&)> AdjustPowerLevelCallback;

    void onPowerLevel(SetPowerLevelCallback cb);
    void onAdjustPowerLevel(AdjustPowerLevelCallback cb);

    // event
    bool sendPowerLevelEvent(int powerLevel, String cause = "PHYSICAL_INTERACTION");

    // handle
    bool handleRequest(const char* deviceId, const char* action, JsonObject &request_value, JsonObject &response_value) override;
    
  private:
  
    SetPowerLevelCallback setPowerLevelCallback;
    AdjustPowerLevelCallback adjustPowerLevelCallback;
};

SinricProDimSwitch::SinricProDimSwitch(const char* deviceId, unsigned long eventWaitTime) : SinricProDevice(deviceId, eventWaitTime),
  setPowerLevelCallback(nullptr),
  adjustPowerLevelCallback(nullptr) {}

bool SinricProDimSwitch::handleRequest(const char* deviceId, const char* action, JsonObject &request_value, JsonObject &response_value)
{
  if (strcmp(deviceId, this->deviceId) != 0)
    return false;

  if (SinricProDevice::handleRequest(deviceId, action, request_value, response_value))
    return true;

  bool success = false;
  String actionString = String(action);

  if (setPowerLevelCallback && actionString == "setPowerLevel")
  {
    int powerLevel = request_value["powerLevel"];
    success = setPowerLevelCallback(String(deviceId), powerLevel);
    // Mod From v2.5.1
    response_value["powerLevel"] = limitValue(powerLevel, DIM_SWITCH_MIN_POWER_LEVEL, DIM_SWITCH_MAX_POWER_LEVEL);
    //////
  }

  if (adjustPowerLevelCallback && actionString == "adjustPowerLevel")
  {
    int powerLevelDelta = request_value["powerLevelDelta"];
    success = adjustPowerLevelCallback(String(deviceId), powerLevelDelta);
    // Mod From v2.5.1
    response_value["powerLevel"] = limitValue(powerLevelDelta, DIM_SWITCH_MIN_POWER_LEVEL, DIM_SWITCH_MAX_POWER_LEVEL);
    //////
  }

  return success;
}

/**
   @brief Set callback function for setPowerLevel request

   @param     cb    Function pointer to a SetPowerLevelCallback function
   @see       SetPowerLevelCallback
 **/
void SinricProDimSwitch::onPowerLevel(SetPowerLevelCallback cb)
{
  setPowerLevelCallback = cb;
}

/**
   @brief Set callback function for adjustPowerLevel request

   @param     cb    Function pointer to a AdjustPowerLevelCallback function
   @see       AdjustPowerLevelCallback
 **/
void SinricProDimSwitch::onAdjustPowerLevel(AdjustPowerLevelCallback cb)
{
  adjustPowerLevelCallback = cb;
}

/**
   @brief Send `setPowerLevel` event to SinricPro Server indicating actual power level

   @param   powerLevel    integer `0..100` report the powerlevel that the device have been set to
   @param   cause         (optional) Reason why event is sent (default = `"PHYSICAL_INTERACTION"`)
   @return  the success of sending the event
   @retval  true          event has been sent successfully
   @retval  false         event has not been sent, maybe you sent to much events in a short distance of time
 **/
bool SinricProDimSwitch::sendPowerLevelEvent(int powerLevel, String cause)
{
  DynamicJsonDocument eventMessage = prepareEvent(deviceId, "setPowerLevel", cause.c_str());
  JsonObject event_value = eventMessage["payload"]["value"];
  event_value["powerLevel"] = powerLevel;
  return sendEvent(eventMessage);
}

#endif    //_SINRICDIMSWITCH_H_

