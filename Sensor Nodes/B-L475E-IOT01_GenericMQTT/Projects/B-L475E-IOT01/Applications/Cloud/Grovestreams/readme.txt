/**
  @page Grovestreams Grovestreams Cloud application

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   Description of Grovestreams Cloud application.
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


This application implements a Grovestreams Cloud HTTP client for the B-L475E-IOT01 board
using the on-board Inventek ISM43362 Wifi module connectivity,
and mbedTLS for secure network communication.

The application connects to Grovestreams IoT Cloud with the credentials provided
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

---> in Projects/Common/Grovestreams
Inc:
  grovestreams.h                     Include file for Grovestreams application.
  grovestreams_mbedtls_config.h      Application-specific mbedTLS configuration. 

Src:
  grovestreams.c                     Grovestreams HTTP application.

godaddy_comodo.pem                   List of required root CA certificates to be pasted on the board console at first launch.
                                     Grovestreams uses GoDaddy; the RTC init over HTTPS uses Comodo.

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


  - A Grovestreams account is required, along with a Component and Streams.
    For the Led state a boolean stream named "Led" must be added to the component created in Grovestreams.

    Detailed instructions:

    - Grovestreams account creation:
    Connect to http://grovestreams.com.
    Create an account (email and password).
    Log-in to grovestreams.com.
    Create an organization and enter it.
    - Component creation
    In Observation Studio, create a Component. (right-click on Components and select New).
    Give it a name (example: "iotboard"). Enter a Component ID (example: 1234).
    The application will use this component ID to publish stream values to the cloud.
    - Led stream:
    Right-click on the component, select "Edit component".
    Right-click on "Streams", select "Add / Stream / Stream (Most commonly used)"
    Enter Name: "Led", ID: "Led", Data Type: boolean type, do not change the Unit.
    The application will use the Stream ID "Led" to publish Led state values in Led stream.
    - API key:
    In the top menu, select Admin / API keys.
    In the API keys box, click on "+" to create a new key.
    Give it a name like "Put/get key", and an ID ("PUT_GET_KEY").
    In the same "Add API Key" dialog box, in Resources section, click on "Add Type"
    then select "Component/*/feed".
    Click again on "Add Type" then select type "Component/*/stream".
    For both types, check boxes only for PUT and GET actions.
    Click on Save to exit the "Add API key" box.
    Check the box for the API key just created, and click on "View secret key" button.
    Take note of the API secret key that is displayed (copy/paste to a temporary file)
    and close the box.
    The application will use this API key to publish stream values to the cloud.

    - device configuration:
    Flash the board with the compiled application. Connect to the board's USB COM port with serial terminal.
    Press the board's User button when asked if you want to update the device security parameters or credentials.
    Enter the API key noted previously.
    Enter the component ID as noted previously (do not confuse with the component's name).
    Enter the TLS security certificate (Projects\Common\Grovestreams\godaddy_comodo.pem)


@par How to use it ?

In order to make the program work, you must do the following:

 - WARNING: Before opening the project with any toolchain be sure your folder
   installation path is not too in-depth since the toolchain may report errors
   after building.
   
 - WARNING: read and write operations frequency
   Too frequent read and write operations to the cloud server may lead to some latencies or disconnections.
   The limits need to be checked and conform with the cloud provider documentation.
   See https://grovestreams.com/developers/limits.html

 - Open the IAR IDE and compile the project (see the release note for detailed 
   information about the version). Alternatively you can use the Keil uVision 
   toolchain (see the release note for detailed information about the version). 
   Alternatively you can use the System Workbench for STM32 (see the release note 
   for detailed information about the version). 

 - Program the firmware on the STM32 board: If you generated a raw binary file, 
   you can copy (or drag and drop) it from Projects\B-L475E-IOT01\Applications\Cloud\Grovestreams\EWARM\B-L475E-IOT01\Exe
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

     - Enter the Grovestreams api key created on Grovestreams web site.

     - Enter the component ID needed for Grovestreams Cloud.

     - Enter the root CA certificate for grovestreams.com and for RTC time setting.
       
   - After the parameters are configured, it is possible to change them by restarting the board 
     and pushing User button (blue button) when prompted during the initialization.

@par Connectivity test

  - By default the Grovestreams project tries to automatically connect to Grovestreams cloud
    (using the parameters set above). 

  - Push the User button (blue button) to toggle the LED value and send it to the cloud.
    After a few seconds, the application reads back the LED value from the cloud
    and changes the LED state accordingly.

  - In the case of B-L475E-IOT01 board, the boards sensor values are sent every 10-20 seconds to the cloud.

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
