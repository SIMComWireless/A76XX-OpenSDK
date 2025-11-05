/**
  ******************************************************************************
  * @file    demo_httppost.c
  * @author  SIMCom OpenSDK Team (modified)
  * @brief   HTTPS POST demo optimized.
  *******************************************************************************
  */

#include "simcom_os.h"
#include "simcom_debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "simcom_network.h"
#include "simcom_simcard.h"
#include "simcom_common.h"
#include "simcom_uart.h"
#include "simcom_system.h"
#include "A7671_V101_GPIO.h"
#include "simcom_https.h"
#include "cJSON.h"
#include "simcom_ssl.h"
#include "simcom_file.h"

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

static char ca[] =

"-----BEGIN CERTIFICATE-----\r\n"
"MIIDejCCAmKgAwIBAgIQf+UwvzMTQ77dghYQST2KGzANBgkqhkiG9w0BAQsFADBX\r\n"
"MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\r\n"
"CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIzMTEx\r\n"
"NTAzNDMyMVoXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\r\n"
"GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFI0\r\n"
"MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE83Rzp2iLYK5DuDXFgTB7S0md+8Fhzube\r\n"
"Rr1r1WEYNa5A3XP3iZEwWus87oV8okB2O6nGuEfYKueSkWpz6bFyOZ8pn6KY019e\r\n"
"WIZlD6GEZQbR3IvJx3PIjGov5cSr0R2Ko4H/MIH8MA4GA1UdDwEB/wQEAwIBhjAd\r\n"
"BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAd\r\n"
"BgNVHQ4EFgQUgEzW63T/STaj1dj8tT7FavCUHYwwHwYDVR0jBBgwFoAUYHtmGkUN\r\n"
"l8qJUC99BM00qP/8/UswNgYIKwYBBQUHAQEEKjAoMCYGCCsGAQUFBzAChhpodHRw\r\n"
"Oi8vaS5wa2kuZ29vZy9nc3IxLmNydDAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v\r\n"
"Yy5wa2kuZ29vZy9yL2dzcjEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqG\r\n"
"SIb3DQEBCwUAA4IBAQAYQrsPBtYDh5bjP2OBDwmkoWhIDDkic574y04tfzHpn+cJ\r\n"
"odI2D4SseesQ6bDrarZ7C30ddLibZatoKiws3UL9xnELz4ct92vID24FfVbiI1hY\r\n"
"+SW6FoVHkNeWIP0GCbaM4C6uVdF5dTUsMVs/ZbzNnIdCp5Gxmx5ejvEau8otR/Cs\r\n"
"kGN+hr/W5GvT1tMBjgWKZ1i4//emhA1JG1BbPzoLJQvyEotc03lXjTaCzv8mEbep\r\n"
"8RqZ7a2CPsgRbuvTPBwcOMBBmuFeU88+FSBX6+7iP0il8b4Z0QFqIwwMHfs/L6K1\r\n"
"vepuoxtGzi4CZ68zJpiq1UvSqTbFJjtbD4seiMHl\r\n"
"-----END CERTIFICATE-----\r\n";

/** @brief HelloWorld main task reference */
sTaskRef httppostProcesser;
/** @brief HelloWorld main task stack */
static UINT8 httppostProcesserStack[1024*10];

// extern void system_printf(const char * format, ...);
// #define //SYSINFO(fmt, ...)    system_printf(fmt, ##__VA_ARGS__)

static char HTTP_POST_URL[] = "https://api.omnione.energy/api/telemetry/devices/14345f6d-8850-412e-8f6f-a1e56b91f76f";
static char AUTH_BEARER[] = "Authorization: Bearer 7a2de93af6368c8b8ab817d5fa58d295c1f48770324e7426827dbb7d7e9ec0fb";
//static char COOKIE[] = "Cookie: __cf_bm=eiGxB0DJoY.2PMsaSvlV0HVkoNUmdlB.FHhdbV_Sm4A-1760686202-1.0.1.1-gLQkSwL1ZKDSaX3nqL8qgbSR_XbgZc1CB7llsBkucuIWSQSQhJmFd0M3JPK5LU4jARuQuchkqwERTM5PAuoeKQGeBU_GLWqYFHOtT.._VII";

