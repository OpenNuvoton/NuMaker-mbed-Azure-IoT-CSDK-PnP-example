// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PNP_DPS_LL_H
#define PNP_DPS_LL_H

#ifdef USE_PROV_MODULE_FULL

#include "iothub_device_client_ll.h"
#include "pnp_device_client_ll.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// PnP_CreateDeviceClientLLHandle_ViaDps is used to create a IOTHUB_DEVICE_CLIENT_LL_HANDLE, invoking the DPS client
// to retrieve the needed hub information.
//
// NOTE: This function will BLOCK waiting for DPS to finish provisioning.
//
IOTHUB_DEVICE_CLIENT_LL_HANDLE PnP_CreateDeviceClientLLHandle_ViaDps(const PNP_DEVICE_CONFIGURATION* pnpDeviceConfiguration);

#ifdef __cplusplus
}
#endif

#endif /* #ifdef USE_PROV_MODULE_FULL */

#endif /* PNP_DPS_LL_H */
