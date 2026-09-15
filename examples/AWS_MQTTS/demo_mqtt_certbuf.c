/**
 * @file    demo_mqtt_certbuf.c
 * @brief   MQTT over TLS mutual-authentication demo -- certificates supplied as
 *          in-memory buffers (approach 2).
 */
/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#include "simcom_os.h"
#include "simcom_debug.h"
#include "simcom_common.h"
#include "simcom_network.h"
#include "simcom_file.h"          /* File system APIs: sAPI_fopen/sAPI_fseek/sAPI_fread/... */
#include "simcom_ssl.h"           /* sAPI_SslSetContextIdMsg / SSL configuration macros      */
#include "simcom_mqtts_client.h"  /* sAPI_MqttXXX                                            */

/* --------------------------- User configuration ----------------------------*/
#define MQTT_CLIENT_INDEX       0                 /* MQTT client index, range 0~1 */
#define MQTT_SSL_CTX_ID         0                 /* SSL context index, range 0~9 */

/* AWS IoT endpoint and ClientId; change them to match your own project. */
#define MQTT_BROKER_URL         "tcp://a1ejuhsp1v2i1w-ats.iot.ap-south-1.amazonaws.com:8883"
#define MQTT_CLIENT_ID          "xxxxxx"  /* must match the CN of the client certificate */

/** Paths of the certificates inside the module file system (lfs); change them
 *  to match where the files are actually stored.
 *  @note These files are read through sAPI_fopen, i.e. they must live on a
 *        mounted file system drive such as C:/ or D:/. */
#define CERT_PATH_CA            "C:/AmazonRootCA1.pem"
#define CERT_PATH_CLIENT        "C:/certificatePem.pem"
#define CERT_PATH_KEY           "C:/privateKeyPem.pem"
/* SSL context parameters */
#define MQTT_SSL_VERSION        "4"   /* 0:SSL3.0 1:TLS1.0 2:TLS1.1 3:TLS1.2 4:ALL          */
#define MQTT_AUTH_MODE          "2"   /* 2: mutual auth (verify server + present client cert) */
#define MQTT_ENABLE_SNI         "1"   /* AWS IoT requires SNI                                 */

/** 0: validate the certificate validity period.
 *  If the module clock is not synchronised and the handshake fails with a
 *  "certificate not yet valid" error, temporarily set this to 1 to verify the
 *  rest of the chain. */
#define MQTT_IGNORE_LOCALTIME   1

#define MQTT_KEEPALIVE_S        60
#define MQTT_CLEAN_SESSION      1

/** 1: publish one test message after a successful connection
 *  (the broker policy must allow it). */
#define DEMO_MQTT_PUB_TEST      0
/** Keep the connection for N seconds and then tear it down on purpose so the
 *  demo can be run repeatedly; 0 = keep the connection and never exit. */
#define DEMO_TEARDOWN_AFTER_S   0

/* --------------------------- Internal types and variables ------------------*/
/**
 * @brief Descriptor of one certificate held in RAM.
 */
typedef struct
{
    char *data;   /* Certificate content (PEM text), guaranteed NUL-terminated */
    int   len;    /* Number of bytes actually read                            */
} cert_buf_t;

static cert_buf_t g_ca   = {NULL, 0};
static cert_buf_t g_cert = {NULL, 0};
static cert_buf_t g_key  = {NULL, 0};

static sMsgQRef g_mqtt_urc_msgq   = NULL;
static sTaskRef g_mqtt_demo_task  = NULL;
static UINT8    g_mqtt_demo_stack[SC_DEFAULT_THREAD_STACKSIZE * 2] = {0xA5};

/* --------------------------- Helper functions ------------------------------*/
/**
 * @brief  Convert an MQTT result code into a human readable string.
 * @param  rc  Result code returned by the sAPI_MqttXXX family.
 * @return Pointer to a constant string describing @p rc.
 */
static const char *mqtt_err_str(SCmqttReturnCode rc)
{
    switch (rc)
    {
        case SC_MQTT_RESULT_SUCCESS:           return "success";
        case SC_MQTT_RESULT_SOCK_CREATE_FAIL:  return "socket create fail";
        case SC_MQTT_RESULT_SOCK_CONN_FAIL:    return "socket connect fail";
        case SC_MQTT_RESULT_DNS_ERROR:         return "dns error";
        case SC_MQTT_RESULT_SSL_HANDSHAKE_ERR: return "ssl handshake fail(check certs/time/SNI)";
        case SC_MQTT_RESULT_NOT_SET_CERTS:     return "certs not set(check cabuf/certbuf/keybuf)";
        case SC_MQTT_RESULT_BAD_USRNAME_PWD:   return "bad username/password";
        case SC_MQTT_RESULT_NOT_AUTHORIZED:    return "not authorized(check cert policy/ClientId)";
        default:                               return "see SCmqttResultType";
    }
}

