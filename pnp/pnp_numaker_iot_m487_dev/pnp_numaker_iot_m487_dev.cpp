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

// This sample implements a PnP based NuMaker IoT M487 Dev model.  It demonstrates a complex 
// model in that the model has properties, commands, and telemetry off of the root component
// as well as subcomponents.

// The DTDL for component is at:
// https://github.com/Azure/iot-plugandplay-models/blob/main/dtmi/nuvoton/numaker_iot_m487_dev-1.json

// Standard C header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mbed port header files
#include "mbed.h"
#include "NTPClient.h"

// IoTHub Device Client and IoT core utility related header files
#include "iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"

// PnP utilities.
#include "pnp_device_client_ll.h"
#include "pnp_protocol.h"

// Headers that provide implementation for subcomponents
#include "pnp_motion_sensor_bmx055_component.h"
#include "pnp_deviceinfo_component.h"


// Values of connection / security settings read from environment variables and/or DPS runtime
PNP_DEVICE_CONFIGURATION g_pnpDeviceConfiguration;

// Amount of time to sleep between polling hub, in milliseconds.  Set to wake up every 100 milliseconds.
static unsigned int g_sleepBetweenPollsMs = 100;

// Every time the main loop wakes up, on the g_sendTelemetryPollInterval(th) pass will send a telemetry message.
// So we will send telemetry every (g_sendTelemetryPollInterval * g_sleepBetweenPollsMs) milliseconds;
static const int g_sendTelemetryPollInterval = 20;

// Whether tracing at the IoTHub client is enabled or not. 
static bool g_hubClientTraceEnabled = MBED_CONF_APP_IOTHUB_CLIENT_TRACE;

// DTMI indicating this device's ModelId.
static const char g_NuMakerIoTM487DevModelId[] = "dtmi:nuvoton:numaker_iot_m487_dev;1";

// PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE represents the motion sensor BMX055 component that is sub-component of the NuMaker IoT M487 Dev.
// Note that we do NOT have an analogous DeviceInfo component handle because there is only DeviceInfo subcomponent and its
// implementation is straightforward.
PNP_MOTIONSENSORBMX055_COMPONENT_HANDLE g_motionSensorBMX055Handle;

// Name of subcomponents that NuMaker IoT M487 Dev implements.
static const char g_motionSensorBMX055ComponentName[] = "motionSensorBMX055";
static const size_t g_motionSensorBMX055ComponentSize = sizeof(g_motionSensorBMX055ComponentName) - 1;
static const char g_deviceInfoComponentName[] = "deviceInformation";

static const char* g_modeledComponents[] = {g_motionSensorBMX055ComponentName, g_deviceInfoComponentName};
static const size_t g_numModeledComponents = sizeof(g_modeledComponents) / sizeof(g_modeledComponents[0]);

// Command implemented by the NuMakerIoTM487Dev component itself to implement reboot.
static const char g_rebootCommand[] = "reboot";

// An empty JSON body for PnP command responses
static const char g_JSONEmpty[] = "{}";
static const size_t g_JSONEmptySize = sizeof(g_JSONEmpty) - 1;

// Name of the led property as defined in this component's DTML
static const char g_ledPropertyName[] = "led";

// Name of the button1/2 property as defined in this component's DTML
static const char g_button1PropertyName[] = "button1";
static const char g_button2PropertyName[] = "button2";

// Format string for sending button telemetry
static const char g_button1TelemetryBodyFormat[] = "{\"button1\":%s}";
static const char g_button2TelemetryBodyFormat[] = "{\"button2\":%s}";

// led instance
static DigitalOut g_led(LED3);

// button1/2 instance
InterruptIn g_button1(SW2);
InterruptIn g_button2(SW3);

