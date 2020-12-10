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

// Standard C header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// PnP routines
#include "pnp_protocol.h"
#include "pnp_motion_sensor_bmx055_component.h"

// Core IoT SDK utilities
#include "azure_c_shared_utility/xlogging.h"

// Motion sensor BMX055 driver
#include "BMX055.h"

// Format string for sending 9-axis telemetry
static const char g_accelXTelemetryBodyFormat[] = "{\"accelX\":%.02f}";
static const char g_accelYTelemetryBodyFormat[] = "{\"accelY\":%.02f}";
static const char g_accelZTelemetryBodyFormat[] = "{\"accelZ\":%.02f}";
static const char g_gyroXTelemetryBodyFormat[] = "{\"gyroX\":%.02f}";
static const char g_gyroYTelemetryBodyFormat[] = "{\"gyroY\":%.02f}";
static const char g_gyroZTelemetryBodyFormat[] = "{\"gyroZ\":%.02f}";
static const char g_magnetXTelemetryBodyFormat[] = "{\"magnetX\":%.02f}";
static const char g_magnetYTelemetryBodyFormat[] = "{\"magnetY\":%.02f}";
static const char g_magnetZTelemetryBodyFormat[] = "{\"magnetZ\":%.02f}";

// Format string for sending chip temperature telemetry
static const char g_tempTelemetryBodyFormat[] = "{\"temperature\":%.02f}";

//
// PNP_MOTIONSENSORBMX055_COMPONENT represents motion sensor BMX055 component
//
typedef struct PNP_MOTIONSENSORBMX055_COMPONENT_TAG
{
    // Name of this component
    char componentName[PNP_MAXIMUM_COMPONENT_LENGTH + 1];

    // 9-axis motion sensor data
    BMX055_ACCEL_TypeDef  accel;
    BMX055_GYRO_TypeDef   gyro;
    BMX055_MAGNET_TypeDef magnet;
    
    // Chip temperature
    float temp;
}
PNP_MOTIONSENSORBMX055_COMPONENT;

// Instance of motion sensor BMX055
BMX055 g_bmx055(PD_0, PD_1);

// Send telemetry: one axis
static void SendTelemetry_OneAxisOrTemp(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL, const char *telemetryBodyFormat, float telemetryData)
{
    PNP_MOTIONSENSORBMX055_COMPONENT* pnpMotionSensorBMX055Component = (PNP_MOTIONSENSORBMX055_COMPONENT*)pnpMotionSensorBMX055ComponentHandle;
    IOTHUB_MESSAGE_HANDLE messageHandle = NULL;
    IOTHUB_CLIENT_RESULT iothubResult;

    char axisStringBuffer[32];

    if (snprintf(axisStringBuffer, sizeof(axisStringBuffer), telemetryBodyFormat, telemetryData) < 0)
    {
        LogError("snprintf of current 9-axis telemetry failed");
    }
    else if ((messageHandle = PnP_CreateTelemetryMessageHandle(pnpMotionSensorBMX055Component->componentName, axisStringBuffer)) == NULL)
    {
        LogError("Unable to create telemetry message");
    }
    else if ((iothubResult = IoTHubDeviceClient_LL_SendEventAsync(deviceClientLL, messageHandle, NULL, NULL)) != IOTHUB_CLIENT_OK)
    {
        LogError("Unable to send telemetry message, error=%d", iothubResult);
    }

    IoTHubMessage_Destroy(messageHandle);
}

// Send telemetry: acceleration x/y/z
static void SendTelemetry_Accel(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    PNP_MOTIONSENSORBMX055_COMPONENT* pnpMotionSensorBMX055Component = (PNP_MOTIONSENSORBMX055_COMPONENT*)pnpMotionSensorBMX055ComponentHandle;

    g_bmx055.get_accel(&pnpMotionSensorBMX055Component->accel);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_accelXTelemetryBodyFormat, pnpMotionSensorBMX055Component->accel.x);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_accelYTelemetryBodyFormat, pnpMotionSensorBMX055Component->accel.y);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_accelZTelemetryBodyFormat, pnpMotionSensorBMX055Component->accel.z);
}

// Send telemetry: gyroscope x/y/z
static void SendTelemetry_Gyro(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    PNP_MOTIONSENSORBMX055_COMPONENT* pnpMotionSensorBMX055Component = (PNP_MOTIONSENSORBMX055_COMPONENT*)pnpMotionSensorBMX055ComponentHandle;

    g_bmx055.get_gyro(&pnpMotionSensorBMX055Component->gyro);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_gyroXTelemetryBodyFormat, pnpMotionSensorBMX055Component->gyro.x);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_gyroYTelemetryBodyFormat, pnpMotionSensorBMX055Component->gyro.y);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_gyroZTelemetryBodyFormat, pnpMotionSensorBMX055Component->gyro.z);
}

