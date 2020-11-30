// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hsm_client_data.h"
#include "azure_macro_utils/macro_utils.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"

// This sample is provided for development
// For more information please see the devdoc using_custom_hsm.md
static const char* const COMMON_NAME = "custom-hsm-example";
static const char* const CERTIFICATE = "-----BEGIN CERTIFICATE-----""\n"
"BASE64 Encoded certificate Here""\n"
"-----END CERTIFICATE-----";
static const char* const PRIVATE_KEY = "-----BEGIN PRIVATE KEY-----""\n"
"BASE64 Encoded certificate Here""\n"
"-----END PRIVATE KEY-----";

// Provided for sample only
static const char* const SYMMETRIC_KEY = "Symmetric Key value";
static const char* const REGISTRATION_NAME = "Registration Name";

// Provided for sample only, canned values
static const unsigned char EK[] = { 0x45, 0x6e, 0x64, 0x6f, 0x72, 0x73, 0x65, 0x6d, 0x65, 0x6e, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x0d, 0x0a };
static const size_t EK_LEN = sizeof(EK)/sizeof(EK[0]);
static const unsigned char SRK[] = { 0x53, 0x74, 0x6f, 0x72, 0x65, 0x20, 0x72, 0x6f, 0x6f, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x0d, 0x0a };
static const size_t SRK_LEN = sizeof(SRK) / sizeof(SRK[0]);
static const unsigned char ENCRYPTED_DATA[] = { 0x45, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x65, 0x64, 0x20, 0x64, 0x61, 0x74, 0x61, 0x0d, 0x0a };

typedef struct CUSTOM_HSM_SAMPLE_INFO_TAG
{
    const char* certificate;
    const char* common_name;
    const char* key;
    const unsigned char* endorsment_key;
    size_t ek_length;
    const unsigned char* storage_root_key;
    size_t srk_len;
    char* symm_key;
    char* registration_name;
} CUSTOM_HSM_SAMPLE_INFO;

int hsm_client_x509_init()
{
    return 0;
}

void hsm_client_x509_deinit()
{
}

int hsm_client_tpm_init()
{
    return 0;
}

void hsm_client_tpm_deinit()
{
}

HSM_CLIENT_HANDLE custom_hsm_create()
{
    HSM_CLIENT_HANDLE result;
    CUSTOM_HSM_SAMPLE_INFO* hsm_info = malloc(sizeof(CUSTOM_HSM_SAMPLE_INFO));
    if (hsm_info == NULL)
    {
        LogError("Failure allocating hsm info");
        result = NULL;
    }
    else
    {
        char* symm_key = NULL;
        char* registration_name = NULL;
        if (mallocAndStrcpy_s(&symm_key, SYMMETRIC_KEY) != 0)
        {
            LogError("Failed allocating symmetric key");
            symm_key = NULL;
        }
        else if (mallocAndStrcpy_s(&registration_name, REGISTRATION_NAME) != 0)
        {
            LogError("Failed allocating registration name");
            registration_name = NULL;
        }

        if (!symm_key || !registration_name)
        {
            if (symm_key)
            {
                free(symm_key);
                symm_key = NULL;
            }

            if (registration_name) {
                free(registration_name);
                registration_name = NULL;
            }

            if (hsm_info) {
                free(hsm_info);
                hsm_info = NULL;
            }

            result = NULL;
        }
        else
        {
            // TODO: initialize any variables here
            hsm_info->certificate = CERTIFICATE;
            hsm_info->key = PRIVATE_KEY;
            hsm_info->common_name = COMMON_NAME;
            hsm_info->endorsment_key = EK;
            hsm_info->ek_length = EK_LEN;
            hsm_info->storage_root_key = SRK;
            hsm_info->srk_len = SRK_LEN;
            hsm_info->symm_key = symm_key;
            hsm_info->registration_name = registration_name;
            result = hsm_info;
        }
    }
    return result;
}

void custom_hsm_destroy(HSM_CLIENT_HANDLE handle)
{
    if (handle != NULL)
    {
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        // Free anything that has been allocated in this module
        if (hsm_info->symm_key)
        {
            free(hsm_info->symm_key);
            hsm_info->symm_key = NULL;
        }

        if (hsm_info->registration_name)
        {
            free(hsm_info->registration_name);
            hsm_info->registration_name = NULL;
        }

        free(hsm_info);
    }
}

char* custom_hsm_get_certificate(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the certificate for the iothub sdk to free
        // this value will be sent unmodified to the tlsio
        // layer to be processed
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        size_t len = strlen(hsm_info->certificate);
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            LogError("Failure allocating certificate");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->certificate);
        }
    }
    return result;
}

char* custom_hsm_get_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the private key for the iothub sdk to free
        // this value will be sent unmodified to the tlsio
        // layer to be processed
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        size_t len = strlen(hsm_info->key);
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            LogError("Failure allocating certificate");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->key);
        }
    }
    return result;
}

char* custom_hsm_get_common_name(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the common name for the iothub sdk to free
        // this value will be sent to dps
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        size_t len = strlen(hsm_info->common_name);
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            LogError("Failure allocating certificate");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->common_name);
        }
    }
    return result;
}