//
// PnP_NuMakerIoTM487DevComponent_ReportProperty_Led sends the led property to IoTHub
//
static void PnP_NuMakerIoTM487DevComponent_ReportProperty_Led(IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient)
{
    IOTHUB_CLIENT_RESULT iothubClientResult;
    STRING_HANDLE jsonToSend = NULL;

    /* Reported led state (0/1 for On/Off) */
    int led_state = !g_led;

    if ((jsonToSend = PnP_CreateReportedProperty(NULL, g_ledPropertyName, led_state ? "true" : "false")) == NULL)
    {
        LogError("Unable to build %s property", g_ledPropertyName);
    }
    else
    {
        const char* jsonToSendStr = STRING_c_str(jsonToSend);
        size_t jsonToSendStrLen = strlen(jsonToSendStr);

        if ((iothubClientResult = IoTHubDeviceClient_LL_SendReportedState(deviceClient, (const unsigned char*)jsonToSendStr, jsonToSendStrLen, NULL, NULL)) != IOTHUB_CLIENT_OK)
        {
            LogError("Unable to send reported state, error=%d", iothubClientResult);
        }
        else
        {
            LogInfo("Sending %s property to IoTHub", g_ledPropertyName);
        }
    }

    STRING_delete(jsonToSend);
}

//
// PnP_NuMakerIoTM487DevComponent_SendTelemetry_Button sends the button1/2 property to IoTHub
//
static void PnP_NuMakerIoTM487DevComponent_SendTelemetry_Button(IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient, int button_num)
{
    IOTHUB_CLIENT_RESULT iothubClientResult;
    STRING_HANDLE jsonToSend = NULL;

    const char *buttonTelemetryBodyFormat;
    const char *buttonTelemetryData;

    if (button_num == 1) {
        buttonTelemetryBodyFormat = g_button1TelemetryBodyFormat;
        buttonTelemetryData = g_button1.read() ? "false" : "true";
    } else if (button_num == 2) {
        buttonTelemetryBodyFormat = g_button2TelemetryBodyFormat;
        buttonTelemetryData = g_button2.read() ? "false" : "true";
    } else {
        LogError("Unsupported button%d", button_num);
        return;
    }

    IOTHUB_MESSAGE_HANDLE messageHandle = NULL;
    IOTHUB_CLIENT_RESULT iothubResult;

    char buttonStringBuffer[32];

    if (snprintf(buttonStringBuffer, sizeof(buttonStringBuffer), buttonTelemetryBodyFormat, buttonTelemetryData) < 0)
    {
        LogError("snprintf of current button telemetry failed");
    }
    else if ((messageHandle = PnP_CreateTelemetryMessageHandle(NULL, buttonStringBuffer)) == NULL)
    {
        LogError("Unable to create telemetry message");
    }
    else if ((iothubResult = IoTHubDeviceClient_LL_SendEventAsync(deviceClient, messageHandle, NULL, NULL)) != IOTHUB_CLIENT_OK)
    {
        LogError("Unable to send telemetry message, error=%d", iothubResult);
    }

    IoTHubMessage_Destroy(messageHandle);
}

//
// PnP_NuMakerIoTM487DevComponent_ProcessPropertyUpdate processes an incoming property update and, if the property is in this model, will
// send a reported property acknowledging receipt of the property request from IoTHub.
//
static void PnP_NuMakerIoTM487DevComponent_ProcessPropertyUpdate_Led(IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient, const char* propertyName, JSON_Value* propertyValue, int version)
{
    if (strcmp(propertyName, g_ledPropertyName) != 0)
    {
        LogError("Property=%s was requested to be changed but is not part of the model interface definition", propertyName);
    }
    else if (json_value_get_type(propertyValue) != JSONBoolean)
    {
        LogError("JSON field %s is not a number", g_ledPropertyName);
    }
    else
    {
        int led_state = (int)json_value_get_boolean(propertyValue);

        LogInfo("Received led=%d for component=%s", led_state, "NuMaker IoT M487 Dev");

        // Update led state (0/1 for On/Off)
        g_led = !led_state;

        // Report updated led state
        PnP_NuMakerIoTM487DevComponent_ReportProperty_Led(deviceClient);
    }
}

//
// PnP_NuMakerIoTM487DevComponent_InvokeRebootCommand processes the reboot command on the root interface
//
static int PnP_NuMakerIoTM487DevComponent_InvokeRebootCommand(JSON_Value* rootValue)
{
    int result;

    if (json_value_get_type(rootValue) != JSONNumber)
    {
        LogError("Delay payload is not a number");
        result = PNP_STATUS_BAD_FORMAT;
    }
    else
    {
        // See caveats section in ../readme.md; we don't actually respect the delay value to keep the sample simple.
        int delayInSeconds = (int)json_value_get_number(rootValue);
        LogInfo("NuMaker IoT M487 Dev 'reboot' command invoked with delay=%d seconds", delayInSeconds);
        result = PNP_STATUS_SUCCESS;
        
        // Schedule reboot in specified time
        LogInfo("System will reboot in %d seconds", delayInSeconds);
        mbed_event_queue()->call_in(std::chrono::seconds(delayInSeconds), NVIC_SystemReset);
    }
    
    return result;
}

