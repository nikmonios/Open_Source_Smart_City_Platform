/**
  ******************************************************************************
  * @file    ubidots.c
  * @author  MCD Application Team
  * @brief   Ubidots cloud.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
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
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "net.h"
#include "msg.h"
#include "http_lib.h"
#include "ubidots.h"
#include "iot_flash_config.h"
#include "main.h"

/* The init/deinit netif functions are called from cloud.c.
 * However, the application needs to reinit whenever the connectivity seems to be broken. */
extern int net_if_reinit(void * if_ctxt);

/* Private defines -----------------------------------------------------------*/
#define NET_READ_TIMEOUT  "10000"
#define MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET  3

/* Private typedef -----------------------------------------------------------*/
/* Private macro -----------------------------------------------------------*/
#define MIN(min1,min2) ((min1) < (min2))?(min1):(min2)
/* Private variables ---------------------------------------------------------*/
http_handle_t Handle;
static const  char ServerName[] = "things.ubidots.com"; /* Ubidots for Education server */
static int g_connection_needed_score;

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief send one string value to a device
  * @param handle HTTP handle open on a Ubidots server
  * @param token
  * @param device label
  * @param variable name
  * @param value
  * @retval 0 success
  *         <0 error
  */
int UBIDOTS_Send_Value_String(http_handle_t Handle, const char *Token, const char *Device, const char *Variable, const char *Value);
/**
  * @brief send one integer value to a device
  * @param handle HTTP handle open on a Ubidots server
  * @param token
  * @param device label
  * @param variable name
  * @param value
  * @retval 0 success
  *         <0 error
  */
int UBIDOTS_Send_Value_Int(http_handle_t Handle, const char *Token, const char *Device, const char *Variable, int Value);
/**
  * @brief send one floating point value to a device
  * @param handle HTTP handle open on a Ubidots server
  * @param token
  * @param device label
  * @param variable name
  * @param value
  * @retval 0 success
  *         <0 error
  */
int UBIDOTS_Send_Value_Float(http_handle_t Handle, const char *Token, const char *Device, const char *Variable, float Value);
/**
  * @brief send multiple values to multiple components streams
  * @param handle HTTP handle open on a Ubidots server
  * @param token
  * @param json_buffer NULL-terminated C-string buffer in JSON format formatted like this: 
  *              { "variable1_label": variable1_value,
  *                "variable2_label": variable2_value,
  *                 ... }
  * @retval 0 success
  *         <0 error
  */
int UBIDOTS_Send_Values_JSON(http_handle_t Handle, const char *Token, const char *Device, const char *JsonBuffer);
/**
  * @brief get a value from a Ubidots variable
  * @param handle HTTP handle open on an Ubidots server
  * @param security token
  * @param device label
  * @param variable name
  * @param read_buffer response from server
  * @retval >0 size of received data
  *         <0 error
  */
int UBIDOTS_Get_Value(http_handle_t Handle, const char *Token, const char *Device, const char *Variable, char *ReadBuffer, uint32_t ReadBufferSize);
/**
  * @brief returns Ubidots token
  *        depending on what is stored in lUserConfigPtr->iot_config.device_name
  * @param none
  * @retval pointer to token recorded in Flash memory
  */
const char *UBIDOTS_Token(void);
/**
  * @brief returns component ID
  * @param none
  * @retval pointer to component ID
  */
const char *UBIDOTS_DeviceLabel(void);

/* Exported functions --------------------------------------------------------*/

int UBIDOTS_Send_Value_String(http_handle_t Handle, const char *Token, const char *Device, const char *Variable, const char *Value)
{
  char json_buffer[200];
  int ret = 0;

  memset(json_buffer, 0, sizeof(json_buffer));
  ret = snprintf(json_buffer, sizeof(json_buffer)-1,
         "{\"%s\": \"%s\"}",
         Variable,
         Value);

  ret = UBIDOTS_Send_Values_JSON(Handle,
                 Token,
                 Device,
                 json_buffer);
  msg_debug("UBIDOTS_Send_Value_String() : UBIDOTS_Send_Values_JSON() ret=%d\n", ret);
  return ret;
}