static sMsgQRef httpsUIResp_msgq = NULL;

/**
 * @brief Helper to set HTTP parameter and log result.
 * @param key parameter name
 * @param value parameter value
 * @return return code from sAPI_HttpPara
 */
static int http_set_para( char *key, char *value)
{
    int ret = sAPI_HttpPara(key, value);
    if (ret == SC_HTTPS_SUCCESS)
    {
        //SYSINFO("http set %s success", key);
        sAPI_Debug("http set %s success", key);
    }
    else
    {
        //SYSINFO("http set %s fail:%d", key, ret);
        sAPI_Debug("http set %s fail:%d", key, ret);
    }
        
    return ret;
}

/**
 * @brief Configure HTTP parameters (URL, headers).
 * @param url target url
 */
void HTTP_ParaForm(char *url)
{
    //SYSINFO("HTTP_ParaForm");
    sAPI_Debug("HTTP_ParaForm");
    http_set_para("URL", url);
    http_set_para("CONTENT", "application/json");
    http_set_para("USERDATA", AUTH_BEARER);
    /* Optional cookie header omitted by default; enable if required */
}

/**
 * @brief Handle HTTP action response notification from queue.
 */
void HTTP_Action(UINT32 timer_out)
{
    //SYSINFO("HTTP_Action");
    sAPI_Debug("HTTP_Action");
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    sAPI_MsgQRecv(httpsUIResp_msgq, &msgQ_data_recv, timer_out);

    SChttpApiTrans *sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);
    if (!sub_data) return;

    char str[128];
    snprintf(str, sizeof(str), "\r\nstatus-code=%ld content-length=%ld\r\n", sub_data->status_code, sub_data->action_content_len);
    //SYSINFO("%s", str);
    sAPI_Debug("%s", str);

    if (sub_data->data) sAPI_Free(sub_data->data);
    sAPI_Free(sub_data);
}

/**
 * @brief Read HTTP response body from queue and print.
 */
void HTTP_ReadAction(UINT32 timer_out)
{
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    sAPI_MsgQRecv(httpsUIResp_msgq, &msgQ_data_recv, timer_out);
    SChttpApiTrans *sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);
    if (!sub_data) return;

    sAPI_Debug("read data (len=%d): %s", sub_data->dataLen, (sub_data->data ? sub_data->data : NULL));

    if (sub_data->data) sAPI_Free(sub_data->data);
    sAPI_Free(sub_data);
}

static void show_httphead_urc(UINT32 timer_out)
{
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    SChttpApiTrans *sub_data = NULL;

    sAPI_MsgQRecv(httpsUIResp_msgq, &msgQ_data_recv, timer_out);

    sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);

    if (sub_data->data != NULL && sub_data->dataLen > 0)
    {
        sAPI_UartWrite(SC_UART, (UINT8 *)sub_data->data, sub_data->dataLen);
        sAPI_Debug("\rsub_data->dataLen[%d]\r\n", sub_data->dataLen);
        sAPI_Debug("\rsub_data->data[%s]\r\n", sub_data->data);
    }

    sAPI_Free(sub_data->data);
    sAPI_Free(sub_data);
}

/**
 * @brief Post JSON payload via HTTP.
 * @param data JSON string (caller owns memory; freed after posting here if needed)
 */