int custom_hsm_get_endorsement_key(HSM_CLIENT_HANDLE handle, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == NULL)
    {
        LogError("Invalid parameter specified");
        result = MU_FAILURE;
    }
    else
    {
        // TODO: Retrieve the endorsement key and malloc the value and return
        // it to the sdk
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        if ((*key = (unsigned char*)malloc(hsm_info->ek_length)) == NULL)
        {
            LogError("Failure allocating endorsement key");
            result = MU_FAILURE;
        }
        else
        {
            memcpy(*key, hsm_info->endorsment_key, hsm_info->ek_length);
            *key_len = hsm_info->ek_length;
            result = 0;
        }
    }
    return result;
}

int custom_hsm_get_storage_root_key(HSM_CLIENT_HANDLE handle, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == NULL)
    {
        LogError("Invalid handle value specified");
        result = MU_FAILURE;
    }
    else
    {
        // TODO: Retrieve the storage root key and malloc the value and return
        // it to the sdk
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        if ((*key = (unsigned char*)malloc(hsm_info->srk_len)) == NULL)
        {
            LogError("Failure allocating storage root key");
            result = MU_FAILURE;
        }
        else
        {
            memcpy(*key, hsm_info->storage_root_key, hsm_info->srk_len);
            *key_len = hsm_info->srk_len;
            result = 0;
        }
    }
    return result;
}

int custom_hsm_sign_with_identity(HSM_CLIENT_HANDLE handle, const unsigned char* data, size_t data_len, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || data == NULL || key == NULL || key_len == NULL)
    {
        LogError("Invalid handle value specified");
        result = MU_FAILURE;
    }
    else
    {
        (void)data;
        (void)data_len;

        // TODO: Need to implement signing the data variable and malloc the key and return it to the sdk

        size_t signed_data_len = 10;
        if ((*key = (unsigned char*)malloc(signed_data_len)) == NULL)
        {
            LogError("Failure allocating storage root key");
            result = MU_FAILURE;
        }
        else
        {
            memcpy(*key, ENCRYPTED_DATA, signed_data_len);
            *key_len = signed_data_len;
            result = 0;
        }
    }
    return result;
}

int custom_hsm_activate_identity_key(HSM_CLIENT_HANDLE handle, const unsigned char* key, size_t key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == 0)
    {
        LogError("Invalid handle value specified");
        result = MU_FAILURE;
    }
    else
    {
        // Decrypt the key and store the value in the hsm
        result = 0;
    }
    return result;
}

char* custom_hsm_symm_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the symmetric key for the iothub 
        // The SDK will call free() this value
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        size_t len = strlen(hsm_info->symm_key);
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            LogError("Failure allocating certificate");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->symm_key);
        }
    }
    return result;
}

char* custom_hsm_get_registration_name(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the registration name for the iothub 
        // The SDK will call free() this value
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        size_t len = strlen(hsm_info->registration_name);
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            LogError("Failure allocating certificate");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->registration_name);
        }
    }
    return result;
}

int custom_hsm_set_key_info(HSM_CLIENT_HANDLE handle, const char* reg_name, const char* symm_key)
{
    int result;
    if (handle == NULL || reg_name == NULL || symm_key == NULL)
    {
        LogError("Invalid parameter specified handle: %p, reg_name: %p, symm_key: %p", handle, reg_name, symm_key);
        result = MU_FAILURE;
    }
    else
    {
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;

        char* temp_reg_name;
        char* temp_key;
        if (mallocAndStrcpy_s(&temp_reg_name, reg_name) != 0)
        {
            LogError("Failure allocating registration name");
            result = MU_FAILURE;
        }
        else if (mallocAndStrcpy_s(&temp_key, symm_key) != 0)
        {
            LogError("Failure allocating symmetric key");
            free(temp_reg_name);
            result = MU_FAILURE;
        }
        else
        {
            if (hsm_info->symm_key != NULL)
            {
                free(hsm_info->symm_key);
            }
            if (hsm_info->registration_name != NULL)
            {
                free(hsm_info->registration_name);
            }
            hsm_info->symm_key = temp_key;
            hsm_info->registration_name = temp_reg_name;
            result = 0;
        }
    }
    return result;
}

// Defining the v-table for the x509 hsm calls
static const HSM_CLIENT_X509_INTERFACE x509_interface =
{
    custom_hsm_create,
    custom_hsm_destroy,
    custom_hsm_get_certificate,
    custom_hsm_get_key,
    custom_hsm_get_common_name
};

// Defining the v-table for the x509 hsm calls
static const HSM_CLIENT_TPM_INTERFACE tpm_interface =
{
    custom_hsm_create,
    custom_hsm_destroy,
    custom_hsm_activate_identity_key,
    custom_hsm_get_endorsement_key,
    custom_hsm_get_storage_root_key,
    custom_hsm_sign_with_identity
};

static const HSM_CLIENT_KEY_INTERFACE symm_key_interface =
{
    custom_hsm_create,
    custom_hsm_destroy,
    custom_hsm_symm_key,
    custom_hsm_get_registration_name,
    custom_hsm_set_key_info
};

const HSM_CLIENT_TPM_INTERFACE* hsm_client_tpm_interface()
{
    // tpm interface pointer
    return &tpm_interface;
}

const HSM_CLIENT_X509_INTERFACE* hsm_client_x509_interface()
{
    // x509 interface pointer
    return &x509_interface;
}

const HSM_CLIENT_KEY_INTERFACE* hsm_client_key_interface()
{
    return &symm_key_interface;
}
