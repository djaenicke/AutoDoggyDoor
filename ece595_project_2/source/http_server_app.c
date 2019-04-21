/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "lwip/opt.h"

#if LWIP_SOCKET
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/ip.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"
#include "http_server_app.h"

#include "ethernetif.h"
#include "board.h"

#include "httpsrv.h"
#include "mdns.h"

#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "time_schedule.h"
#include "http_client_app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CGI_DATA_LENGTH_MAX (96)

/*******************************************************************************
* Prototypes
******************************************************************************/
static void cgi_urldecode(char *url);
static int cgi_rtc_data(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_weather_status(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_process_time_schedule(HTTPSRV_CGI_REQ_STRUCT *param);
static int ssi_date_time(HTTPSRV_SSI_PARAM_STRUCT *param);
static bool cgi_get_varval(char *var_str, char *var_name, char *var_val, uint32_t length);

/*******************************************************************************
* Variables
******************************************************************************/
/* FS data.*/
extern const HTTPSRV_FS_DIR_ENTRY httpsrv_fs_data[];

/*
 * Authentication users
 */
static const HTTPSRV_AUTH_USER_STRUCT users[] = {
    {"admin", "admin"}, {NULL, NULL} /* Array terminator */
};

/*
 * Authentication information.
 */
static const HTTPSRV_AUTH_REALM_STRUCT auth_realms[] = {
    {"Please use uid:admin pass:admin to login", "/auth.html", HTTPSRV_AUTH_BASIC, users},
    {NULL, NULL, HTTPSRV_AUTH_INVALID, NULL} /* Array terminator */
};

char cgi_data[CGI_DATA_LENGTH_MAX + 1];

const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = {
    {"rtcdata", cgi_rtc_data},
    {"weatherdata", cgi_weather_status},
    {"ts", cgi_process_time_schedule},
    {0, 0} // DO NOT REMOVE - last item - end of table
};

const HTTPSRV_SSI_LINK_STRUCT ssi_lnk_tbl[] = {{"date_time", ssi_date_time}, {0, 0}};

/*******************************************************************************
 * Code
 ******************************************************************************/
static int cgi_rtc_data(HTTPSRV_CGI_REQ_STRUCT *param)
{
    #define BUFF_SIZE sizeof("00:00:00")

    HTTPSRV_CGI_RES_STRUCT response;
    rtc_datetime_t datetime;
    char str[BUFF_SIZE];
    uint32_t length = 0;

    if (param->request_method != HTTPSRV_REQ_GET)
    {
        return (0);
    }

    Get_RTC_Time(&datetime);

    response.ses_handle = param->ses_handle;
    response.content_type = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code = HTTPSRV_CODE_OK;

    /*
    ** When the keep-alive is used we have to calculate a correct content length
    ** so the receiver knows when to ACK the data and continue with a next request.
    ** Please see RFC2616 section 4.4 for further details.
    */

    /* Calculate content length while saving it to buffer */
    length = snprintf(str, BUFF_SIZE, "%02d:%02d:%02d", datetime.hour, datetime.minute, datetime.second);
    response.data = str;
    response.data_length = length;
    response.content_length = response.data_length;

    /* Send response */
    HTTPSRV_cgi_write(&response);
    return (response.content_length);
}

static int cgi_weather_status(HTTPSRV_CGI_REQ_STRUCT *param)
{
    HTTPSRV_CGI_RES_STRUCT response;
    Weather_Status_T w_status;
    char str[sizeof("GOOD_WEATHER")];
    uint32_t length = 0;

    if (param->request_method != HTTPSRV_REQ_GET)
    {
        return (0);
    }

    w_status = Get_Weather_Status();

    response.ses_handle = param->ses_handle;
    response.content_type = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code = HTTPSRV_CODE_OK;


    /* Calculate content length while saving it to buffer */

    if (GOOD_WEATHER == w_status)
    {
        sprintf(str, "GOOD_WEATHER");
        length = sizeof("GOOD_WEATHER");
    }
    else
    {
        length = sprintf(str, "BAD_WEATHER");
        length = sizeof("BAD_WEATHER");
    }

    response.data = str;
    response.data_length = length;
    response.content_length = response.data_length;

    /* Send response */
    HTTPSRV_cgi_write(&response);
    return (response.content_length);
}

/* Example Common Gateway Interface callback. */
static int cgi_process_time_schedule(HTTPSRV_CGI_REQ_STRUCT *param)
{
    HTTPSRV_CGI_RES_STRUCT response = {0};
    char * buffer;

    response.ses_handle = param->ses_handle;
    response.status_code = HTTPSRV_CODE_OK;

    if (param->request_method == HTTPSRV_REQ_POST)
    {
        buffer = malloc(param->content_length);

        if (NULL != buffer)
        {
            HTTPSRV_cgi_read(param->ses_handle, buffer, param->content_length);
            Update_Restricted_Intervals(buffer);
            free(buffer);
        }
    }

    HTTPSRV_cgi_write(&response);

    return(0);
}

static bool cgi_get_varval(char *src, char *var_name, char *dst, uint32_t length)
{
    char *name;
    bool result;
    uint32_t index;
    uint32_t n_length;

    result = false;
    dst[0] = 0;
    name = src;

    n_length = strlen(var_name);

    while ((name = strstr(name, var_name)) != 0)
    {
        if (name[n_length] == '=')
        {
            name += n_length + 1;

            index = strcspn(name, "&");
            if (index >= length)
            {
                index = length - 1;
            }
            strncpy(dst, name, index);
            dst[index] = '\0';
            result = true;
            break;
        }
        else
        {
            name = strchr(name, '&');
        }
    }

    return (result);
}

/* Example Server Side Include callback. */
static int ssi_date_time(HTTPSRV_SSI_PARAM_STRUCT *param)
{
    if (strcmp(param->com_param, "time") == 0)
    {
        HTTPSRV_ssi_write(param->ses_handle, __TIME__, strlen(__TIME__));
    }
    else if (strcmp(param->com_param, "date") == 0)
    {
        HTTPSRV_ssi_write(param->ses_handle, __DATE__, strlen(__DATE__));
    }
    return (0);
}

/* Decode URL encoded string in place. */
static void cgi_urldecode(char *url)
{
    char *src = url;
    char *dst = url;

    while (*src != '\0')
    {
        if ((*src == '%') && (isxdigit((int)*(src + 1))) && (isxdigit((int)*(src + 2))))
        {
            *src = *(src + 1);
            *(src + 1) = *(src + 2);
            *(src + 2) = '\0';
            *dst++ = strtol(src, NULL, 16);
            src += 3;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}


/*!
 * @brief Callback function to generate TXT mDNS record for HTTP service.
 */
void http_srv_txt(struct mdns_service *service, void *txt_userdata)
{
    mdns_resp_add_service_txtitem(service, "path=/", 6);
}

/*!
 * @brief Initializes server.
 */
void HTTP_Server_Socket_Init(void)
{
    HTTPSRV_PARAM_STRUCT params;
    uint32_t httpsrv_handle;

    /* Init Fs*/
    HTTPSRV_FS_init(httpsrv_fs_data);

    /* Init HTTPSRV parameters.*/
    memset(&params, 0, sizeof(params));
    params.root_dir = "";
    params.index_page = "/index.html";
    params.auth_table = auth_realms;
    params.cgi_lnk_tbl = cgi_lnk_tbl;
    params.ssi_lnk_tbl = ssi_lnk_tbl;

    /* Init HTTP Server.*/
    httpsrv_handle = HTTPSRV_init(&params);
    if (httpsrv_handle == 0)
    {
        LWIP_PLATFORM_DIAG(("HTTPSRV_init() is Failed"));
    }
    else
    {
        printf("HTTP server started successfully!\r\n");
    }
}


#endif // LWIP_SOCKET