/**
 * @brief  Release the RAM buffer of a certificate.
 * @param  buf  Certificate descriptor to free; safe to call more than once.
 * @note   The pointer is set to NULL and @c len to 0 on return.
 */
static void cert_buf_free(cert_buf_t *buf)
{
    if (buf->data != NULL)
    {
        sAPI_Free(buf->data);
        buf->data = NULL;
    }
    buf->len = 0;
}

/**
 * @brief  Read one certificate file from the file system into a RAM buffer
 *         (the core step of this demo).
 * @param  path  Full path of the certificate file, for example "C:/ca.pem".
 * @param  out   Output buffer; on success the caller owns the data and must
 *               release it with cert_buf_free().
 * @retval  0  Success.
 * @retval -1  Failure.
 * @note   cabuf/certbuf/keybuf derive their length from strlen(), so the
 *         content must be NUL-terminated PEM text. For binary DER
 *         certificates use the file-name approach instead:
 *         sAPI_SslSetContextIdMsg("cacert", id, "xxx.der").
 */
static int cert_read_file(const char *path, cert_buf_t *out)
{
    SCFILE *fp = NULL;
    long size = 0;
    int rd = 0;

    out->data = NULL;
    out->len = 0;

    fp = sAPI_fopen(path, "rb");
    if (fp == NULL)
    {
        sAPI_Debug("[certbuf] fopen fail: %s", path);
        return -1;
    }

    /* File size: seek to the end, use ftell, then rewind to the beginning. */
    if ((sAPI_fseek(fp, 0, FS_SEEK_END) != 0) || ((size = sAPI_ftell(fp)) <= 0))
    {
        sAPI_Debug("[certbuf] seek/tell fail: %s", path);
        sAPI_fclose(fp);
        return -1;
    }
    sAPI_fseek(fp, 0, FS_SEEK_BEGIN);

    /* 32KB upper bound: large enough for a single PEM certificate or private
     * key, yet below the INT16 length limit of the API. */
    if (size > 32 * 1024)
    {
        sAPI_Debug("[certbuf] file too large(%ld): %s", size, path);
        sAPI_fclose(fp);
        return -1;
    }

    out->data = (char *)sAPI_Malloc(size + 1);
    if (out->data == NULL)
    {
        sAPI_Debug("[certbuf] malloc %ld fail", size);
        sAPI_fclose(fp);
        return -1;
    }
    memset(out->data, 0, size + 1);

    rd = sAPI_fread(out->data, 1, size, fp);
    sAPI_fclose(fp);

    if (rd <= 0)
    {
        sAPI_Debug("[certbuf] fread fail(%d): %s", rd, path);
        cert_buf_free(out);
        return -1;
    }

    out->data[rd] = '\0';   /* NUL-terminate so that strlen() can be used */
    out->len = rd;
    sAPI_Debug("[certbuf] read ok: %s (%d bytes)", path, rd);
    return 0;
}

/**
 * @brief  Configure the SSL context and inject the certificates as buffers.
 * @retval  0  Success.
 * @retval -1  Failure.
 * @note   Consumes the global buffers g_ca / g_cert / g_key; the context keeps
 *         its own copy, so the caller may free them right after this call.
 */
