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

// This header implements a simulated DeviceInformation interface defined by dtmi:azure:DeviceManagement:DeviceInformation;1.
// This would (if actually tied into the appropriate Operating System APIs) return information about the amount of memory,
// the OS and its version, and other information about the device itself.

// DeviceInfo only reports properties defining the device and does not accept requested properties or commands.

#ifndef PNP_DEVICEINFO_COMPONENT_H
#define PNP_DEVICEINFO_COMPONENT_H

#include "iothub_device_client_ll.h"

//
// PnP_DeviceInfoComponent_Report_All_Properties sends properties corresponding to the DeviceInfo interface to the cloud.
//
void PnP_DeviceInfoComponent_Report_All_Properties(const char* componentName, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL);

#endif /* PNP_DEVICEINFO_COMPONENT_H */