//
// SetEmptyCommandResponse sets the response to be an empty JSON.  IoT Hub wants
// legal JSON, regardless of error status, so if command implementation did not set this do so here.
//
static void SetEmptyCommandResponse(unsigned char** response, size_t* responseSize, int* result)
{
    if ((*response = (unsigned char *)calloc(1, g_JSONEmptySize)) == NULL)
    {
        LogError("Unable to allocate empty JSON response");
        *result = PNP_STATUS_INTERNAL_ERROR;
    }
    else
    {
        memcpy(*response, g_JSONEmpty, g_JSONEmptySize);
        *responseSize = g_JSONEmptySize;
        // We only overwrite the caller's result on error; otherwise leave as it was
    }
}

//
// PnP_NuMakerIoTM487DevComponent_DeviceMethodCallback is invoked by IoT SDK when a device method arrives.
//
static int PnP_NuMakerIoTM487DevComponent_DeviceMethodCallback(const char* methodName, const unsigned char* payload, size_t size, unsigned char** response, size_t* responseSize, void* userContextCallback)
{
    (void)userContextCallback;

    char* jsonStr = NULL;
    JSON_Value* rootValue = NULL;
    int result;
    unsigned const char *componentName;
    size_t componentNameSize;
    const char *pnpCommandName;

    *response = NULL;
    *responseSize = 0;

    // Parse the methodName into its PnP (optional) componentName and pnpCommandName.
    PnP_ParseCommandName(methodName, &componentName, &componentNameSize, &pnpCommandName);

    // Parse the JSON of the payload request.
    if ((jsonStr = PnP_CopyPayloadToString(payload, size)) == NULL)
    {
        LogError("Unable to allocate twin buffer");
        result = PNP_STATUS_INTERNAL_ERROR;
    }
    else if ((rootValue = json_parse_string(jsonStr)) == NULL)
    {
        LogError("Unable to parse twin JSON");
        result = PNP_STATUS_INTERNAL_ERROR;
    }
    else
    {
        if (componentName != NULL)
        {
            LogInfo("Received PnP command for component=%.*s, command=%s", (int)componentNameSize, componentName, pnpCommandName);
            if (strncmp((const char*)componentName, g_motionSensorBMX055ComponentName, g_motionSensorBMX055ComponentSize) == 0)
            {
                result = PnP_MotionSensorBMX055Component_ProcessCommand(g_motionSensorBMX055Handle, pnpCommandName, rootValue, response, responseSize);
            }
            else
            {
                LogError("PnP component=%.*s is not supported by NuMaker IoT M487 Dev", (int)componentNameSize, componentName);
                result = PNP_STATUS_NOT_FOUND;
            }
        }
        else
        {
            LogInfo("Received PnP command for NuMaker IoT M487 Dev component, command=%s", pnpCommandName);
            if (strcmp(pnpCommandName, g_rebootCommand) == 0)
            {
                result = PnP_NuMakerIoTM487DevComponent_InvokeRebootCommand(rootValue);
            }
            else
            {
                LogError("PnP command=s%s is not supported by NuMaker IoT M487 Dev", pnpCommandName);
                result = PNP_STATUS_NOT_FOUND;
            }
        }
    }

    if (*response == NULL)
    {
        SetEmptyCommandResponse(response, responseSize, &result);
    }

    json_value_free(rootValue);
    free(jsonStr);

    return result;
}

