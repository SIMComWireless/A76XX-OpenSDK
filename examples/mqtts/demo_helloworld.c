/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-12-31 15:59:15
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2025-03-28 10:50:50
 * @FilePath: \A110B01V11A7672M7_SDK_CUS_RU_241231\SIMCOM_SDK_SET\sc_demo\V1\src\demo_helloworld.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/**
  ******************************************************************************
  * @file    demo_helloworld.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of helloworld.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <stdarg.h>
#include "simcom_os.h"
#include "simcom_debug.h"
#include "simcom_ssl.h"
#include "simcom_mqtts_client.h"
#include "simcom_file.h"
#include "simcom_ftps.h"
#include "sc_flash.h"
#include "simcom_common.h"
#include "simcom_network.h"

char *ca_cert =
        "-----BEGIN CERTIFICATE-----\r\n"
        "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\r\n"
        "-----END CERTIFICATE-----\r\n";

 

    char *client_cert =
        "-----BEGIN CERTIFICATE-----\r\n"
        "MIIEiDCCAnCgAwIBAgIUXJnqMJf1pNH1sPDYldpTXheUZRkwDQYJKoZIhvcNAQEL\r\n"
        "BQAwFzEVMBMGA1UEAxMMUklDLUludGVyLUNBMB4XDTI1MDMxODIwNTIyNloXDTI4\r\n"
        "MDMwMjIwNTI1NlowNzE1MDMGA1UEAxMsNjdkOWRjYTZhZGY1NDdmZDVkNTQ4ZWFi\r\n"
        "Lm9iamVjdHMucmlnaHRlY2guaW8wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\r\n"
        "AoIBAQCo0TyRjLQOGHanUq8naP+oPPxG2kcmPSpvkmauRyLNgSzMQnRorN6nN3WM\r\n"
        "UPyymTUF+QhjVL0J+zloIy3I5adKBbuamPx+PGz2OMPo6+GhI/uk8tHC2/0E/Xj4\r\n"
        "9z32g9BEJ3ZCAd/NSChsltmLEBRdUufkjUJ8QVMsj0m8XL7Xd4RGGLS/s7ThcIJI\r\n"
        "oeS/VK3LqCznR3pXDw0A5MmlgW/yZvrxBjbw4Y/Eg9p+MbRlT/Slt7ibqkt8sGR2\r\n"
        "u7MStXhTdTkbVsDq91f5XooOPmVkaAdRLdvq7RR/HXAUFeLqE1bqtq9SKnkYJSvS\r\n"
        "4E2IP5yzE8VR+IJeEtc+xh8ZKzOhAgMBAAGjgaswgagwDgYDVR0PAQH/BAQDAgOo\r\n"
        "MB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAdBgNVHQ4EFgQU1qOCi2OP\r\n"
        "d42o5Ftw0weI9C0RH4cwHwYDVR0jBBgwFoAUmP7inKw4RUikvtMwJCI9+NxnCaMw\r\n"
        "NwYDVR0RBDAwLoIsNjdkOWRjYTZhZGY1NDdmZDVkNTQ4ZWFiLm9iamVjdHMucmln\r\n"
        "aHRlY2guaW8wDQYJKoZIhvcNAQELBQADggIBACbcHtSYdFHDmmzy2+EH1T3bizUI\r\n"
        "EUg3OIMer07CfUkhIESE6bO32+VGBvOuhsGg+yLqANjvjiYwwagIEqXzaVWFIckd\r\n"
        "HYDYb7Pwf/hZnU43LDLU4FFGRgrZTpzmHmlNE1hwx1rX82Y3G/S/wXVv9ulMMxxu\r\n"
        "M0xuIDXTeQ7hTZHei9V5qfbLV2X8lOdv8rpkSylFOF6mRQKFh3/c76sHaxG2arMb\r\n"
        "k5t79SYLWx6Ss4dol0qua646tWtDf3De4fRVT1McItO1IHo6T6JRTblTyMKlhwWp\r\n"
        "/N1Y4RY1s7dwV6QvmGgezwNpX/ophDIIRithZ9Z8BR0RtjBLxwqeJs13+j0cbtuP\r\n"
        "IqRVXNkMpgmCk0ja1l+x1Cg/GfyEg1H/rutDszAgwJeKghjEMQktSH9XTAXdUcpH\r\n"
        "RnGx6c3v2Dr5nAHC1UNZ4JUVPQS9C8HRWLTExr4UKsomATS6zzQwePJs154Ty4uW\r\n"
        "oxjiKPLCxdI338JGO+LSZiopBqMfagzggzR1Cw9UN32O2coXIvqyg+pc8ZcOsBjy\r\n"
        "XsEeAcKvqxoBRSHaudoTZxsMZnex84DTasHm744KeIsweKiVM38q5vywf6dOik6V\r\n"
        "/leocpHqWccBrZESd8YhOicALrsYSclOv1ZCHs0BDFPoVLb3JNKtGAPfIAgDa80Y\r\n"
        "/s9fUiojSMnqrQpC\r\n"
        "-----END CERTIFICATE-----\r\n";

 

    char *client_key =
        "-----BEGIN RSA PRIVATE KEY-----\r\n"
        "MIIEpgIBAAKCAQEAqNE8kYy0Dhh2p1KvJ2j/qDz8RtpHJj0qb5JmrkcizYEszEJ0\r\n"
        "aKzepzd1jFD8spk1BfkIY1S9Cfs5aCMtyOWnSgW7mpj8fjxs9jjD6OvhoSP7pPLR\r\n"
        "wtv9BP14+Pc99oPQRCd2QgHfzUgobJbZixAUXVLn5I1CfEFTLI9JvFy+13eERhi0\r\n"
        "v7O04XCCSKHkv1Sty6gs50d6Vw8NAOTJpYFv8mb68QY28OGPxIPafjG0ZU/0pbe4\r\n"
        "m6pLfLBkdruzErV4U3U5G1bA6vdX+V6KDj5lZGgHUS3b6u0Ufx1wFBXi6hNW6rav\r\n"
        "Uip5GCUr0uBNiD+csxPFUfiCXhLXPsYfGSszoQIDAQABAoIBAQCR7YSo4HLIeHcg\r\n"
        "vkWwEIBmookEiizUrizfkzL1VYKNGCtsScmsrjotW7Bd4af+jpcaGaIZkydx2FtQ\r\n"
        "XJB4R6RrRTddNP+V84/Q61LWJgi9LYialleiVF2MEbufosFKNbkzINWFy271WXmw\r\n"
        "HFnibrzbyw2vMDiXhjRqoVYA4D8LqtnpOmhjzYQNJ+pYZTngYa6csUUdvTKz0+ns\r\n"
        "Hh0kLtiMYP16w7ZQAOPQmnPKtwLOqN0Lr5in0l0Izl+2mPmiv7rZyfPBdWMD+lPd\r\n"
        "HutrL5n46nd3qZjPLcCRQccFjaGNQNHXQGhMJh+RYIJmuj1TTZqEhEK/wkboEHjp\r\n"
        "TTW8x8TtAoGBAMjBTtkNAvVaa/pbA/DNYDOmvacINuiKJZtJ5t3es/4fMSylJROl\r\n"
        "MBHVPQkTc4gBChZ0uXG7AaAvQDcQoR5LHO1n0eM0Rz1N3KIlSOvc5y+umbveQ4vk\r\n"
        "pZVwO4QkQZhglggD+nUb1T7cn15Ul1j/r8+lVtWOE4BHxKlK4KqAAvzfAoGBANdF\r\n"
        "/hNloCahzSUl8m2M58nAfRpuFhy6sYT7tegda7heVw1QQwxne87qY/e352bF1l65\r\n"
        "cWYS+zB6Zjzpj1gIjIBpZAlzNYHBOOpQO6oj5ZhRtv5Z9i59pd2WwU/gEHyly072\r\n"
        "2TkA0FB/1shHBT+TiK898S2n6v9Iq9oQoHzmR19/AoGBAJ0dxTLUHxucv+M5NjVQ\r\n"
        "1ti1x7ohELAf5lzJktjUAfSBvv+c5A6i+qMKS3F9+q5XeeinQ7eBzzzpng06g45s\r\n"
        "5N/coAR8lsCg3ms8WPzXb0v/DyxcQGsM8JarNrktkvTJqsHtMyhSNyuyiTvPYn/x\r\n"
        "5EvTr8kFH7gG8yA3jOuDslLrAoGBAJB15MGJggZKsArMwzmmw+jMpmc1FtuioPv3\r\n"
        "miOwkpf/nvVNNiE91ISPIBSdMcjy3B9m3GU+OZhXmwInTc0qt3Z4wuvghziSvKno\r\n"
        "u5E6U5l6xI09O4oJQWRJBWKfnxC2hY9w8WZiWGic0TpHBouaarAGpjEYLQew8bn9\r\n"
        "TGVkduLrAoGBALIqCsY2hvi2HPUjunePQCTFBvOtNfiVxmccEBtW1drP+frQSwTC\r\n"
        "qI5ui/xhQQrziHeeFoRiuLlFNUs8gomMJO4dAQjKJLkkd1PLm3QkOhfGxWndUW5O\r\n"
        "YD1oWsgp2EN8Gbs/MLf3ZyUCH2pTCuQwETQK4c6po53I0OnZ9SrKJjmB\r\n"
        "-----END RSA PRIVATE KEY-----\r\n";

char *ca_cert_path = "c:/cacert.pem";
char *client_cert_path = "c:/clientcert.pem";
char *client_key_path = "c:/clientkey.pem";

sTaskRef helloWorldProcesser;
static UINT8 helloWorldProcesserStack[1024*10];

static sMsgQRef urc_mqtt_msgq_1 = NULL;

extern void PrintfResp(char *format);
/**
  * @brief  simcom printf
  * @param  pointer *format
  * @note   The message will be output by sAPI_Debug and captured by CATSTUDIO tool.
  * @retval void
  */
 void simcom_printf(const char *format,...){
  char tmpstr[256];

  va_list ap;     	
  va_start(ap, format);   	
  vsprintf(tmpstr, format, ap);        	
  va_end(ap);  	
  PrintfResp(tmpstr);

}

