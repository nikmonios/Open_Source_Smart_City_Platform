/**
  ******************************************************************************
  * @file    exositehttp.c
  * @author  MCD Application Team
  * @brief   Exosite cloud.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
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
#include "exositehttp.h"
#include "iot_flash_config.h"
#include "main.h"

/* The init/deinit netif functions are called from cloud.c.
 * However, the application needs to reinit whenever the connectivity seems to be broken. */
extern int net_if_reinit(void * if_ctxt);

/* Private defines -----------------------------------------------------------*/
#define NET_READ_TIMEOUT  "10000"
#define MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET  3

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
http_handle_t Handle;
static int g_connection_needed_score;

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief test Exosite device provisioning
  *        It uses device ID stored by user in lUserConfigPtr->iot_config.device_name.
  *        It prints device CIK token to be stored in same Flash memory location.
  * @param none
  * @retval none
  */
void exosite_provisioning(http_handle_t handle);
/**
  * @brief set one device resource
  * @param handle HTTP handle open on an Exosite server
  * @param name resource name
  * @param value resource value
  * @retval 0 success
  *         <0 error
  */
int exosite_set_resource(http_handle_t handle, char *name, char *value);
/**
  * @brief set multiple device resources
  * @param handle HTTP handle open on an Exosite server
  * @param resources C string with format "<resource>=<value>&<resource2>=<value2>&..."
  * @retval 0 success
  *         <0 error
  */
int exosite_set_resources(http_handle_t handle, char *resources);
/**
  * @brief get one Exosite device resource
  * @param handle HTTP handle open on an Exosite server
  * @param resource resource name
  * @param read_buffer response from server in format "<resource>=<value>"
  * @retval >0 size of received data
  *         <0 error
  */
int exosite_get_resource(http_handle_t handle, char *resource, char *read_buffer);
/**
  * @brief returns either device ID or device CIK token
  *        depending on what is stored in lUserConfigPtr->iot_config.device_name
  * @param none
  * @retval pointer to device ID or CIK token
  */
const char *exosite_device_id_token(void);
/**
  * @brief returns product ID (exosite server name for this product)
  * @param none
  * @retval pointer to product ID
  */
const char *exosite_product_id(void);
/* Exported functions --------------------------------------------------------*/

void exosite_provisioning(http_handle_t handle)
{
  char post_body[50];
  uint8_t response_buffer[1024];
  uint8_t *response_body = NULL;
  uint32_t response_size = 0;
  int ret = 0;
  /* provisioning */
  /* at that point, exosite_device_id_token() should return device id as set in Exosite cloud */
  printf("Provisioning device id: %s\n", exosite_device_id_token());
  /* curl 'https://<product_id>.m2.exosite.io/provision/activate' -H 'Content-Type: application/x-www-form-urlencoded; charset=utf-8' -d 'id=<device_id>' */
  snprintf(post_body, sizeof(post_body)-1, "id=%s", exosite_device_id_token()); /* example : "id=B-L475E-IOT01" */
  memset(response_buffer, 0, sizeof(response_buffer));
  ret = http_post(handle, "/provision/activate", "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n", 
                  (uint8_t*)post_body, strlen(post_body),
                  response_buffer, sizeof(response_buffer));
  msg_debug("provisioning : http_post() ret=%d\n", ret);
  msg_debug("response buffer :\n%s\n", response_buffer);
  response_size = ret;
  response_body = http_find_body(response_buffer, &response_size);
  printf("Token for device (CIK):\n%s\n", response_body);
  printf("Note the token, reset the device, press User button when asked to configure the cloud details, and enter device token/CIK\n");
}