int UBIDOTS_Send_Value_Int(http_handle_t Handle, const char *Token, const char *Device, const char *Variable, int Value)
{
  char json_buffer[200];
  int ret = 0;

  memset(json_buffer, 0, sizeof(json_buffer));
  ret = snprintf(json_buffer, sizeof(json_buffer)-1,
         "{\"%s\": %d}",
         Variable,
         Value);

  ret = UBIDOTS_Send_Values_JSON(Handle,
                 Token,
                 Device,
                 json_buffer);
  msg_debug("UBIDOTS_Send_Values_JSON() ret=%d\n", ret);
  return ret;
}

int UBIDOTS_Send_Value_Float(http_handle_t Handle, const char *Token, const char *Device, const char *Variable, float Value)
{
  char json_buffer[200];
  int ret = 0;

  memset(json_buffer, 0, sizeof(json_buffer));
  ret = snprintf(json_buffer, sizeof(json_buffer)-1,
         "{\"%s\": %f}",
         Variable,
         Value);

  ret = UBIDOTS_Send_Values_JSON(Handle,
                 Token,
                 Device,
                 json_buffer);
  msg_debug("UBIDOTS_Send_Values_JSON() ret=%d\n", ret);
  return ret;
}

int UBIDOTS_Send_Values_JSON(http_handle_t Handle, const char *Token, const char *Device, const char *JsonBuffer)
{
  uint8_t response_buffer[1024];
  char query[100];
  char headers[100];
  int ret = 0;
  
  ret = snprintf(headers, sizeof(headers)-1, 
         "Content-Type: application/json\r\n"
         "X-Auth-Token: %s\r\n",
         Token);
  
  ret = snprintf(query, sizeof(query)-1,
         "/api/v1.6/devices/%s",
         Device);

  memset(response_buffer, 0, sizeof(response_buffer));
  ret = http_post(Handle,
                  query,
                  headers,
                  (const uint8_t *)JsonBuffer,  /* body */
                  strlen(JsonBuffer),   /* body length */
                  response_buffer,
                  sizeof(response_buffer));
  msg_debug("write data : http_post() ret=%d\n", ret);
  msg_debug("response buffer :\n%s\n", response_buffer);
  if (ret < 0)
  {
    if (ret != HTTP_EOF)
    {
      msg_error("error sending data to cloud ( http_post() returns %d )\n", ret);
    }
  }
  else
  {
    ret = http_response_status(response_buffer, ret);
    ret = (ret>=200 && ret <300)?0:-ret;
  }
  return ret;
}

int UBIDOTS_Get_Value(http_handle_t Handle, const char *Token, const char *Device, const char *Variable, char *ReadBuffer, uint32_t ReadBufferSize)
{
  uint8_t response_buffer[1024];
  char query[200];
  char headers[200];
  uint8_t *response_body = NULL;
  int ret = 0;
  unsigned int response_size = 0;
  uint32_t response_body_size = 0;

  ret = snprintf(headers, sizeof(headers)-1, 
         "Content-Type: application/json\r\n"
         "Content-Length: 0\r\n"
         "X-Auth-Token: %s\r\n",
         Token);

  ret = snprintf(query, sizeof(query)-1,
         "/api/v1.6/devices/%s/%s/lv",
         Device,
         Variable);

  /* read data */
  memset(response_buffer, 0, sizeof(response_buffer));
  ret = http_get(Handle, query,
                 headers,
                 response_buffer, sizeof(response_buffer));
  response_size = ret;
  msg_debug("read data : http_get() ret=%d\n", response_size);
  msg_debug("response buffer :\n%s\n", response_buffer);
  if (ret < 0)
  {
    if (ret != HTTP_EOF)
    {
      msg_error("error reading data from cloud ( http_get() returns %d)\n", ret);
    }
    return ret;
  }
  else if (ret == 0)
  {
    msg_error("response has no content\n");
    return 0;
  }
  response_body_size = response_size;
  response_body = http_find_body(response_buffer, &response_body_size);
  memcpy(ReadBuffer, (char *)response_body, MIN(response_body_size, ReadBufferSize));
  return response_body_size;
}