static int ssl_ctx_setup_with_buffers(void)
{
    int ret;
    char val[8];

    ret = sAPI_SslSetContextIdMsg("sslversion", MQTT_SSL_CTX_ID, MQTT_SSL_VERSION);
    if (ret != 0)
    {
        sAPI_Debug("[certbuf] set sslversion fail, ret=%d", ret);
        return -1;
    }

    /* AWS IoT requires SNI. */
    ret = sAPI_SslSetContextIdMsg("enableSNI", MQTT_SSL_CTX_ID, MQTT_ENABLE_SNI);
    if (ret != 0)
    {
        sAPI_Debug("[certbuf] set enableSNI fail, ret=%d", ret);
        return -1;
    }

    ret = sAPI_SslSetContextIdMsg("authmode", MQTT_SSL_CTX_ID, MQTT_AUTH_MODE);
    if (ret != 0)
    {
        sAPI_Debug("[certbuf] set authmode fail, ret=%d", ret);
        return -1;
    }

    snprintf(val, sizeof(val), "%d", MQTT_IGNORE_LOCALTIME);
    ret = sAPI_SslSetContextIdMsg("ignorelocaltime", MQTT_SSL_CTX_ID, val);
    if (ret != 0)
    {
        sAPI_Debug("[certbuf] set ignorelocaltime fail, ret=%d", ret);
        return -1;
    }

    /* ===== Certificate buffer injection ===== */
    ret = sAPI_SslSetContextIdMsg("cabuf", MQTT_SSL_CTX_ID, g_ca.data);   /* CA certificate */
    if (ret != 0)
    {
        sAPI_Debug("[certbuf] set cabuf fail, ret=%d", ret);
        return -1;
    }
    sAPI_Debug("[certbuf] cabuf   set ok, len=%d", g_ca.len);

    ret = sAPI_SslSetContextIdMsg("certbuf", MQTT_SSL_CTX_ID, g_cert.data); /* Client certificate */
    if (ret != 0)
    {
        sAPI_Debug("[certbuf] set certbuf fail, ret=%d", ret);
        return -1;
    }
    sAPI_Debug("[certbuf] certbuf set ok, len=%d", g_cert.len);

    ret = sAPI_SslSetContextIdMsg("keybuf", MQTT_SSL_CTX_ID, g_key.data);   /* Client private key */
    if (ret != 0)
    {
        sAPI_Debug("[certbuf] set keybuf fail, ret=%d", ret);
        return -1;
    }
    sAPI_Debug("[certbuf] keybuf  set ok, len=%d", g_key.len);

    return 0;
}

/* --------------------------- Demo main task --------------------------------*/
/**
 * @brief  Demo task: wait for network registration, read the certificates into
 *         RAM, set up the SSL context and connect to the broker over TLS.
 * @param  arg  Unused.
 * @note   Runs as a dedicated task created by MqttCertBufDemoInit().
 */
