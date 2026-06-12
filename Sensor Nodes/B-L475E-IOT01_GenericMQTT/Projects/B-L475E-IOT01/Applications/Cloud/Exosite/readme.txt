/**
  @page Exosite Exosite Cloud application

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   Description of Exosite Cloud application.
  ******************************************************************************
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  @endverbatim

@par Application Description


This application implements an Exosite Cloud IoT client for the B-L475E-IOT01 board
using the on-board Inventek ISM43362 Wifi module connectivity,
and mbedTLS for secure network communication.

The application connects to Exosite IoT Cloud with the credentials provided
by the user. When the User button is pushed, it sends a LED toggle command to
the IoT Cloud endpoint, which returns back the message to the board and 
triggers the LED toggle. 

Upon a double-push of the User button, the application exits.

This application supports following build configuration:
- B-L475E-IOT01         : The LED state will be published once the User button is
                          pushed.


@par Directory contents

---> in .

Inc
  es_wifi_conf.h                     Es_Wifi configuration.
  es_wifi_io.h                       Functions prototypes for es_wifi IO operations.
  flash.h                            Management of the internal flash memory.
  main.h                             Header containing config parameters for the application and globals.
  stm32l4xx_hal_conf.h               HAL configuration file.
  stm32l4xx_it.h                     STM32 interrupt handlers header file.
  vl53l0x_platform.h                 vl53l0x proximity sensor platform include file.
  vl53l0x_proximity.h                vl53l0x proximity sensor include file.
  wifi.h                             Wifi core resources definitions.

Src
  es_wifi_io.c                       Es_Wifi IO porting.
  flash_l4.c                         Flash programming interface.
  main.c                             Main application file.
  stm32l4xx_hal_msp.c                Specific initializations.
  stm32l4xx_it.c                     STM32 interrupt handlers.
  system_stm32l4xx.c                 System initialization.
  vl53l0x_platform.c                 vl53l0x proximity sensor platform file.
  vl53l0x_proximity.c                vl53l0x proximity sensor.
  wifi.c                             WiFi-LL interface.

---> in Projects/Common/Exosite
Inc:
  exositehttp.h                      Include file for Exosite application.
  exosite_mbedtls_config.h           Application-specific mbedTLS configuration.

Src:
  exositehttp.c                      Exosite HTTP application.

DigiCert_Comodo.pem                  List of required root CA certificates to be pasted on the board console at first launch.
                                     Exosite uses DigiCert; the RTC init over HTTPS uses Comodo.

---> in Projects/Common/Shared
Inc
  cloud.h                            Cloud header file
  heap.h                             Heap check functions
  http_lib.h                         HTTP library functions
  iot_flash_config.h                 Header for configuration
  mbedtls_net.h                      MbedTLS network callbacks declaration
  msg.h                              Trace message API
  net.h                              Network API
  net_internal.h                     Network abstraction at transport layer level.
                                     Internal definitions
  rfu.h                              Remote firmare upate over TCP/IP
  sensors_data.h                     Sensors header
  timedate.h                         Configuration of the RTC
  timer.h                            Timer basic functions
  timingSystem.h                     Header file for timingSystem.c
  version.h                          STM32 FW version definition

Src
  cloud.c                             Common cloud application functions.
  entropy_hardware_poll.c             RNG entropy source for mbedTLS.
  heap.c                              Heap check functions.
  http_lib.c                          HTTP Client library.
  iot_flash_config.c                  Dialog and storage management utils for the user-configured settings.
  mbedtls_net.c                       Network adapter for mbedTLS on NET.
  net.c                               Network socket API.
  net_tcp_wifi.c                      NET TCP / WiFi-LL implementation.
  net_tls_mbedtls.c                   NET TLS / mbedTLS implementation.
  rfu.c                               Remote firmare upate over TCP/IP.
  sensors_data.c                      Board-specific file to retrieve and format sensors data.
  STM32CubeRTCInterface.c             Std. time port to the RTC.
  timedate.c                          Initialization of the RTC from the network.
  timingSystem.c                      Wrapper to STM32 timing.
  wifi_net.c                          WiFi_LL init/deinit functions.

@par Hardware and Software environment
  - B-L475E-IOT01 board.
    MCU board: B-L475E-IOT01 (MB1297 rev D), with FW "Inventek eS-WiFi ISM43362-M3G-L44-SPI C3.5.2.3.BETA9"
    Note: the FW version is displayed on the board console at boot time.

  - WiFi access point.
      * With a transparent Internet connectivity: No proxy, no firewall blocking the outgoing traffic.
      * Running a DHCP server delivering the IP and DNS configuration to the board.


  - An Exosite account is required, along with a Product and a Device.
    For the Led state a boolean resource name "Led" must be added to the device created in Exosite.
    
    see http://docs.exosite.com/
    
    Detailed instructions:
    
    - Exosite account creation:
    Connect to http://exosite.io.
    Create an account (email and password).
    Log-in to exosite.io.
    Create a Product (click on "New Solution / Add Product"). Give it a name (ex: "iotcloud").
    Leave the default settings and click "Add".
    Click on the product. Go to "Settings" tab.
    Check that the product is configured so that devices connect through HTTPS with a Token.
    For Provisioning / Device Identity Format, don't enforce a device prefix.
    Allow devices to register their own identity.
    Go back to Devices tab.
    Note the Product / Solution ID. It can be copied to the clipboard by clicking on the
    small green icon labelled "ID" on the top left of the screen.
    It is an hexadecimal string different from Product name. Paste it in a temporary text file.
    This will be needed during board configuration phase.
    - Device creation
    Go to Devices tab. Create a New Device. Enter a device identity like "device01".
    In the device list, click on the device's menu (three vertical dots on the right) and 
    select "Set authentication key". Take note of the generated key (copy/paste to a temporary file).
    This key ( = CIK token) must be entered during device configuration on serial terminal.
    - Led resource:
    On Exosite web site, go to the product's resources tab, and create a "Led" resource
    with boolean type. Tick the box "I want to be able to modify this value from the cloud".
    It is required to set the initial state of the Led resource on each device.
    Click on Devices, then select the device, then click on the "pen" (Edit) icon near 
    "Led" resource and set the Led state (either True or False).
    - Board configuration:
    Flash the board with the compiled application. Connect to the board's USB COM port with serial terminal.
    Press the board's User button when asked if you want to update the device security parameters or credentials.
    Enter the Product ID noted previously.
    Enter the device token/CIK noted previously.
    Enter the TLS security certificate (Projects\Common\Exosite\DigiCert_Comodo.pem)


@par How to use it ?

In order to make the program work, you must do the following:

 - WARNING: Before opening the project with any toolchain be sure your folder
   installation path is not too in-depth since the toolchain may report errors
   after building.

 - Open the IAR IDE and compile the project (see the release note for detailed 
   information about the version). Alternatively you can use the Keil uVision 
   toolchain (see the release note for detailed information about the version). 
   Alternatively you can use the System Workbench for STM32 (see the release note 
   for detailed information about the version). 

 - Program the firmware on the STM32 board: If you generated a raw binary file, 
   you can copy (or drag and drop) it from Projects\B-L475E-IOT01\Applications\Cloud\Exosite\EWARM\B-L475E-IOT01\Exe
   to the USB mass storage location created when you plug the STM32 
   board to your PC. If the host is a Linux PC, the STM32 device can be found in 
   the /media folder with the name e.g. "DIS_L4IOT". For example, if the created mass 
   storage location is "/media/DIS_L4IOT", then the command to program the board 
   with a binary file named "my_firmware.bin" is simply: cp my_firmware.bin 
   /media/DIS_L4IOT. 

   Alternatively, you can program the STM32 board directly through one of the 
   supported development toolchains, or thanks to the STM32 ST-Link Utility.
  
 - Configure the required settings (to be done only once): 
   - When the board is connected to a PC with USB (ST-LINK USB port), 
     open a serial terminal emulator, find the board's COM port and configure it with:
      - 8N1, 115200 bauds, no HW flow control
      - set the line endings to LF or CR-LF (Transmit) and LF (receive).

   - At first boot, enter the required parameters:

     - Wifi network configuration (SSID, security mode, password).

     - Enter the product ID needed for Exosite Cloud.

     - Enter the Exosite device token (CIK) created on Exosite web site.

     - Enter the root CA certificate for *.exosite.io and for RTC time setting.
       
   - After the parameters are configured, it is possible to change them by restarting the board 
     and pushing User button (blue button) when prompted during the initialization.

@par Connectivity test

  - By default the Exosite project tries to automatically connect to Exosite cloud
    (using the parameters set above). 

  - Push the User button (blue button) to toggle the LED value and send it to the cloud.
    After a few seconds, the application reads back the LED value from the cloud
    and changes the LED state accordingly.

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