void HTTP_PostData(const char *data)
{
    int ret;
    if (!data) {
        //SYSINFO("HTTP_PostData: null data");
        sAPI_Debug("HTTP_PostData: null data");
        return;
    }

    //SYSINFO("HTTP_PostData len=%zu", strlen(data));
    sAPI_Debug("HTTP_PostData len=%zu", strlen(data));
    sAPI_Debug("body:%s", data);
    ret = sAPI_HttpData((char *)data, strlen(data)); /* API expects non-const */
    if(ret != SC_HTTPS_SUCCESS)
    {
        //SYSINFO("http set data fail:%d", ret);
        sAPI_Debug("http set data fail:%d", ret);
        return;
    }

    ret = sAPI_HttpAction(1);   /* 1: POST */
    if(ret == SC_HTTPS_SUCCESS)
    {
        //SYSINFO("http action success");
        sAPI_Debug("http action success");
        HTTP_Action(SC_SUSPEND);
    }
    else
    {
        //SYSINFO("http action fail:%d", ret);
        sAPI_Debug("http action fail:%d", ret);
        HTTP_Action(1);
        return;
    }

    ret = sAPI_HttpHead();
    if (ret == SC_HTTPS_SUCCESS)
    {
        PrintfResp("\r\nhttps read head success\r\n");
        show_httphead_urc(SC_SUSPEND);
    }
    else
    {
        show_httphead_urc(1);
        PrintfResp("\r\nhttps read head fail\r\n");
        return;
    }

    ret = sAPI_HttpRead(1, 0, 512); /* read data */
    if(ret == SC_HTTPS_SUCCESS)
    {
        //SYSINFO("http read success");
        sAPI_Debug("http read success");
        HTTP_ReadAction(SC_SUSPEND);
    }
    else
    {
        //SYSINFO("http read fail:%d", ret);
        HTTP_ReadAction(1);
        sAPI_Debug("http read fail:%d", ret);
        return;
    }
}

/**
 * @brief Initialize HTTPS context, certificate and message queue.
 */
void HTTP_INIT(void)
{
    SC_STATUS status;
    int32_t ret;

    status = sAPI_MsgQCreate(&httpsUIResp_msgq, "httpsUIResp_msgq", sizeof(SIM_MSG_T), 8, SC_FIFO);
    if (SC_SUCCESS != status)
    {
        //SYSINFO("msgQ create fail:%d", status);
        sAPI_Debug("msgQ create fail:%d", status);
        return;
    }

    ret = sAPI_SslSetContextIdMsg("authmode",0,"1"); /* verify server cert */
    //SYSINFO("set ssl authmode %d", ret);
    sAPI_Debug("set ssl authmode %d", ret);

    //ret = sAPI_SslSetContextIdMsg("cacert",0,"c:/ca.pem");cabuf
    ret = sAPI_SslSetContextIdMsg("cabuf",0,ca);
    //SYSINFO("set ssl ca %d", ret);
    sAPI_Debug("set ssl ca %d", ret);

    ret = sAPI_SslSetContextIdMsg("sslversion",0,"3"); /* all version */
    //SYSINFO("sslversion result = %d", ret);
    sAPI_Debug("sslversion result = %d", ret);

    ret = sAPI_SslSetContextIdMsg("enableSNI",0,"1"); /* enable SNI */
    //SYSINFO("enableSNI result = %d", ret);
    sAPI_Debug("enableSNI result = %d", ret);

    ret = sAPI_HttpInit(1, httpsUIResp_msgq);
    //SYSINFO("https init %d", ret);
    sAPI_Debug("https init %d", ret);

    ret = sAPI_HttpPara("SSLCFG", "0");
    //SYSINFO("https set SSLCFG %d", ret);
    sAPI_Debug("https set SSLCFG %d", ret);
}

/**
 * @brief Build a compact JSON payload for POST.
 * @return heap-allocated JSON string (caller must free).
 */
