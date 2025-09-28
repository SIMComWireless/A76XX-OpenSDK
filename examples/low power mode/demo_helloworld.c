/**
  ******************************************************************************
  * @file    demo_helloworld.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of helloworld.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "simcom_os.h"
#include "simcom_debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "simcom_network.h"
#include "simcom_simcard.h"
#include "simcom_mqtts_client.h"
#include "simcom_common.h"
#include "simcom_gpio.h"
#include "simcom_system.h"
#include "simcom_rtc.h"
#include "sc_power.h"
#include "A7671_V101_GPIO.h"
#include "cJSON.h"
#include <time.h>

/**
 * @defgroup HelloWorldDemo Hello World Demo
 * @brief SIMCom MQTT Hello World Demo
 * @{
 */

/** @brief HelloWorld task reference */
sTaskRef helloWorldProcesser;
/** @brief HelloWorld task stack */
static UINT8 helloWorldProcesserStack[1024*30];

/** @brief MQTT receive task reference */
static sTaskRef gMqttRecvTask = NULL;
/** @brief MQTT receive task stack */
static UINT8 gMqttRecvTaskStack[SC_DEFAULT_THREAD_STACKSIZE * 2];
/** @brief MQTT message queue reference */
static sMsgQRef urc_mqtt_msgq_1 = NULL;
/** @brief MQTT data update flag reference */
static sFlagRef mqtt_data_update  = NULL;
/** @brief MQTT update period (20 seconds) */
#define MQTT_UPDATE_PERIOD  20*200
/** @brief MQTT data timer reference */
static sTimerRef mqtt_data_timer = NULL;

/** @brief Printf buffer */
static char printf_buf[1024]={0};
extern void PrintfResp(char *format);
/** @brief Maximum print length for one line */
#define MAX_PRINT_ONE_LEN 180

#define SYSTEM_MODE_NORMAL 0
#define SYSTEM_MODE_SLEEP  1

/** @brief System mode: 0-normal, 1-sleep */
static UINT8 System_Mode = SYSTEM_MODE_NORMAL;

#define WAKEUP_GPIO_PIN  SC_MODULE_GPIO_03   //CTS
SC_GPIOConfiguration pinconfig;

/** @brief System info print macro */
#define SYSINFO(fmt, ...)    system_printf(fmt, ##__VA_ARGS__)

/**
 * @brief Print system information with line splitting
 * @param format Format string
 * @param ... Variable arguments
 */
void system_printf(const char * format, ...)
{
    va_list ap;
    int i = 0;
    int len = 0;
    char tmp[200];
    va_start(ap, format);

    memset(printf_buf,0x00,sizeof(printf_buf));

    vsnprintf(printf_buf, sizeof(printf_buf), format, ap);

    len = strlen(printf_buf);

    do
    {
        if((len-i) > MAX_PRINT_ONE_LEN)
        {
            memset(tmp,0x00,sizeof(tmp));
            memcpy(tmp,printf_buf+i,MAX_PRINT_ONE_LEN);
            PrintfResp(tmp);
            i+=MAX_PRINT_ONE_LEN;
        }
        else
        {
            memset(tmp,0x00,sizeof(tmp));
            memcpy(tmp,printf_buf+i,len-i);
            strcat(tmp,"\r\n");
            PrintfResp(tmp);
            i = len;
        }
    }while(i<len);

    va_end(ap);
}

/**
 * @brief Calculate weekday using Zeller's congruence
 * @param year Year
 * @param month Month
 * @param day Day
 * @return Weekday (0=Sunday, 6=Saturday)
 */
int zellers_weekday(int year, int month, int day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    
    int century = year / 100;
    int year_of_century = year % 100;
    
    int weekday = (day + 13*(month+1)/5 + year_of_century + 
                  year_of_century/4 + century/4 + 5*century) % 7;
    
    return (weekday + 6) % 7;
}

/**
 * @brief Check if year is leap year
 * @param year Year
 * @return 1 if leap year, 0 otherwise
 */
unsigned char is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/**
 * @brief Get days in month
 * @param year Year
 * @param month Month
 * @return Number of days in month
 */
int days_in_month(int year, int month) {
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days[month - 1];
}

/**
 * @brief Add time to RTC structure, handling overflow
 * @param rtc Pointer to t_rtc structure
 * @param add_hours Hours to add
 * @param add_minutes Minutes to add
 * @param add_seconds Seconds to add
 */
