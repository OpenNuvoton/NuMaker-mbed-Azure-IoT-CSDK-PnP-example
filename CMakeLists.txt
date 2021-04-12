# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET NuMaker-mbed-Azure-IoT-CSDK-PnP-example)

include(${MBED_PATH}/tools/cmake/app.cmake)

add_subdirectory(${MBED_PATH})
add_subdirectory(mbed-client-for-azure)
add_subdirectory(ntp-client)

add_executable(${APP_TARGET})

mbed_configure_app_target(${APP_TARGET})

project(${APP_TARGET})

target_include_directories(${APP_TARGET}
    PRIVATE
        .
        pnp/common
        pnp/pnp_temperature_controller
        drivers/sensor/COMPONENT_BMX055
)

target_sources(${APP_TARGET}
    PRIVATE
        hsm_custom/custom_hsm_example.c
        pnp/common/pnp_device_client_ll.c
        pnp/common/pnp_dps_ll.c
        pnp/common/pnp_protocol.c
        pnp/pnp_numaker_iot_m487_dev/pnp_deviceinfo_component.cpp
        pnp/pnp_numaker_iot_m487_dev/pnp_motion_sensor_bmx055_component.cpp
        pnp/pnp_numaker_iot_m487_dev/pnp_numaker_iot_m487_dev.cpp
        drivers/sensor/COMPONENT_BMX055/BMX055.cpp
)

if("NUVOTON" IN_LIST MBED_TARGET_LABELS)
    target_sources(${APP_TARGET}
        PRIVATE
            "targets/TARGET_NUVOTON/platform_entropy.cpp"
    )
endif()

target_link_libraries(${APP_TARGET}
    PRIVATE
        mbed-client-for-azure
        mbed-os
        ntp-client
        mbed-mbedtls
        mbed-netsocket
        mbed-wifi
)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