/**
  * @brief  Ubidots main function
  * @param  None
  * @retval None
  */
void UBIDOTS_Main(void const *arg)
{
  int ret = 0;
  uint8_t bp_pushed = 0;
  bool ledstate = false;
  bool help_banner_displayed = false;
  char read_state[100];
#ifdef SENSOR
#define TIMER_COUNT_FOR_SENSOR_PUBLISH 10
  unsigned int time_counter = TIMER_COUNT_FOR_SENSOR_PUBLISH;
  char sensors_buffer[1000];
#endif

  g_connection_needed_score = 1;

  printf("\n*** Ubidots HTTP Cloud Application ***\n\n");

  Led_SetState(false);

  /* re-connection loop */
  do
  {
    net_ipaddr_t ip;

    printf("Opening connection to server %s.\n", ServerName);

    /* If the socket connection failed MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET times in a row, 
     * even if the netif still has a valid IP address, we assume that the network link is down
     * and must be reset. */
    if ( (net_get_ip_address(hnet, &ip) == NET_ERR) || (g_connection_needed_score > MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET) )
    {
      msg_info("Network link %s down. Trying to reconnect.\n", (g_connection_needed_score > MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET) ? "may be" : "");
      if (net_reinit(hnet, (net_if_reinit)) != 0)
      {
        msg_error("Netif re-initialization failed.\n");
        continue;
      }
      else
      {
        msg_info("Netif re-initialized successfully.\n");
        HAL_Delay(1000);
        g_connection_needed_score = 1;
      }
    }
    ret = http_create_session(&Handle, ServerName, 443, HTTP_PROTO_HTTPS );
    msg_debug("http_create_session() ret=%d\n", ret);
    if (ret != NET_OK)
    {
      msg_error("error %d creating session\n", ret);
    }
    else
    {
      ret = http_sock_setopt(Handle, "tls_ca_certs", (void *)lUserConfigPtr->tls_root_ca_cert, strlen(lUserConfigPtr->tls_root_ca_cert));
      msg_debug("http_sock_setopt(\"tls_ca_certs\") ret=%d\n", ret);
      if (ret != NET_OK)
      {
        msg_error("error setting connection option (%d)\n", ret);
      }
      ret |= http_sock_setopt(Handle, "tls_server_name", (const uint8_t*)ServerName, strlen(ServerName)+1);
      msg_debug("http_sock_setopt(\"tls_server_name\") ret=%d\n", ret);
      if (ret != NET_OK)
      {
        msg_error("error setting connection option tls_server_name (%d)\n", ret);
      }
      ret |= http_sock_setopt(Handle, "sock_read_timeout", (const uint8_t*)NET_READ_TIMEOUT, strlen(NET_READ_TIMEOUT));
      if (ret != NET_OK)
      {
        msg_error("error setting socket option read timeout (%d)\n", ret);
      }
    }

    if (ret == NET_OK)
    {
      ret = http_connect(Handle);
      msg_debug("http_connect() ret=%d\n", ret);
      if ( ret == NET_OK)
      {
         printf("Connection phase complete.\n");
      }
    }
    if (ret != NET_OK)
    {
      printf("error %d opening connection to server %s\n", ret, ServerName);
      g_connection_needed_score++;
      HAL_Delay(1000);
    }
    else
    {
      /* main loop */
      if (help_banner_displayed == false)
      {
        printf("Press User button (blue button) once to toggle Led state and send to Cloud.\n");
        printf("Press User button twice to end.\n");
        help_banner_displayed = true;
      }

      bp_pushed = BP_NOT_PUSHED;
      g_connection_needed_score = 0;
      do
      {
        if (bp_pushed == BP_SINGLE_PUSH)
        {
          ledstate = !ledstate;

          printf("New LED state: %s\n", ledstate?"1":"0");
          ret = UBIDOTS_Send_Value_Int(Handle, UBIDOTS_Token(), UBIDOTS_DeviceLabel(), "led", ledstate?1:0);
          msg_debug("sending new Led state: ret = %d\n", ret);
          if (ret >= 0)
          {
            printf("Led state sent successfully to cloud.\n");
          }
          else
          {
            printf("Error sending led state to cloud: %d\n", ret);
            g_connection_needed_score++;
          }
        }
        else
        {
          ret = UBIDOTS_Get_Value(Handle, UBIDOTS_Token(), UBIDOTS_DeviceLabel(), "led", read_state, sizeof(read_state));
          msg_debug("get resource: %d - read state : %.*s\n", ret, ret, read_state);
          if (ret <= 0)
          {
            if (ret == HTTP_EOF)
            {
              msg_info("Connection closed by peer.\n");
            }
            else
            {
              msg_error("Error %d reading resource state.\n", ret);
            }
            g_connection_needed_score++;
            break;
          }
          if (strstr(read_state, "1.0") != NULL)
          {
            Led_SetState(true);
            ledstate = true;
          }
          else if (strstr(read_state, "0.0") != NULL)
          {
            Led_SetState(false);
            ledstate = false;
          }
#ifdef SENSOR
          time_counter++;
          if (time_counter > TIMER_COUNT_FOR_SENSOR_PUBLISH)
          {
            time_counter = 0;
            PrepareSensorsData(sensors_buffer, sizeof(sensors_buffer)-1, NULL);
            printf("Sending sensor values to cloud:\n%s\n", sensors_buffer);
            ret = UBIDOTS_Send_Values_JSON(Handle, UBIDOTS_Token(), UBIDOTS_DeviceLabel(), sensors_buffer);
            if (ret < 0)
            {
              if (ret == HTTP_EOF)
              {
                msg_info("Connection closed by peer.\n");
              }
              else
              {
                msg_error("Error %d sending sensors state.\n", ret);
              }
              g_connection_needed_score++;
              break;
            }
          }
#endif
        }
        bp_pushed = Button_WaitForPush(5000);
      } while ((bp_pushed != BP_MULTIPLE_PUSH) && (g_connection_needed_score == 0));
    }
    http_close(Handle);
  } while ((bp_pushed != BP_MULTIPLE_PUSH) && (g_connection_needed_score != 0));

  if (bp_pushed == BP_MULTIPLE_PUSH ) 
  {
    printf("\nDetected *double push*. End the application.\n");
  }
  
  printf("End.\n");
}

/* needed by cloud.c */
bool app_needs_device_keypair(void)
{
  return false;
}

int cloud_device_enter_credentials(void)
{
  int ret = 0;
  iot_config_t iot_config;

  memset(&iot_config, 0, sizeof(iot_config_t));
    
  printf("\nEnter Ubidots Security Token: \n");
  getInputString(iot_config.server_name, USER_CONF_SERVER_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.server_name);
  
  printf("\nEnter Device API Label: \n");
  getInputString(iot_config.device_name, USER_CONF_DEVICE_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.device_name);

  if(setIoTDeviceConfig(&iot_config) != 0)
  {
    ret = -1;
    msg_error("Failed programming the IoT device configuration to Flash.\n");
  }
  
  return ret;
}

const char *UBIDOTS_Token(void)
{
  return (const char *)lUserConfigPtr->iot_config.server_name;
}

const char *UBIDOTS_DeviceLabel(void)
{
  return (const char *)lUserConfigPtr->iot_config.device_name;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