void add_time_complete(t_rtc *rtc, int add_hours, int add_minutes, int add_seconds) {

    rtc->tm_sec += add_seconds;

    if (rtc->tm_sec >= 60) {
        rtc->tm_min += rtc->tm_sec / 60;
        rtc->tm_sec %= 60;
    }

    rtc->tm_min += add_minutes;
    
    if (rtc->tm_min >= 60) {
        rtc->tm_hour += rtc->tm_min / 60;
        rtc->tm_min %= 60;
    }
    
    rtc->tm_hour += add_hours;
    
    if (rtc->tm_hour >= 24) {
        int extra_days = rtc->tm_hour / 24;
        rtc->tm_hour %= 24;
        rtc->tm_mday += extra_days;
    }
    
    while (rtc->tm_mday > days_in_month(rtc->tm_year, rtc->tm_mon)) {
        rtc->tm_mday -= days_in_month(rtc->tm_year, rtc->tm_mon);
        rtc->tm_mon++;
        
        if (rtc->tm_mon > 12) {
            rtc->tm_mon = 1;
            rtc->tm_year++;
        }
    }
}

/**
 * @brief MQTT lost connection callback
 * @param index MQTT client index
 * @param cause Lost connection cause
 */
static void app_MqttLostConnCb(int index, int cause)
{
    SYSINFO("MQTT LostConn Notice:index_%d,cause_%d", index, cause);
}

/**
 * @brief MQTT receiving task processor
 * @param arg Unused
 * @note Receives MQTT messages from queue and prints topic/payload
 */
void MqttRecvTaskProcesser(void *arg)
{
    while (1)
    {
        SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
        SCmqttData *sub_data = NULL;
        sAPI_MsgQRecv(urc_mqtt_msgq_1, &msgQ_data_recv, SC_SUSPEND);
        if ((SC_SRV_MQTT != msgQ_data_recv.msg_id) || (0 != msgQ_data_recv.arg1)
            || (NULL == msgQ_data_recv.arg3))
            continue;
        sub_data = (SCmqttData *)(msgQ_data_recv.arg3);
        SYSINFO("MQTT TEST----------index: [%d]; tpoic_len: [%d]; tpoic: [%s]", sub_data->client_index, sub_data->topic_len, sub_data->topic_P); 
        SYSINFO("MQTT TEST----------payload_len: [%d]",sub_data->payload_len);      
        SYSINFO("recieve topic: %s,topic len:%d", sub_data->topic_P , sub_data->topic_len);
        SYSINFO("recieve payload: %s,payload len:%d", sub_data->payload_P , sub_data->payload_len);
        sAPI_Free(sub_data->topic_P);
        sAPI_Free(sub_data->payload_P);
        sAPI_Free(sub_data);
    }
}

/**
 * @brief Timer callback for MQTT data update
 * @param timerArgc Timer argument
 */
void timer_callback(UINT32 timerArgc)
{
    sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_DISABLE);
    if(System_Mode == SYSTEM_MODE_NORMAL)
    {
        sAPI_FlagSet(mqtt_data_update, 0x01, SC_FLAG_OR);
    }
}

/**
 * @brief GPIO wakeup handler
 */
static void GPIO_WakeupHandler(void)
{
    sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_DISABLE);
    sAPI_TimerStop(mqtt_data_timer);
    sAPI_FlagSet(mqtt_data_update, 0x02, SC_FLAG_OR);
}

/**
 * @brief Prepare MQTT JSON package
 * @return Pointer to JSON string (must be freed by caller)
 */
static char* mqtt_package_preparation()
{
    SIMComVersion simcominfo = {0};
    RFVersion RFinfo = {0};
    CUSVersion CUSinfo = {0};
    char ImeiValue[20] = {0};
    unsigned int cpuUsedRate = 0;
    unsigned int heapFreeSize = 0;
    scRealTime Srealtime = {0};
    char RTCbuf[50] = {0};

    sAPI_SysGetVersion(&simcominfo);
    sAPI_SysGetRFVersion(&RFinfo);
    sAPI_SysGetCusVersion(&CUSinfo);
    sAPI_SysGetImei(ImeiValue);
    sAPI_GetSystemInfo(&cpuUsedRate, &heapFreeSize);
    sAPI_NetworkGetRealTime(&Srealtime);
    sprintf(RTCbuf,"20%02d-%02d-%02d %02d:%02d:%02d:%c%02d",Srealtime.year,Srealtime.mon,
                        Srealtime.day,Srealtime.hour,Srealtime.min,Srealtime.sec,Srealtime.sign,Srealtime.timezone);

    cJSON * root =  cJSON_CreateObject();

    cJSON_AddItemToObject(root, "manufacture_namestr", cJSON_CreateString(simcominfo.manufacture_namestr));
    cJSON_AddItemToObject(root, "Module_modelstr", cJSON_CreateString(simcominfo.Module_modelstr));
    cJSON_AddItemToObject(root, "Revision", cJSON_CreateString(simcominfo.Revision));
    cJSON_AddItemToObject(root, "GCAP", cJSON_CreateString(simcominfo.GCAP));
    cJSON_AddItemToObject(root, "cgmr", cJSON_CreateString(simcominfo.cgmr));
    cJSON_AddItemToObject(root, "internal_verstr", cJSON_CreateString(simcominfo.internal_verstr));
    cJSON_AddItemToObject(root, "version_tagstr", cJSON_CreateString(simcominfo.version_tagstr));
    cJSON_AddItemToObject(root, "SDK_Version", cJSON_CreateString(simcominfo.SDK_Version)); 
    cJSON_AddItemToObject(root, "rfversion1", cJSON_CreateString(RFinfo.rfversion1));
    cJSON_AddItemToObject(root, "rfversion2", cJSON_CreateString(RFinfo.rfversion2));
    cJSON_AddItemToObject(root, "cusversion", cJSON_CreateString(CUSinfo.cusversion));
    cJSON_AddItemToObject(root, "ImeiValue", cJSON_CreateString(ImeiValue));
    cJSON_AddItemToObject(root, "cpuUsedRate", cJSON_CreateNumber(cpuUsedRate));
    cJSON_AddItemToObject(root, "heapFreeSize", cJSON_CreateNumber(heapFreeSize));
    cJSON_AddItemToObject(root, "Date", cJSON_CreateString(RTCbuf));

    char *json_str = cJSON_Print(root);
    SYSINFO("%s", json_str);

    cJSON_Delete(root);

    return json_str;
}