//
// PnP_NuMakerIoTM487DevComponent_ApplicationPropertyCallback is the callback function is invoked when PnP_ProcessTwinData() visits each property.
//
static void PnP_NuMakerIoTM487DevComponent_ApplicationPropertyCallback(const char* componentName, const char* propertyName, JSON_Value* propertyValue, int version, void* userContextCallback)
{
    // This sample uses the pnp_device_client.h/.c to create the IOTHUB_DEVICE_CLIENT_LL_HANDLE as well as initialize callbacks.
    // The convention used is that IOTHUB_DEVICE_CLIENT_LL_HANDLE is passed as the userContextCallback on the initial twin callback.
    // The pnp_protocol.h/.c pass this userContextCallback down to this visitor function.
    IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient = (IOTHUB_DEVICE_CLIENT_LL_HANDLE)userContextCallback;

    if (componentName == NULL)
    {
        if (strcmp(propertyName, g_ledPropertyName) == 0) {
            PnP_NuMakerIoTM487DevComponent_ProcessPropertyUpdate_Led(deviceClient, propertyName, propertyValue, version);
        } else {
            // The PnP protocol does not define a mechanism to report errors such as this to IoTHub, so 
            // the best we can do here is to log for diagnostics purposes.
            LogError("Property=%s arrived for NuMaker IoT M487 Dev component itself.  This does not support writeable properties on it (all properties are on subcomponents)", propertyName);
        }
    }
    else if (strcmp(componentName, g_motionSensorBMX055ComponentName) == 0)
    {
        PnP_MotionSensorBMX055Component_ProcessPropertyUpdate(g_motionSensorBMX055Handle, deviceClient, propertyName, propertyValue, version);
    }
    else
    {
        LogError("Component=%s is not implemented by the NuMaker IoT M487 Dev", componentName);
    }
}

//
// PnP_NuMakerIoTM487DevComponent_DeviceTwinCallback is invoked by IoT SDK when a twin - either full twin or a PATCH update - arrives.
//
static void PnP_NuMakerIoTM487DevComponent_DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char* payload, size_t size, void* userContextCallback)
{
    // Invoke PnP_ProcessTwinData to actualy process the data.  PnP_ProcessTwinData uses a visitor pattern to parse
    // the JSON and then visit each property, invoking PnP_NuMakerIoTM487DevComponent_ApplicationPropertyCallback on each element.
    if (PnP_ProcessTwinData(updateState, payload, size, g_modeledComponents, g_numModeledComponents, PnP_NuMakerIoTM487DevComponent_ApplicationPropertyCallback, userContextCallback) == false)
    {
        // If we're unable to parse the JSON for any reason (typically because the JSON is malformed or we ran out of memory)
        // there is no action we can take beyond logging.
        LogError("Unable to process twin json.  Ignoring any desired property update requests");
    }
}

//
// GetConnectionSettingsFromConfiguration reads how to connect to the IoT Hub (using 
// either a connection string or a DPS symmetric key) from the configuration.
//
static bool GetConnectionSettingsFromConfiguration()
{
#ifdef USE_PROV_MODULE_FULL
    g_pnpDeviceConfiguration.securityType = PNP_CONNECTION_SECURITY_TYPE_DPS;
    g_pnpDeviceConfiguration.u.dpsConnectionAuth.endpoint = MBED_CONF_APP_PROVISION_ENDPOINT;
    g_pnpDeviceConfiguration.u.dpsConnectionAuth.idScope = MBED_CONF_APP_PROVISION_ID_SCOPE;
    g_pnpDeviceConfiguration.u.dpsConnectionAuth.deviceId = MBED_CONF_APP_PROVISION_REGISTRATION_ID;
    g_pnpDeviceConfiguration.u.dpsConnectionAuth.deviceKey = MBED_CONF_APP_PROVISION_SYMMETRIC_KEY;
#else
    g_pnpDeviceConfiguration.securityType = PNP_CONNECTION_SECURITY_TYPE_CONNECTION_STRING;
    g_pnpDeviceConfiguration.u.connectionString = MBED_CONF_APP_IOTHUB_CONNECTION_STRING;
#endif

    return true;
}

