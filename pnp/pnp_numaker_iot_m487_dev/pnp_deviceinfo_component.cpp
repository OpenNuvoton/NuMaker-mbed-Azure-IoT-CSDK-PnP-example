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

// The DTDL for this interface is defined at:
//  https://github.com/Azure/iot-plugandplay-models/blob/main/dtmi/azure/devicemanagement/deviceinformation-1.json

// PnP routines
#include "pnp_deviceinfo_component.h"
#include "pnp_protocol.h"

// Core IoT SDK utilities
#include "azure_c_shared_utility/xlogging.h"

// Property names along with their simulated values.  
// NOTE: the property values must be legal JSON values.  Strings specifically must be enclosed with an extra set of quotes to be legal json string values.
// The property names in this sample do not hard-code the extra quotes because the underlying PnP sample adds this to names automatically.
#define PNP_ENCODE_STRING_FOR_JSON(str) #str

static const char PnPDeviceInfo_SoftwareVersionPropertyName[] = "swVersion";
static const char PnPDeviceInfo_SoftwareVersionPropertyValue[] = PNP_ENCODE_STRING_FOR_JSON("1.0.0.0");

static const char PnPDeviceInfo_ManufacturerPropertyName[] = "manufacturer";
static const char PnPDeviceInfo_ManufacturerPropertyValue[] = PNP_ENCODE_STRING_FOR_JSON("Nuvoton");

static const char PnPDeviceInfo_ModelPropertyName[] = "model";
static const char PnPDeviceInfo_ModelPropertyValue[] = PNP_ENCODE_STRING_FOR_JSON("NuMaker IoT M487 Dev");

static const char PnPDeviceInfo_OsNamePropertyName[] = "osName";
static const char PnPDeviceInfo_OsNamePropertyValue[] = PNP_ENCODE_STRING_FOR_JSON("Mbed OS");

static const char PnPDeviceInfo_ProcessorArchitecturePropertyName[] = "processorArchitecture";
static const char PnPDeviceInfo_ProcessorArchitecturePropertyValue[] = PNP_ENCODE_STRING_FOR_JSON("Cortex-M4");

static const char PnPDeviceInfo_ProcessorManufacturerPropertyName[] = "processorManufacturer";
static const char PnPDeviceInfo_ProcessorManufacturerPropertyValue[] = PNP_ENCODE_STRING_FOR_JSON("Nuvoton");

// The storage and memory fields below are doubles.  They should NOT be escaped since they will be legal JSON values when output.
static const char PnPDeviceInfo_TotalStoragePropertyName[] = "totalStorage";
static const char PnPDeviceInfo_TotalStoragePropertyValue[] = "512";

static const char PnPDeviceInfo_TotalMemoryPropertyName[] = "totalMemory";
static const char PnPDeviceInfo_TotalMemoryPropertyValue[] = "160";

//
// SendReportedPropertyForDeviceInformation sends a property as part of DeviceInfo component.
//
static void SendReportedPropertyForDeviceInformation(IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL, const char* componentName, const char* propertyName, const char* propertyValue)
{
    IOTHUB_CLIENT_RESULT iothubClientResult;
    STRING_HANDLE jsonToSend = NULL;

    if ((jsonToSend = PnP_CreateReportedProperty(componentName, propertyName, propertyValue)) == NULL)
    {
        LogError("Unable to build reported property response for propertyName=%s, propertyValue=%s", propertyName, propertyValue);
    }
    else
    {
        const char* jsonToSendStr = STRING_c_str(jsonToSend);
        size_t jsonToSendStrLen = strlen(jsonToSendStr);

        if ((iothubClientResult = IoTHubDeviceClient_LL_SendReportedState(deviceClientLL, (const unsigned char*)jsonToSendStr, jsonToSendStrLen, NULL, NULL)) != IOTHUB_CLIENT_OK)
        {
            LogError("Unable to send reported state for property=%s, error=%d", propertyName, iothubClientResult);
        }
        else
        {
            LogInfo("Sending device information property to IoTHub.  propertyName=%s, propertyValue=%s", propertyName, propertyValue);
        }
    }

    STRING_delete(jsonToSend);
}

void PnP_DeviceInfoComponent_Report_All_Properties(const char* componentName, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    // NOTE: It is possible to put multiple property updates into a single JSON and IoTHubDeviceClient_LL_SendReportedState invocation.
    // This sample does not do so for clarity, though production devices should seriously consider such property update batching to
    // save bandwidth.  PnP_CreateReportedProperty does not currently accept arrays.
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_SoftwareVersionPropertyName, PnPDeviceInfo_SoftwareVersionPropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_ManufacturerPropertyName, PnPDeviceInfo_ManufacturerPropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_ModelPropertyName, PnPDeviceInfo_ModelPropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_OsNamePropertyName, PnPDeviceInfo_OsNamePropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_ProcessorArchitecturePropertyName, PnPDeviceInfo_ProcessorArchitecturePropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_ProcessorManufacturerPropertyName, PnPDeviceInfo_ProcessorManufacturerPropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_TotalStoragePropertyName, PnPDeviceInfo_TotalStoragePropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_TotalMemoryPropertyName, PnPDeviceInfo_TotalMemoryPropertyValue);
}