/**
 * @brief HelloWorld main task processor
 * @param argv Unused
 */
void sTask_HelloWorldProcesser(void* argv)
{
    SCcpsiParm cpsi = { 0 };
    int state_cgreg;
    int32_t ret;
    SCAPNact SCact[2] = {0};
    static unsigned char mqtt_update_count = 0;

    SYSINFO("Task runs successfully");

    sAPI_NetworkSetCtzu(1); //Set time zone from network

    sAPI_NetworkInit(); //network init

    do {
      sAPI_TaskSleep(200);
      sAPI_NetworkGetCpsi(&cpsi);
      sAPI_NetworkGetCgreg(&state_cgreg);
      SYSINFO("NetworkMode->%s,cgreg->%d", cpsi.networkmode , state_cgreg);
    } while (strcmp(cpsi.networkmode, "LTE,Online"));

    while(1)
    {
        ret = sAPI_NetworkGetCgact(SCact);
        if(ret == 0)
        {
            for(int i=0; i<sizeof(SCact)/sizeof(SCAPNact); i++)
            {
                if(SCact[i].isdefine == 1)
                {
                    SYSINFO("cid=%d,isActived=%d,isdefine=%d",SCact[i].cid,SCact[i].isActived,SCact[i].isdefine);
                }
            }

            if(SCact[0].isActived == 1)
            {
                SYSINFO("Network is connected");
                break;
            }
            else
            {
                SYSINFO("Network is disconnected,try to connect it");
                ret = sAPI_NetworkSetCgact(1,1);
                if(ret != 0)
                {
                    SYSINFO("sAPI_NetworkSetCgact error");
                }
            }
        }
        else
        {
            SYSINFO("sAPI_NetworkGetCgact error");
        }
        sAPI_TaskSleep(100);
    }

    if(NULL == urc_mqtt_msgq_1)
    {
        if(sAPI_MsgQCreate(&urc_mqtt_msgq_1, "urc_mqtt_msgq_1", (sizeof(SIM_MSG_T)), 4, SC_FIFO) != SC_SUCCESS)
        {
            SYSINFO("message queue creat err!");
        }
    }

    if (NULL == gMqttRecvTask)
    {
        if (sAPI_TaskCreate(&gMqttRecvTask, gMqttRecvTaskStack, sizeof(gMqttRecvTaskStack), SC_DEFAULT_TASK_PRIORITY,
                            (char *)"mqttProcesser", MqttRecvTaskProcesser, (void *)0) != SC_SUCCESS)
        {
            SYSINFO("Task Create error!");
        }
    }

    if(NULL == mqtt_data_update)
    {
        if(sAPI_FlagCreate(&mqtt_data_update) != SC_SUCCESS)
        {
            SYSINFO("flag creat err!");
        }
    }

    if(NULL == mqtt_data_timer)
    {
        if(sAPI_TimerCreate(&mqtt_data_timer) != SC_SUCCESS)
        {
            SYSINFO("timer creat err!");
        }
    }

    ret = sAPI_MqttStart(-1);
    if (SC_MQTT_RESULT_SUCCESS == ret)
    {
        SYSINFO("MQTT Init Successful!");
    }
    else
    {
        SYSINFO("Init FAIL,ERRCODE = [%d]", ret);
    }

    ret = sAPI_MqttAccq(0, NULL, 0, "simcom", 0, urc_mqtt_msgq_1);
    if (SC_MQTT_RESULT_SUCCESS == ret)
    {
        SYSINFO("MQTT accquire Successful!");
    }
    else
    {
        SYSINFO("Init FAIL,ERRCODE = [%d]", ret);
    }
    sAPI_MqttConnLostCb(app_MqttLostConnCb);
    ret = sAPI_MqttConnect(0, NULL, 0, "tcp://test.mosquitto.org:1883", 60, 1, NULL, NULL);
    if (SC_MQTT_RESULT_SUCCESS == ret)
    {
        SYSINFO("MQTT connect Successful!");
    }
    else
    {
        SYSINFO("connect FAIL,ERRCODE = [%d]", ret);
    }

    ret = sAPI_MqttSub( 0, "test/download", strlen("test/download"), 1, 0);

    if (SC_MQTT_RESULT_SUCCESS == ret)
    {
        SYSINFO("MQTT Sub Successful!");
    }
    else
    {
        SYSINFO("Sub FAIL,ERRCODE = [%d]", ret);
    }

    sAPI_TimerStart(mqtt_data_timer, MQTT_UPDATE_PERIOD, 0, timer_callback, 0x00);

    pinconfig.pinDir = SC_GPIO_IN_PIN;
    pinconfig.initLv = 1;
    pinconfig.pinPull = SC_GPIO_PULLUP_ENABLE;//pull_up
    pinconfig.pinEd = SC_GPIO_FALL_EDGE;
    pinconfig.isr = NULL;
    pinconfig.wu = GPIO_WakeupHandler;

    ret = sAPI_GpioConfig(WAKEUP_GPIO_PIN, pinconfig);
    if (ret == SC_GPIORC_OK)
        SYSINFO("Config OK!");

    ret = sAPI_GpioWakeupEnable(WAKEUP_GPIO_PIN, SC_GPIO_FALL_EDGE);
    if (ret == SC_GPIORC_OK)
    {
        SYSINFO("Gpio set wake up OK !");
    }

    while(1)
    {
      static UINT32 flag = 0;
      sAPI_FlagWait(mqtt_data_update, 0x01|0x02, SC_FLAG_OR_CLEAR, &flag, SC_SUSPEND);
      //deal with the data
      char* json_data = mqtt_package_preparation();
      if(json_data == NULL)
      {
          SYSINFO("json package err");
          continue;
      }

      sAPI_MqttTopic (0, "test/upload", strlen("test/upload"));
      sAPI_MqttPayload(0, json_data, strlen(json_data));
      free(json_data);
      sAPI_MqttPub(0, 1, 60, 0, 0);

      if(flag & 0x01)
      {
          SYSINFO("publish data triggered by timer");
      }
      else if(flag & 0x02)
      {
          SYSINFO("publish data triggered by gpio");
      }

      mqtt_update_count++;
      SYSINFO("mqtt update count = %d", mqtt_update_count);

      if(mqtt_update_count < 5) //after 5 times update, enter RTC mode
      {
           if(mqtt_data_timer != NULL)
          {
              sAPI_TimerStart(mqtt_data_timer, MQTT_UPDATE_PERIOD, 0, timer_callback, 0x00);
          }

          SYSINFO("Module will go to sleep mode");
          sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_ENABLE);
      }
      else
      {
          t_rtc timeval;
          sAPI_TimerStop(mqtt_data_timer);
          ret = sAPI_MqttDisConnect(0, NULL, 0, 60);
          ret = sAPI_MqttRel(0);
          ret = sAPI_MqttStop();

          sAPI_GetRealTimeClock(&timeval);
          SYSINFO("Current RTC time:%02d-%02d-%02d %02d:%02d:%02d %02d",timeval.tm_year,
                  timeval.tm_mon,timeval.tm_mday,timeval.tm_hour,timeval.tm_min,timeval.tm_sec,timeval.tm_wday);
          
          add_time_complete(&timeval, 0, 1, 0); //add 1 minute
          sAPI_RtcSetAlarm(&timeval); //set alarm after 1 minute

          sAPI_RtcEnableAlarm(1);
          SYSINFO("Module will go to RTC mode");
          sAPI_TaskSleep(100);
          sAPI_SysPowerOff(); //enter RTC mode
      }
    }
}

/**
 * @brief HelloWorld demo task initial
 */
void sAPP_HelloWorldDemo(void)
{
    SC_STATUS status = SC_SUCCESS;

    status = sAPI_TaskCreate(&helloWorldProcesser, helloWorldProcesserStack, sizeof(helloWorldProcesserStack), 150, "helloWorldProcesser",sTask_HelloWorldProcesser,(void *)0);
    if(SC_SUCCESS != status)
    {
        sAPI_Debug("Task create fail,status = [%d]",status);
    }
}

/** @} */