static char* json_package_preparation(void)
{
    scRealTime Srealtime = {0};
    char RTCbuf[64] = {0};

    sAPI_NetworkGetRealTime(&Srealtime);
    /* produce Zulu time string */
    snprintf(RTCbuf, sizeof(RTCbuf), "20%02d-%02d-%02dT%02d:%02d:%02dZ",
             Srealtime.year, Srealtime.mon, Srealtime.day,
             Srealtime.hour, Srealtime.min, Srealtime.sec);

    cJSON * root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON *data_array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "data", data_array);

    cJSON *device_obj = cJSON_CreateObject();
    cJSON_AddItemToArray(data_array, device_obj);

    cJSON_AddStringToObject(device_obj, "name", "power");
    cJSON_AddStringToObject(device_obj, "description", "The power of the device");
    cJSON_AddStringToObject(device_obj, "pointKey", "power");
    cJSON_AddStringToObject(device_obj, "value", "10.5");
    cJSON_AddStringToObject(device_obj, "unit", "kW");
    cJSON_AddStringToObject(device_obj, "phase", "L1");
    cJSON_AddStringToObject(device_obj, "energySourceType", "GRID");
    cJSON_AddStringToObject(device_obj, "timestamp", RTCbuf);

    /* Use compact representation to reduce payload size */
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);

    //SYSINFO("Generated JSON: %s\n", json_str);
    //SYSINFO("JSON length: %zu\n", strlen(json_str));
    sAPI_Debug("Generated JSON: %s\n", json_str);
    sAPI_Debug("JSON length: %zu\n", strlen(json_str));

    return json_str;
}

/**
 * @brief HTTP POST task processor.
 */
void sTask_HTTPPOSTProcesser(void* argv)
{
    SCcpsiParm cpsi = { 0 };
    int state_cgreg;
    int32_t ret;
    SCAPNact SCact[2] = {0};

    //SYSINFO("Task runs successfully");

    sAPI_NetworkSetCtzu(1); /* Set time zone from network */
    sAPI_NetworkInit(); /* network init */

    do {
        sAPI_TaskSleep(200);
        sAPI_NetworkGetCpsi(&cpsi);
        sAPI_NetworkGetCgreg(&state_cgreg);
        //SYSINFO("NetworkMode->%s,cgreg->%d", cpsi.networkmode , state_cgreg);
        sAPI_Debug("NetworkMode->%s,cgreg->%d", cpsi.networkmode , state_cgreg);
    } while (strcmp(cpsi.networkmode, "LTE,Online"));

    while (1)
    {
        ret = sAPI_NetworkGetCgact(SCact);
        if (ret == 0)
        {
            for (int i = 0; i < (int)(sizeof(SCact)/sizeof(SCAPNact)); i++)
            {
                if (SCact[i].isdefine == 1)
                {
                    //SYSINFO("cid=%d,isActived=%d,isdefine=%d", SCact[i].cid, SCact[i].isActived, SCact[i].isdefine);
                }
            }

            if (SCact[0].isActived == 1)
            {
                //SYSINFO("Network is connected");
                sAPI_Debug("Network is connected");
                break;
            }
            else
            {
                //SYSINFO("Network is disconnected, try to connect it");
                ret = sAPI_NetworkSetCgact(1,1);
                if (ret != 0)
                {
                    //SYSINFO("sAPI_NetworkSetCgact error:%d", ret);
                }
            }
        }
        else
        {
            //SYSINFO("sAPI_NetworkGetCgact error:%d", ret);
        }
        sAPI_TaskSleep(100);
    }

    sAPI_TaskSleep(200*3);

    HTTP_INIT();
    HTTP_ParaForm(HTTP_POST_URL);

    while (1)
    {
        char *json_data = json_package_preparation();
        if (json_data)
        {
            HTTP_PostData(json_data);
            sAPI_Free(json_data); /* free generated JSON */
        }
        else
        {
            //SYSINFO("json_package_preparation failed");
            sAPI_Debug("json_package_preparation failed");
        }
        sAPI_TaskSleep(200*5); /* 5s */
    }
}

/**
 * @brief HTTP POST demo task initialization.
 */
void sAPP_HTTPPOSTDemo(void)
{
    SC_STATUS status = SC_SUCCESS;

    status = sAPI_TaskCreate(&httppostProcesser, httppostProcesserStack, sizeof(httppostProcesserStack),
                             150, "HTTPPOSTProcesser", sTask_HTTPPOSTProcesser, (void *)0);
    if (SC_SUCCESS != status)
    {
        sAPI_Debug("Task create fail,status = [%d]", status);
    }
}