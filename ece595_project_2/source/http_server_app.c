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
#include "cJSON.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
* Prototypes
******************************************************************************/
static int cgi_rtc_data(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_weather_status(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_time_schedule_status(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_process_time_schedule(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_process_zip_code(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_load_data(HTTPSRV_CGI_REQ_STRUCT *param);

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

const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = {
    {"rtcdata",     cgi_rtc_data},
    {"weatherdata", cgi_weather_status},
    {"ts_status",   cgi_time_schedule_status},
    {"ts",          cgi_process_time_schedule},
    {"zip",         cgi_process_zip_code},
    {"load_data",    cgi_load_data},
    {0, 0} // DO NOT REMOVE - last item - end of table
};

const HTTPSRV_SSI_LINK_STRUCT ssi_lnk_tbl[] = {{0, 0}};

char Working_Buffer[100];
Restricted_Interval_T R_Intervals[NUM_INTERVALS] = {0};

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

static int cgi_time_schedule_status(HTTPSRV_CGI_REQ_STRUCT *param)
{
    HTTPSRV_CGI_RES_STRUCT response;
    Time_Schedule_Status_T ts_status;
    char str[sizeof("NOT_RESTRICTED")];
    uint32_t length = 0;

    if (param->request_method != HTTPSRV_REQ_GET)
    {
        return (0);
    }

    ts_status = Get_Time_Schedule_Status();

    response.ses_handle = param->ses_handle;
    response.content_type = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code = HTTPSRV_CODE_OK;

    /* Calculate content length while saving it to buffer */

    if (NOT_RESTRICTED == ts_status)
    {
        sprintf(str, "NOT_RESTRICTED");
        length = sizeof("NOT_RESTRICTED");
    }
    else if (RESTRICTED == ts_status)
    {
        length = sprintf(str, "RESTRICTED");
        length = sizeof("RESTRICTED");
    }
    else
    {
        length = sprintf(str, "UNKNOWN");
        length = sizeof("UNKNOWN");
    }

    response.data = str;
    response.data_length = length;
    response.content_length = response.data_length;

    /* Send response */
    HTTPSRV_cgi_write(&response);
    return (response.content_length);
}

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

static int cgi_process_zip_code(HTTPSRV_CGI_REQ_STRUCT *param)
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
            Set_Zip_Code(buffer);
            free(buffer);
        }
    }

    HTTPSRV_cgi_write(&response);

    return(0);
}

static int cgi_load_data(HTTPSRV_CGI_REQ_STRUCT *param)
{
    HTTPSRV_CGI_RES_STRUCT response = {0};
    uint8_t i;
    char *string = NULL;
    cJSON *zip = NULL;
    cJSON *intervals  = NULL;
    cJSON *interval   = NULL;
    cJSON *id         = NULL;
    cJSON *days       = NULL;
    cJSON *start_time = NULL;
    cJSON *stop_time  = NULL;

    response.ses_handle = param->ses_handle;
    response.status_code = HTTPSRV_CODE_OK;

    cJSON *json = cJSON_CreateObject();
    uint32_t zip_code = Get_Zip_Code();

    Get_Restricted_Intervals(R_Intervals);

    if (NULL != json)
    {
        sprintf(Working_Buffer, "%d", zip_code);
        zip = cJSON_CreateString(Working_Buffer);

        if (NULL != zip)
        {
            cJSON_AddItemToObject(json, "zip", zip);
        }

        intervals = cJSON_CreateArray();
        if (NULL != intervals)
        {
            cJSON_AddItemToObject(json, "intervals", intervals);
            for (i = 0; i < NUM_INTERVALS; i++)
            {
                interval = cJSON_CreateObject();
                if (NULL != interval)
                {
                    cJSON_AddItemToArray(intervals, interval);

                    /*** ID ***/
                    if (-1 == R_Intervals[i].id)
                    {
                        sprintf(Working_Buffer, "-1");
                    }
                    else
                    {
                        sprintf(Working_Buffer, "%d", R_Intervals[i].id);
                    }
                    id = cJSON_CreateString(Working_Buffer);
                    if (NULL == id)
                        break;
                    cJSON_AddItemToObject(interval, "id", id);

                    /*** Days ***/
                    sprintf(Working_Buffer, "%d", R_Intervals[i].days);
                    days = cJSON_CreateString(Working_Buffer);
                    if (NULL == days)
                        break;
                    cJSON_AddItemToObject(interval, "days", days);

                    /*** Start Time ***/
                    if (-1 == R_Intervals[i].id)
                    {
                        sprintf(Working_Buffer, "");
                    }
                    else
                    {
                        sprintf(Working_Buffer, "%02d:%02d", R_Intervals[i].start.hour, R_Intervals[i].start.minute);
                    }

                    start_time = cJSON_CreateString(Working_Buffer);
                    if (NULL == start_time)
                        break;
                    cJSON_AddItemToObject(interval, "start_time", start_time);

                    /*** End Time ***/
                    if (-1 == R_Intervals[i].id)
                    {
                        sprintf(Working_Buffer, "");
                    }
                    else
                    {
                        sprintf(Working_Buffer, "%02d:%02d", R_Intervals[i].end.hour, R_Intervals[i].end.minute);
                    }

                    stop_time = cJSON_CreateString(Working_Buffer);
                    if (NULL == stop_time)
                        break;
                    cJSON_AddItemToObject(interval, "stop_time", stop_time);
                }
            }
        }

        string = cJSON_Print(json);

        if (string != NULL)
        {
            response.content_type = HTTPSRV_CONTENT_TYPE_JSON;
            response.data = string;
            response.data_length = strlen(string);
            response.content_length = response.data_length;
            HTTPSRV_cgi_write(&response);
        }
    }

    cJSON_Delete(json);
    return (response.content_length);
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