static void mqtt_write_file(char *file_name, char *data)
{
    SCFILE *file_hdl = NULL;
    char pTemBuffer [300] = {0};
    int actul_write_len = 0, buff_data_len = 0, ret = 0;

    file_hdl = sAPI_fopen(file_name, "wb");
    if(file_hdl == NULL)
    {
        sAPI_Debug("sAPI_FsFileOpen err");
        sprintf(pTemBuffer, "FILE SYSTEM Write file failed because open file failed\r\n");
        goto err;
    }
    buff_data_len = strlen(data);
    actul_write_len = sAPI_fwrite(data, buff_data_len,1,file_hdl);
    sAPI_Debug("sAPI_fwrite (%d,%d)\r\n", buff_data_len, actul_write_len);
    if(actul_write_len != buff_data_len)
    {
        sAPI_Debug("sAPI_fwrite err write length: %d\r\n", actul_write_len);
        sprintf(pTemBuffer, "FILE SYSTEM Write file failed\r\n");
        goto err;
    }
    
    ret = sAPI_fclose(file_hdl);
    if(ret != 0)
    {
        sAPI_Debug("sAPI_fclose err");
        sprintf(pTemBuffer, "FILE SYSTEM Write file failed because close file failed\r\nn");
        goto err;
    }else{
        file_hdl = NULL;
    }
    sprintf(pTemBuffer, "FILE SYSTEM Write file successful\r\n\r\nFilename is %s \r\n",file_name);

err:
    PrintfResp(pTemBuffer);
}