static void mqtt_certbuf_demo_task(void *arg)
{
    SCmqttReturnCode rc;
    int pGreg = 0;
    int mqtt_started = 0;
    int client_accqed = 0;

    (void)arg;

    /* [1] Wait for network registration (precondition). */
    while (1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        if (1 == pGreg || 5 == pGreg)
        {
            sAPI_Debug("[certbuf] network ready, cgreg=%d", pGreg);
            break;
        }
        sAPI_Debug("[certbuf] waiting network, cgreg=%d", pGreg);
        sAPI_TaskSleep(10 * 200);   /* 10s */
    }

    rc = sAPI_MqttStart(-1);
    if (rc != SC_MQTT_RESULT_SUCCESS)
    {
        sAPI_Debug("[certbuf] mqtt start fail, rc=%d(%s)", rc, mqtt_err_str(rc));
        return;
    }
    mqtt_started = 1;

    /* [3] File system -> buffer: read the three certificates (core step of this demo). */
    if ((cert_read_file(CERT_PATH_CA, &g_ca) != 0) ||
        (cert_read_file(CERT_PATH_CLIENT, &g_cert) != 0) ||
        (cert_read_file(CERT_PATH_KEY, &g_key) != 0))
    {
        sAPI_Debug("[certbuf] read cert file fail, check CERT_PATH_xxx macros");
        goto cleanup;
    }

    /* [4] Configure the SSL context and inject the certificate buffers. */
    if (ssl_ctx_setup_with_buffers() != 0)
    {
        goto cleanup;
    }

    /* The context has copied the certificate content, so the local buffers can
     * be released immediately. */
    cert_buf_free(&g_ca);
    cert_buf_free(&g_cert);
    cert_buf_free(&g_key);

    rc = sAPI_MqttAccq(SC_MQTT_OP_SET, NULL, MQTT_CLIENT_INDEX, MQTT_CLIENT_ID, 1, g_mqtt_urc_msgq);
    if (rc != SC_MQTT_RESULT_SUCCESS)
    {
        sAPI_Debug("[certbuf] mqtt accq fail, rc=%d(%s)", rc, mqtt_err_str(rc));
        goto cleanup;
    }
    client_accqed = 1;

    rc = sAPI_MqttSslCfg(SC_MQTT_OP_SET, NULL, MQTT_CLIENT_INDEX, MQTT_SSL_CTX_ID);
    if (rc != SC_MQTT_RESULT_SUCCESS)
    {
        sAPI_Debug("[certbuf] mqtt sslcfg fail, rc=%d(%s)", rc, mqtt_err_str(rc));
        goto cleanup;
    }

    rc = sAPI_MqttConnect(SC_MQTT_OP_SET, NULL, MQTT_CLIENT_INDEX,
                          MQTT_BROKER_URL, MQTT_KEEPALIVE_S, MQTT_CLEAN_SESSION,
                          NULL, NULL);
    if (rc != SC_MQTT_RESULT_SUCCESS)
    {
        sAPI_Debug("[certbuf] mqtt connect fail, rc=%d(%s)", rc, mqtt_err_str(rc));
        if (rc == SC_MQTT_RESULT_SSL_HANDSHAKE_ERR)
        {
            sAPI_Debug("[certbuf] hint: cert content/time/SNI, or try filename mode");
        }
        goto cleanup;
    }

    sAPI_Debug("[certbuf] ===== MQTT(TLS two-way auth, cert-buffer) CONNECT OK =====");

#if DEMO_MQTT_PUB_TEST
    /* Connection check: publish one message to a test topic (the broker policy
     * must allow that topic). */
    {
        char *topic   = "certbuf/demo/hello";
        char *payload = "{\"from\":\"module\",\"via\":\"cert-buffer\"}";

        rc = sAPI_MqttTopic(MQTT_CLIENT_INDEX, topic, strlen(topic));
        sAPI_Debug("[certbuf] set topic rc=%d", rc);

        rc = sAPI_MqttPayload(MQTT_CLIENT_INDEX, payload, strlen(payload));
        sAPI_Debug("[certbuf] set payload rc=%d", rc);

        rc = sAPI_MqttPub(MQTT_CLIENT_INDEX, 1, 60, 0, 0);
        sAPI_Debug("[certbuf] publish rc=%d", rc);
    }
#endif

#if (DEMO_TEARDOWN_AFTER_S > 0)
    /* Keep the connection for a while so it can be observed, then tear it down
     * following the AT command sequence: */
    sAPI_TaskSleep(DEMO_TEARDOWN_AFTER_S * 200);

    rc = sAPI_MqttDisConnect(SC_MQTT_OP_SET, NULL, MQTT_CLIENT_INDEX, 60);
    sAPI_Debug("[certbuf] disconnect rc=%d(%s)", rc, mqtt_err_str(rc));

    rc = sAPI_MqttRel(MQTT_CLIENT_INDEX);
    sAPI_Debug("[certbuf] release rc=%d(%s)", rc, mqtt_err_str(rc));
    client_accqed = 0;

    rc = sAPI_MqttStop();
    sAPI_Debug("[certbuf] stop rc=%d(%s)", rc, mqtt_err_str(rc));
    mqtt_started = 0;
#else
    sAPI_Debug("[certbuf] keep connection, demo task exit");
#endif
    return;

cleanup:
    cert_buf_free(&g_ca);
    cert_buf_free(&g_cert);
    cert_buf_free(&g_key);
    if (client_accqed)
    {
        sAPI_MqttRel(MQTT_CLIENT_INDEX);
    }
    if (mqtt_started)
    {
        sAPI_MqttStop();
    }
    sAPI_Debug("[certbuf] demo cleanup done");
}

/* --------------------------- Public entry point ----------------------------*/
/**
 * @brief  Initialise the demo: create the URC message queue and the demo task.
 * @note   Call it once from the application entry or the demo menu.
 *         Retrieve SCmqttData from g_mqtt_urc_msgq and sAPI_Free() its three
 *         pointers.
 */
void MqttCertBufDemoInit(void)
{
    SC_STATUS status;

    if (g_mqtt_urc_msgq == NULL)
    {
        status = sAPI_MsgQCreate(&g_mqtt_urc_msgq, "mqtt_urc_q", sizeof(SIM_MSG_T), 4, SC_FIFO);
        if (status != SC_SUCCESS)
        {
            sAPI_Debug("[certbuf] msgq create fail");
            return;
        }
    }

    if (sAPI_TaskCreate(&g_mqtt_demo_task, g_mqtt_demo_stack, sizeof(g_mqtt_demo_stack),
                        SC_DEFAULT_TASK_PRIORITY, (char *)"mqttCertBufDemo",
                        mqtt_certbuf_demo_task, NULL) != SC_SUCCESS)
    {
        sAPI_Debug("[certbuf] demo task create fail");
        return;
    }

    sAPI_Debug("[certbuf] demo task create success");
}