int exosite_set_resources(http_handle_t handle, char *resources)
{
  uint8_t response_buffer[1024];
  char headers[200];
  int ret = 0;
  
  ret = snprintf(headers, sizeof(headers)-1, 
           "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n"
           "X-Exosite-CIK: %s\r\n",
           exosite_device_id_token());

  /* write data */
  /* for Exosite, format is "<resource>=<value>&<resource2>=<value2>&..." . Ex: "Led=true&temperature=21" */
  memset(response_buffer, 0, sizeof(response_buffer));
  ret = http_post(handle, "/onep:v1/stack/alias",
                  headers,
                  (uint8_t*)resources, strlen(resources),
                  response_buffer, sizeof(response_buffer));
  msg_debug("set resources : http_post() ret=%d\n", ret);
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

int exosite_set_resource(http_handle_t handle, char *name, char *value)
{
  uint8_t response_buffer[1024];
  char headers[200];
  char body[200];
  int ret = 0;
  
  ret = snprintf(headers, sizeof(headers)-1, 
           "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n"
           "X-Exosite-CIK: %s\r\n",
           exosite_device_id_token());
  ret = snprintf(body, sizeof(body)-1,
           "%s=%s",
           name, value);
  /* post resource value */
  memset(response_buffer, 0, sizeof(response_buffer));
  ret = http_post(handle, "/onep:v1/stack/alias",
                  headers,
                  (uint8_t*)body, strlen(body),
                  response_buffer, sizeof(response_buffer));
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

int exosite_get_resource(http_handle_t handle, char *resource, char *read_buffer)
{
  uint8_t response_buffer[1024];
  char headers[200];
  char query[100];
  uint8_t *response_body = NULL;
  int ret = 0;
  uint32_t response_size = 0;
  
  /*
GET /onep:v1/stack/alias?<alias 1>&<alias 2...>&<alias n> HTTP/1.1
Host: example.m2.exosite.io
{X-Exosite-CIK: <token>}
Accept: application/x-www-form-urlencoded; charset=utf-8
<blank line>
  */
  ret = snprintf(headers, sizeof(headers)-1,
         "Accept: application/x-www-form-urlencoded; charset=utf-8\r\n"
         "X-Exosite-CIK: %s\r\n",
         exosite_device_id_token());

  ret = snprintf(query, sizeof(query)-1,
         "/onep:v1/stack/alias?%s",
         resource);

  /* read data */
  memset(response_buffer, 0, sizeof(response_buffer));
  ret = http_get(handle, query,
                  headers,
                  response_buffer, sizeof(response_buffer));
  response_size = ret;
  msg_debug("read data : http_get() ret=%d\n", ret);
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
  response_body = http_find_body(response_buffer, &response_size);
  memcpy(read_buffer, (char *)response_body, response_size);
  read_buffer[response_size] = '\0';
  return response_size;
}

/**
  * @brief  Exosite main function
  * @param  None
  * @retval None
  */
void exosite_main(void const *arg)
{
  int ret = 0;
  uint8_t bp_pushed = 0;
  bool ledstate = false;
  bool help_banner_displayed = false;
  char read_state[50];
  char server_name[50];
#ifdef SENSOR
#define TIMER_COUNT_FOR_SENSOR_PUBLISH 10
  unsigned int time_counter = TIMER_COUNT_FOR_SENSOR_PUBLISH;
  char sensors_buffer[1000];
#endif

  g_connection_needed_score = 1;

  printf("\n***** Exosite HTTP Cloud application *****\n");
  
  snprintf(server_name, sizeof(server_name)-1,"%s.m2.exosite.io", exosite_product_id());
  
  Led_SetState(false);

  /* re-connection loop */
  do
  {
    net_ipaddr_t ip;

    printf("Opening connection to server %s.\n", server_name);

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
    ret = http_create_session(&Handle, server_name, 443, HTTP_PROTO_HTTPS );
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
      ret |= http_sock_setopt(Handle, "tls_server_name", (const uint8_t*)server_name, strlen(server_name)+1);
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
      printf("error %d opening connection to server %s\n", ret, server_name);
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

          printf("New LED state: %s\n", ledstate?"On":"Off");
          ret = exosite_set_resource(Handle, "Led", ledstate?"true":"false");
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
          ret = exosite_get_resource(Handle, "Led", read_state);
          msg_debug("get resource: %d - read state : %s\n", ret, read_state);
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
          if (strstr(read_state, "Led=true") != NULL)
          {
            Led_SetState(true);
            ledstate = true;
          }
          else if (strstr(read_state, "Led=false") != NULL)
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
            ret = exosite_set_resources(Handle, sensors_buffer);
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
        bp_pushed = Button_WaitForPush(1000);
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

  printf("\nEnter Exosite product ID: (it will be prepended to .m2.exosite.io) \n");
  getInputString(iot_config.server_name, USER_CONF_SERVER_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.server_name);

  printf("\nEnter device token (CIK): \n");
  getInputString(iot_config.device_name, USER_CONF_DEVICE_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.device_name);

  if(setIoTDeviceConfig(&iot_config) != 0)
  {
    ret = -1;
    msg_error("Failed programming the IoT device configuration to Flash.\n");
  }

  return ret;
}

/*
 * returns device id before provisioning and device CIK token after provisioning
 */
const char * exosite_device_id_token(void)
{
  return (const char *)lUserConfigPtr->iot_config.device_name;
}

const char * exosite_product_id(void)
{
  return (const char *)lUserConfigPtr->iot_config.server_name;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