// Send telemetry: magnetometer x/y/z
static void SendTelemetry_Magnet(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    PNP_MOTIONSENSORBMX055_COMPONENT* pnpMotionSensorBMX055Component = (PNP_MOTIONSENSORBMX055_COMPONENT*)pnpMotionSensorBMX055ComponentHandle;

    g_bmx055.get_magnet(&pnpMotionSensorBMX055Component->magnet);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_magnetXTelemetryBodyFormat, pnpMotionSensorBMX055Component->magnet.x);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_magnetYTelemetryBodyFormat, pnpMotionSensorBMX055Component->magnet.y);
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_magnetZTelemetryBodyFormat, pnpMotionSensorBMX055Component->magnet.z);
}

// Send telemetry: temperature
static void SendTelemetry_Temp(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    PNP_MOTIONSENSORBMX055_COMPONENT* pnpMotionSensorBMX055Component = (PNP_MOTIONSENSORBMX055_COMPONENT*)pnpMotionSensorBMX055ComponentHandle;

    pnpMotionSensorBMX055Component->temp = g_bmx055.get_chip_temperature();
    SendTelemetry_OneAxisOrTemp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL, g_tempTelemetryBodyFormat, pnpMotionSensorBMX055Component->temp);
}

PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE PnP_MotionSensorBMX055Component_CreateHandle(const char* componentName)
{
    if (g_bmx055.chip_ready() == 0)
    {
        LogError("Bosch BMX055 is NOT available!!\r\n");
        return NULL;
    }

    PNP_MOTIONSENSORBMX055_COMPONENT* motionSensorBMX055Component;
    
    if (strlen(componentName) > PNP_MAXIMUM_COMPONENT_LENGTH)
    {
        LogError("componentName=%s is too long.  Maximum length is=%d", componentName, PNP_MAXIMUM_COMPONENT_LENGTH);
        motionSensorBMX055Component = NULL;
    }
    else if ((motionSensorBMX055Component = (PNP_MOTIONSENSORBMX055_COMPONENT*)calloc(1, sizeof(PNP_MOTIONSENSORBMX055_COMPONENT))) == NULL)
    {
        LogError("Unable to allocate motion sensor BMX055");
    }
    else
    {
        strcpy(motionSensorBMX055Component->componentName, componentName);

        // Default 9-axis motion sensor data
        motionSensorBMX055Component->accel.x = 0.0f;
        motionSensorBMX055Component->accel.y = 0.0f;
        motionSensorBMX055Component->accel.z = 0.0f;
        motionSensorBMX055Component->gyro.x = 0.0f;
        motionSensorBMX055Component->gyro.y = 0.0f;
        motionSensorBMX055Component->gyro.z = 0.0f;
        motionSensorBMX055Component->magnet.x = 0.0f;
        motionSensorBMX055Component->magnet.y = 0.0f;
        motionSensorBMX055Component->magnet.z = 0.0f;

        // Default chip temperature
        motionSensorBMX055Component->temp = 0.0f;
    }

    return (PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE)motionSensorBMX055Component;
}

void PnP_MotionSensorBMX055Component_Destroy(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle)
{
    if (pnpMotionSensorBMX055ComponentHandle != NULL)
    {
        free(pnpMotionSensorBMX055ComponentHandle);
    }
}

int PnP_MotionSensorBMX055Component_ProcessCommand(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, const char *pnpCommandName, JSON_Value* commandJsonValue, unsigned char** response, size_t* responseSize)
{
    PNP_MOTIONSENSORBMX055_COMPONENT* pnpMotionSensorBMX055Component = (PNP_MOTIONSENSORBMX055_COMPONENT*)pnpMotionSensorBMX055ComponentHandle;
    int result;

    LogError("PnP command=%s is not supported on %s component", pnpCommandName, pnpMotionSensorBMX055Component->componentName);
    result = PNP_STATUS_NOT_FOUND;

    return result;
}

void PnP_MotionSensorBMX055Component_ProcessPropertyUpdate(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL, const char* propertyName, JSON_Value* propertyValue, int version)
{
    PNP_MOTIONSENSORBMX055_COMPONENT* pnpMotionSensorBMX055Component = (PNP_MOTIONSENSORBMX055_COMPONENT*)pnpMotionSensorBMX055ComponentHandle;

    LogError("Property=%s was requested to be changed but is not part of the %s interface definition", propertyName, pnpMotionSensorBMX055Component->componentName);
}

void PnP_MotionSensorBMX055Component_SendTelemetry(PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE pnpMotionSensorBMX055ComponentHandle, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    SendTelemetry_Accel(pnpMotionSensorBMX055ComponentHandle, deviceClientLL);
    //SendTelemetry_Gyro(pnpMotionSensorBMX055ComponentHandle, deviceClientLL);
    //SendTelemetry_Magnet(pnpMotionSensorBMX055ComponentHandle, deviceClientLL);
    SendTelemetry_Temp(pnpMotionSensorBMX055ComponentHandle, deviceClientLL);
}
