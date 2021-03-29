# Example for Azure IoT Plug and Play on Nuvoton's Mbed Enabled boards

This is an example to show [Azure IoT Plug and Play](https://docs.microsoft.com/en-us/azure/iot-pnp/) on Nuvoton's Mbed Enabled boards.
It relies on the following modules:

-   [Mbed OS](https://github.com/ARMmbed/mbed-os)
-   [Azure IoT Device SDK port for Mbed OS](https://github.com/ARMmbed/mbed-client-for-azure):
    -   [Azure IoT C SDKs and Libraries](https://github.com/Azure/azure-iot-sdk-c)
    -   [Adapters for Mbed OS](https://github.com/ARMmbed/mbed-client-for-azure/tree/master/mbed/adapters)
    -   Other dependency libraries
-   [NTP client library](https://github.com/ARMmbed/ntp-client)

This example is port of [Azure IoT Plug and Play Temperature Controller sample](https://github.com/Azure/azure-iot-sdk-c/tree/master/iothub_client/samples/pnp/pnp_temperature_controller),
which implements the model [dtmi:com:example:TemperatureController;1](https://github.com/Azure/opendigitaltwins-dtdl/blob/master/DTDL/v2/samples/TemperatureController.json).

For connection with Azure IoT Hub, it supports two authentication types.
Check [below](#compile-with-mbed-cli) for their respective configuration.

-   Connecting via [Device Provisioning Service](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-service#attestation-mechanism) with symmetric key
-   Connecting straight with symmetric key

## Support targets

Platform                        |  Connectivity     | Notes
--------------------------------|-------------------|---------------
Nuvoton NUMAKER_PFM_NUC472      | Ethernet          |
Nuvoton NUMAKER_PFM_M487        | Ethernet          |
Nuvoton NUMAKER_IOT_M487        | Wi-Fi ESP8266     |
Nuvoton NUMAKER_IOT_M263A       | Wi-Fi ESP8266     |

## Support development tools

-   [Arm's Mbed Studio](https://os.mbed.com/docs/mbed-os/v6.3/build-tools/mbed-studio.html)
-   [Arm's Mbed CLI](https://os.mbed.com/docs/mbed-os/v6.3/build-tools/mbed-cli.html)

## Developer guide

This section is intended for developers to get started, import the example application, compile with Mbed CLI, and get it running as Azure IoT Plug and Play device.

### Hardware requirements

-   Nuvoton's Mbed Enabled board

### Software requirements

-   [Arm's Mbed CLI](https://os.mbed.com/docs/mbed-os/v6.3/build-tools/mbed-cli.html)
-   [Azure account](https://portal.azure.com/)
-   [Azure IoT Explorer](https://docs.microsoft.com/en-us/azure/iot-pnp/howto-use-iot-explorer)

### Hardware setup

Connect target board to host through USB.

### Operations on Azure portal

#### Connecting via Device Provisioning Service with symmetric key

Follow the [doc](https://docs.microsoft.com/en-us/azure/iot-dps/quick-setup-auto-provision) to set up DPS on Azure portal.

For easy, choose [individual enrollment](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-service#individual-enrollment) using [symmetric key](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-service#attestation-mechanism).
Take note of the following items.

-   [Device provisioning endpoint](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-service#device-provisioning-endpoint):
    Service endpoint or global device endpoint from Provisioning service overview page
    
-   [ID scope](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-service#id-scope):
    ID Scope value from Provisioning service overview page

-   [Registration ID](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-service#registration-id):
    Registration ID provided when doing individual registration
    
-   [Symmetric key](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-service#attestation-mechanism):
    Symmetric key from individual registration detail page

#### Connecting straight with symmetric key

Follow the [doc](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal#register-a-new-device-in-the-iot-hub) to create a device using symmetric key authentication type.

### Compile with Mbed CLI

In the following, we take [NuMaker-IoT-M487](https://os.mbed.com/platforms/NUMAKER-IOT-M487/) as example board to show this example.

1.  Clone the example and navigate into it
    ```sh
    $ git clone https://github.com/OpenNuvoton/NuMaker-mbed-Azure-IoT-CSDK-PnP-example
    $ cd NuMaker-mbed-Azure-IoT-CSDK-PnP-example
    ```

1.  Deploy necessary libraries
    ```sh
    $ mbed deploy
    ```

1.  Configure connecting with IoT Hub via DPS or straight. To via DPS, set value of `use_dps` to `true`; otherwise, `null`.

    **mbed_app.json**:
    ```json
        "use_dps": {
            "help": "Enable connecting with IoT Hub via DPS",
            "options": [null, true],
            "value": true,
            "macro_name": "USE_PROV_MODULE_FULL"
        },
    ```

1.  On connecting with IoT Hub via DPS,

    1.  Configure DPS parameters. They should have been noted in [above](#operations-on-azure-portal).

        **mbed_app.json**:
        ```json
            "provision_registration_id": {
                "help": "Registration ID when HSM_TYPE_SYMM_KEY is supported; Ignored for other HSM types",
                "value": "\"REGISTRATION_ID\""
            },
            "provision_symmetric_key": {
                "help": "Symmetric key when HSM_TYPE_SYMM_KEY is supported; Ignored for other HSM types",
                "value": "\"SYMMETRIC_KEY\""
            },
            "provision_endpoint": {
                    "help": "Device provisioning service URI",
            "value": "\"global.azure-devices-provisioning.net\""
            },
            "provision_id_scope": {
                "help": "Device provisioning service ID scope",
                "value": "\"ID_SCOPE\""
            },
        ```

    1.  Enable Azure C-SDK provisioning client module and custom HSM.

        **mbed_app.json**:
        ```
        "macros": [
            "USE_PROV_MODULE",
            "HSM_AUTH_TYPE_CUSTOM"
        ],
        ```

1.  On connecting with IoT Hub straight, 

    1.  Configure connection string.

        **mbed_app.json**:
        ```json
            "iothub_connection_string": {
                "help": "Device connection string for IoT Hub authentication when DPS is not used",
                "value": "\"IOTHUB_CONNECTION_STRING\""
            },
        ```

    1.  Disable Azure C-SDK provisioning client module and custom HSM.

        **mbed_app.json**:
        <pre>
            "macros": [
                <del>"USE_PROV_MODULE",</del>
                <del>"HSM_AUTH_TYPE_CUSTOM"</del>
            ],
        </pre>

1.  Configure network interface
    -   Ethernet: Need no further configuration.
    -   WiFi: Configure WiFi `SSID`/`PASSWORD`.

        **mbed_app.json**:
        ```json
            "nsapi.default-wifi-ssid"               : "\"SSID\"",
            "nsapi.default-wifi-password"           : "\"PASSWORD\"",
        ```

1.  Build the example on **NUMAKER_IOT_M487** target and **ARM** toolchain
    ```sh
    $ mbed compile -m NUMAKER_IOT_M487 -t ARM
    ```

1.  Flash by drag-n-drop'ing the built image file below onto **NuMaker-IoT-M487** board

    `BUILD/NUMAKER_IOT_M487/ARM/NuMaker-mbed-Azure-IoT-CSDK-PnP-example.bin`

### Monitor the application through host console

Configure host terminal program with **115200/8-N-1**, and you should see log similar to below:

```
Info: Connecting to the network
Info: Connection success, MAC: a4:cf:12:b7:82:3b
Info: Getting time from the NTP server
Info: Time: Thu Nov26 3:54:25 2020

Info: RTC reports Thu Nov26 3:54:25 2020

Info: Initiating DPS client to retrieve IoT Hub connection information
Info: Provisioning callback indicates success.  iothubUri=nuvoton-test-001.azure-devices.net, deviceId=my-dps-symm-device-001
Info: DPS successfully registered.  Continuing on to creation of IoTHub device client handle.
Info: Successfully created device client.  Hit Control-C to exit program

Info: Sending serialNumber property to IoTHub
Info: Sending device information property to IoTHub.  propertyName=swVersion, propertyValue="1.0.0.0"
Info: Sending device information property to IoTHub.  propertyName=manufacturer, propertyValue="Sample-Manufacturer"
Info: Sending device information property to IoTHub.  propertyName=model, propertyValue="sample-Model-123"
Info: Sending device information property to IoTHub.  propertyName=osName, propertyValue="sample-OperatingSystem-name"
Info: Sending device information property to IoTHub.  propertyName=processorArchitecture, propertyValue="Contoso-Arch-64bit"
Info: Sending device information property to IoTHub.  propertyName=processorManufacturer, propertyValue="Processor Manufacturer(TM)"
Info: Sending device information property to IoTHub.  propertyName=totalStorage, propertyValue=10000
Info: Sending device information property to IoTHub.  propertyName=totalMemory, propertyValue=200
Info: Sending maximumTemperatureSinceLastReboot property to IoTHub for component=thermostat1
Info: Sending maximumTemperatureSinceLastReboot property to IoTHub for component=thermostat2
```

Follow [Use Azure IoT Explorer](https://docs.microsoft.com/en-us/azure/iot-pnp/howto-use-iot-explorer#use-azure-iot-explorer) to connect to your IoT Hub and interact with your device.
For example, go to the path in Azure IoT Explorer:

```
Home > MY_IOT_HUB > Devices > MY_DEVICE > IoT Plug and Play components > DEFAULT_COMPONENT > Commands
```

Issue the command `reboot` with 5s delay, and you should see:

```
Info: Received PnP command for TemperatureController component, command=reboot
Info: Temperature controller 'reboot' command invoked with delay=5 seconds
```

### Walk through source code

#### Implement one Azure IoT Plug and Play device model (`pnp/`)

This directory contains port of [Azure IoT Plug and Play Temperature Controller sample](https://github.com/Azure/azure-iot-sdk-c/tree/master/iothub_client/samples/pnp/pnp_temperature_controller),
which implements the model [dtmi:com:example:TemperatureController;1](https://github.com/Azure/opendigitaltwins-dtdl/blob/master/DTDL/v2/samples/TemperatureController.json).

#### Custom HSM (`hsm_custom/`)

[Azure C-SDK Provisioning Client](https://github.com/Azure/azure-iot-sdk-c/blob/master/provisioning_client/devdoc/using_provisioning_client.md) requires [HSM](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-service#hardware-security-module).
This directory provides one custom HSM library for development.
It is adapted from [Azure C-SDK custom hsm example](https://github.com/Azure/azure-iot-sdk-c/tree/master/provisioning_client/samples/custom_hsm_example).
If connecting with IoT Hub straight (so `HSM_AUTH_TYPE_CUSTOM` undefined above), this directory gets unnecessary and can be removed.

##### Connecting via Device Provisioning Service with symmetric key

If you connect via DPS, provide `provision_registration_id` and `provision_symmetric_key` as [above](#compile-with-mbed-cli).
During provisioning process, `SYMMETRIC_KEY` and `REGISTRATION_NAME` will be overridden through `custom_hsm_set_key_info`.
So you needn't override `SYMMETRIC_KEY` and `REGISTRATION_NAME` below.

```C
// Provided for sample only
static const char* const SYMMETRIC_KEY = "Symmetric Key value";
static const char* const REGISTRATION_NAME = "Registration Name";
```

#### Platform entropy source (`targets/TARGET_NUVOTON/platform_entropy.cpp`)

Mbedtls requires [entropy source](https://os.mbed.com/docs/mbed-os/v6.4/porting/entropy-sources.html).
On targets with `TRNG` hardware, Mbed OS has supported it.
On targets without `TRNG` hardware, substitute platform entropy source must be provided.
This directory provides one platform entropy source implementation for Nuvoton's targets without `TRNG` hardware.