//
// CreateDeviceClientAndAllocateComponents allocates the IOTHUB_DEVICE_CLIENT_LL_HANDLE the application will use along with sub-components
// 
static IOTHUB_DEVICE_CLIENT_LL_HANDLE CreateDeviceClientAndAllocateComponents(void)
{
    IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient = NULL;
    bool result;

    g_pnpDeviceConfiguration.deviceMethodCallback = PnP_NuMakerIoTM487DevComponent_DeviceMethodCallback;
    g_pnpDeviceConfiguration.deviceTwinCallback = PnP_NuMakerIoTM487DevComponent_DeviceTwinCallback;
    g_pnpDeviceConfiguration.enableTracing = g_hubClientTraceEnabled;
    g_pnpDeviceConfiguration.modelId = g_NuMakerIoTM487DevModelId;

    if (GetConnectionSettingsFromConfiguration() == false)
    {
        LogError("Cannot read required environment variable(s)");
        result = false;
    }
    else if ((deviceClient = PnP_CreateDeviceClientLLHandle(&g_pnpDeviceConfiguration)) == NULL)
    {
        LogError("Failure creating IotHub device client");
        result = false;
    }
    else if ((g_motionSensorBMX055Handle = PnP_MotionSensorBMX055Component_CreateHandle(g_motionSensorBMX055ComponentName)) == NULL)
    {
        LogError("Unable to create component handle for %s", g_motionSensorBMX055ComponentName);
        result = false;
    }
    else
    {
        result = true;
    }

    if (result == false)
    {
        PnP_MotionSensorBMX055Component_Destroy(g_motionSensorBMX055Handle);
        if (deviceClient != NULL)
        {
            IoTHubDeviceClient_LL_Destroy(deviceClient);
            IoTHub_Deinit();
            deviceClient = NULL;
        }
    }

    return deviceClient;
}

// Global symbol referenced by the Azure SDK's port for Mbed OS, via "extern"
NetworkInterface *_defaultSystemNetwork;

int main(void)
{
    LogInfo("Connecting to the network");

    _defaultSystemNetwork = NetworkInterface::get_default_instance();
    if (_defaultSystemNetwork == nullptr) {
        LogError("No network interface found");
        return -1;
    }

    int ret = _defaultSystemNetwork->connect();
    if (ret != 0) {
        LogError("Connection error: %d", ret);
        return -1;
    }
    LogInfo("Connection success, MAC: %s", _defaultSystemNetwork->get_mac_address());

    LogInfo("Getting time from the NTP server");

    NTPClient ntp(_defaultSystemNetwork);
    ntp.set_server("time.google.com", 123);
    time_t timestamp = ntp.get_timestamp();
    if (timestamp < 0) {
        LogError("Failed to get the current time, error: %ld", (long)timestamp);
        return -1;
    }
    LogInfo("Time: %s", ctime(&timestamp));

    rtc_init();
    rtc_write(timestamp);
    time_t rtc_timestamp = rtc_read(); // verify it's been successfully updated
    LogInfo("RTC reports %s", ctime(&rtc_timestamp));

    IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient = NULL;

    if ((deviceClient = CreateDeviceClientAndAllocateComponents()) == NULL)
    {
        LogError("Failure creating IotHub device client");
    }
    else
    {
        LogInfo("Successfully created device client.  Hit Control-C to exit program\n");

        int numberOfIterations = 0;

        // During startup, send the non-"writeable" properties.
        PnP_DeviceInfoComponent_Report_All_Properties(g_deviceInfoComponentName, deviceClient);

        // During startup, send the "writeable" properties once.
        PnP_NuMakerIoTM487DevComponent_ReportProperty_Led(deviceClient);

        while (true)
        {
            // Wake up periodically to poll.  Even if we do not plan on sending telemetry, we still need to poll periodically in order to process
            // incoming requests from the server and to do connection keep alives.
            if ((numberOfIterations % g_sendTelemetryPollInterval) == 0)
            {
                PnP_NuMakerIoTM487DevComponent_SendTelemetry_Button(deviceClient, 1);
                PnP_NuMakerIoTM487DevComponent_SendTelemetry_Button(deviceClient, 2);
                PnP_MotionSensorBMX055Component_SendTelemetry(g_motionSensorBMX055Handle, deviceClient);
            }

            IoTHubDeviceClient_LL_DoWork(deviceClient);
            ThreadAPI_Sleep(g_sleepBetweenPollsMs);
            numberOfIterations++;
        }

        // Free the memory allocated with MotionSensorBMX055Component.
        PnP_MotionSensorBMX055Component_Destroy(g_motionSensorBMX055Handle);

        // Clean up the iothub sdk handle
        IoTHubDeviceClient_LL_Destroy(deviceClient);
        // Free all the sdk subsystem
        IoTHub_Deinit();
    }

    return 0;
}
