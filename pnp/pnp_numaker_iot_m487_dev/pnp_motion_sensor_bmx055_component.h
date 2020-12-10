/*
 * Copyright (c) 2020, Nuvoton Technology Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This header implements motion sensor BMX055 as defined at:
// https://github.com/Azure/iot-plugandplay-models/blob/main/dtmi/nuvoton/sensor_bmx055-1.json

#ifndef PNP_MOTION_SENSOR_BMX055_CONTROLLER_H
#define PNP_MOTION_SENSOR_BMX055_CONTROLLER_H

#include "parson.h"
#include "iothub_device_client_ll.h"

//
// Handle representing a thermostat component.
//
typedef void* PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE;

//
// PnP_MotionSensorBMX055Component_CreateHandle allocates a handle to correspond to the motion sensor BMX055.
// This operation is only for allocation and does NOT invoke any I/O operations.
//
PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE PnP_MotionSensorBMX055Component_CreateHandle(const char* componentName);

//
// PnP_MotionSensorBMX055Component_Destroy frees resources associated with pnpMotionSensorBMX055ComponentHandle.
//
void PnP_MotionSensorBMX055Component_Destroy(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle);

//
// PnP_MotionSensorBMX055Component_ProcessCommand is used to process any incoming PnP Commands, transferred via the IoTHub device method channel,
// to the given PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE.  The function returns an HTTP style return code to indicate success or failure.
//
int PnP_MotionSensorBMX055Component_ProcessCommand(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, const char *pnpCommandName, JSON_Value* commandJsonValue, unsigned char** response, size_t* responseSize);

//
// PnP_MotionSensorBMX055Component_ProcessPropertyUpdate processes an incoming property update and, if the property is in this model, will
// send a reported property acknowledging receipt of the property request from IoTHub.
//
void PnP_MotionSensorBMX055Component_ProcessPropertyUpdate(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL, const char* propertyName, JSON_Value* propertyValue, int version);

//
// PnP_MotionSensorBMX055Component_SendTelemetry sends telemetry indicating the current 9-axis motion sensor data.
//
void PnP_MotionSensorBMX055Component_SendTelemetry(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL);

#endif /* PNP_MOTION_SENSOR_BMX055_CONTROLLER_H */