/**
  * @brief  helloworld task processor
  * @param  pointer *argv
  * @note   
  * @retval void
  */
void sTask_HelloWorldProcesser(void* argv)
{
  simcom_printf("\r\nTask runs successfully\r\n");

  SCcpsiParm Scpsi = {0};
  sAPI_NetworkInit();
  while(1)
  {
    sAPI_NetworkGetCpsi(&Scpsi);
    if (strncmp(Scpsi.networkmode, "LTE", sizeof(char) * 3) == 0)
    {
      simcom_printf("NETWORK STATUS IS NORMAL\r\n");
      break;
    }
    else
    {
      simcom_printf("NETWORK STATUS IS NOT OK\r\n");
      sAPI_TaskSleep(200);
    }
  }

  sAPI_FBF_Disable();  //must unlock fbf befor download
  mqtt_write_file(ca_cert_path, ca_cert);
  mqtt_write_file(client_cert_path, client_cert);
  mqtt_write_file(client_key_path, client_key);

  int status;
  status = sAPI_FsSwitchDir(ca_cert_path, DIR_SIMDIR_TO_C);
  simcom_printf("ca_cert_path ret = %d\r\n", status);
  status = sAPI_FsSwitchDir(client_cert_path, DIR_SIMDIR_TO_C);
  simcom_printf("client_cert_path ret = %d\r\n", status);
  status = sAPI_FsSwitchDir(client_key_path, DIR_SIMDIR_TO_C);
  simcom_printf("client_key_path ret = %d\r\n", status);

    if(sAPI_SslSetContextIdMsg("sslversion",0,"4") != 0)
    {
        PrintfResp("wrong sslversion\r\n");
    } 
    if(sAPI_SslSetContextIdMsg("authmode",0,"2") != 0)
    {
        PrintfResp("wrong authmode\r\n");
    }

    if(sAPI_SslSetContextIdMsg("cacert",0,"cacert.pem") != 0)
    {
        PrintfResp("wrong cacert\r\n");
    }

    if(sAPI_SslSetContextIdMsg("clientcert",0,"clientcert.pem") != 0)
    {
        PrintfResp("wrong clientcert\r\n");
    }

    if(sAPI_SslSetContextIdMsg("clientkey",0,"clientkey.pem") != 0)
    {
        PrintfResp("wrong clientkey\r\n");
    }

    if(sAPI_SslSetContextIdMsg("enableSNI",0,"1") != 0)
    {
        PrintfResp("wrong enableSNI\r\n");
    }

    if(sAPI_SslSetContextIdMsg("negotiatetime",0,"300") != 0)
    {
        PrintfResp("wrong negotiatetime\r\n");
    }
    
    SCmqttReturnCode ret = sAPI_MqttStart(-1);
    if(SC_MQTT_RESULT_SUCCESS == ret)
    {
        PrintfResp("MQTT Init Successful!\r\n");
    }
    else
    {
        PrintfResp("MQTT Init Fail!\r\n");
    }
    ret = sAPI_MqttSslCfg(SC_MQTT_OP_SET, NULL, 0, 0);
    if(SC_MQTT_RESULT_SUCCESS == ret)
    {
        PrintfResp("MQTT sAPI_MqttSslCfg Successful!\r\n");
        
    }
    else
    {
        PrintfResp("MQTT sAPI_MqttSslCfg Fail!\r\n");
    }

    ret= sAPI_MqttAccq(0, NULL, 0, "mqtt_test", 1, urc_mqtt_msgq_1);
    if(SC_MQTT_RESULT_SUCCESS == ret)
    {
        sAPI_Debug("accq SUCCESS");
        PrintfResp("MQTT accquire Successful!\r\n");
        
    }
    else
    {
        sAPI_Debug("accq FAIL,ERRCODE = [%d]",ret);
        PrintfResp("MQTT accquire Fail!\r\n");
    }
    ret= sAPI_MqttConnect(0, NULL, 0, "tcp://dev.rightech.io:8883", 60, 1, "mqttRigtehLogin" , "123456");
    if(SC_MQTT_RESULT_SUCCESS == ret)
    {
        sAPI_Debug("connect SUCCESS");
        PrintfResp("MQTT connect Successful!\r\n");
    }
    else
    {
        sAPI_Debug("connect FAIL,ERRCODE = [%d]",ret);
        PrintfResp("MQTT connect Fail!\r\n");
    }

  while(1)
  {
    sAPI_TaskSleep(1000);
  }


}

/**
  * @brief  helloworld task initial
  * @param  void
  * @note   
  * @retval void
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
