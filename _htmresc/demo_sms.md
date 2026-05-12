# demo_sms

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_sms.c

## Overview
`demo_sms` provides a comprehensive interactive UART menu for SMS operations on the A7672 module. It supports both PDU and TEXT mode SMS, covering the full lifecycle: initialization, message writing to SIM, reading, sending (direct or from storage), long/multi-part message sending, deletion, new message indication configuration, character set management, PDU encode/decode, and format setting. The demo uses an asynchronous message queue pattern and a dedicated URC (Unsolicited Result Code) processing task.

## Main Features
- Initialize/deinitialize SMS subsystem resources (message queues and URC processor task).
- Write SMS messages to SIM card in PDU or TEXT mode via `sAPI_SmsWriteMsg`.
- Read stored SMS messages by index via `sAPI_SmsReadMsg`.
- Send SMS messages directly in PDU or TEXT mode via `sAPI_SmsSendMsg`.
- Send stored (previously written) SMS messages by index via `sAPI_SmsSendStorageMsg`.
- Send long/multi-part SMS messages (up to 820 characters) via `sAPI_SmsSendLongMsg`.
- Delete all SMS messages from SIM via `sAPI_SmsDelAllMsg`.
- Delete a single SMS message by index via `sAPI_SmsDelOneMsg`.
- Configure new SMS indication mode (store to SIM, direct text report, or direct PDU report) via `sAPI_SmsSetNewMsgInd`.
- Get current new message indication settings via `sAPI_SmsGetNewMsgInd`.
- List stored SMS messages via `sAPI_SmsListMsg`.
- Get/set character set (GSM, IRA, UCS2) via `sAPI_SmsgetChset` / `sAPI_SmssetChset`.
- Decode PDU to text via `sAPI_SmsDecodePdu`.
- Encode text to PDU via `sAPI_SmsEncodePdu`.
- Get/set SMS format (PDU or TEXT mode) via `sAPI_SmsgetFormat` / `sAPI_SmsSetFormat`.
- URC processor task handles `SC_URC_NEW_MSG_IND` (new message stored with index) and `SC_URC_FLASH_MSG` (direct message content) events.
- Built-in simple test sequence that exercises write, send, and stored-send operations end-to-end.

## Key Header Dependencies
- `simcom_sms.h` — SMS API prototypes, enums (`SC_SMSReturnCode`, `SC_SMS_CMD_TYPE`, `SC_SMS_Stroge`), constants (`SC_SMS_MAX_ADDRESS_LENGTH` = 40, `SC_SMS_MAX_CI_MSG_PDU_SIZE` = 400, `SC_SMS_MAX_MULTIPLE_MSG_LENGTH` = 820), type definitions (`SCsmsFormatMode`, `SCsmsPayload`, `SCsmsPduReadCnf`), and message primitive IDs.
- `simcom_common.h` — Common types and the `SIM_MSG_T` message structure.
- `simcom_os.h` — OS primitives: `sAPI_MsgQCreate`, `sAPI_MsgQRecv`, `sAPI_MsgQDelete`, `sAPI_TaskCreate`, `sAPI_Free`.
- `simcom_debug.h` — Debug logging via `sAPI_Debug`.
- `simcom_uart.h` — UART definitions and `sAPI_UartWriteString`.
- `uart_api.h` — Interactive UART input: `UartReadValue`, `UartReadLine`, `UartRead`, `PrintfResp`, `PrintfOptionMenu`.

## SDK APIs Observed
- `sAPI_SmsWriteMsg(fmatmode, sms, smslen, srcAddr, stat, msgQ)` — Write SMS to SIM. `fmatmode`: 0 = PDU, 1 = TEXT. `stat`: 1 = unsent, 2 = sent. In PDU mode, `srcAddr` is NULL.
- `sAPI_SmsSendMsg(fmatmode, sms, smslen, addr, msgQ)` — Send SMS directly. In PDU mode, the address is embedded in the PDU data, so `addr` can be NULL.
- `sAPI_SmsSendLongMsg(sms, smslen, addr, msgQ)` — Send long/multi-part SMS in text mode. Max content length: `SC_SMS_MAX_MULTIPLE_MSG_LENGTH` (820 bytes).
- `sAPI_SmsSendStorageMsg(msgIndex, addr, msgQ)` — Send a previously stored SMS by its SIM index.
- `sAPI_SmsReadMsg(fmatmode, msgIndex, msgQ)` — Read a stored SMS. Response in `msg.arg3` contains the message content; must be freed with `sAPI_Free`.
- `sAPI_SmsDelAllMsg(msgQ)` — Delete all SMS messages from SIM.
- `sAPI_SmsDelOneMsg(msgIndex, msgQ)` — Delete a single SMS by index.
- `sAPI_SmsSetNewMsgInd(fmatmode, mode, mt, bm, ds, bfr)` — Set new message indication mode. Valid combinations: `(1,2,1,0,0,0)` to store to SIM + report index via `SC_URC_NEW_MSG_IND`; `(1,1,2,0,0,0)` for direct text report via `SC_URC_FLASH_MSG`; `(0,1,2,0,0,0)` for direct PDU report.
- `sAPI_SmsGetNewMsgInd(&mode, &mt, &bm, &ds, &bfr)` — Get current new message indication parameters.
- `sAPI_SmsListMsg(mode)` — List SMS messages with a specified mode.
- `sAPI_SmsgetChset(&format)` / `sAPI_SmssetChset(format)` — Get/set character set: 0 = GSM, 2 = IRA, 4 = UCS2.
- `sAPI_SmsgetFormat(&format)` / `sAPI_SmsSetFormat(format)` — Get/set SMS format: 0 = PDU mode, 1 = TEXT mode.
- `sAPI_SmsDecodePdu(pduText, outAddr, outTextLen, outText)` — Decode PDU string to address and text content.
- `sAPI_SmsEncodePdu(addrStr, contentText, contentLen, pduText, &pduLen)` — Encode address and text content to PDU format.

## Usage
1. Run `SMSDemo()` to display the interactive UART menu (16+ options).
2. Select option 1 (Init) first to create message queues and start the URC processor task. All other operations require initialization.
3. Option 2 (Write): enter format mode (0=PDU, 1=TEXT), message length, message data, source address, and storage status (1=unsent, 2=sent).
4. Option 3 (Read): enter format mode and SIM index to read a stored message.
5. Option 4 (Send): enter format mode, length, data, and destination address to send directly.
6. Option 5 (Send stored): enter SIM index and optional destination address to send a previously written message.
7. Option 6 (Send long msg): enter format mode, length, data (up to 820 bytes), and destination address for multi-part SMS.
8. Option 7 (Delete all) and option 8 (Delete one) remove messages from SIM.
9. Option 9 (Simple test) runs a built-in sequence: delete all, send text SMS, send PDU SMS, write text/PDU to SIM, and send stored PDU SMS.
10. Option 10/11 set/get the new message indication mode.
11. Option 13 sets/gets the character set (GSM/IRA/UCS2).
12. Option 14/15 decode/encode PDU data.
13. Option 16 gets/sets SMS format (PDU/TEXT).
14. Option 99 (DeInit) cleans up resources and exits.

## Implementation Notes
- The SMS demo requires explicit initialization (option 1) before any other operation. The `gSmsDemohasinited` flag prevents double-init and enforces this ordering.
- Three message queues are created: `g_sms_demo_msgQ` for API responses, `g_sms_demo_urc_process_msgQ` for incoming URC events, and `g_sms_demo_urc_rsp_msgQ` for URC-triggered read responses.
- A dedicated task (`sTask_SmsUrcProcessor`) runs at priority 90 on a 4 KB stack, blocking on `g_sms_demo_urc_process_msgQ` with `SC_SUSPEND` (infinite wait).
- The URC processor handles two event types: `SC_URC_NEW_MSG_IND` (triggers an automatic read of the new message at the reported index) and `SC_URC_FLASH_MSG` (directly displays the incoming text content).
- Memory allocated by the SDK for response strings (`msg.arg3`) must be explicitly freed with `sAPI_Free` after use to avoid leaks.
- The `inputDataProcessor` function handles multi-packet UART input, buffering data until a `\r\n` terminator is received. This is necessary because UART transports data in 32-byte chunks.
- The `SC_SMS_MAX_MULTIPLE_MSG_LENGTH` (820) buffer is used for long messages, while `SC_SMS_MAX_CI_MSG_PDU_SIZE` (400) is used for standard PDU messages.
- For certain module variants, the UART is redefined to `SC_UART4`.

## Best Practices
- Always initialize SMS resources (option 1) before performing any SMS operation; attempting to use SMS APIs without initialization will fail.
- Always free the response string pointer (`msg.arg3`) with `sAPI_Free` after reading SMS responses to prevent memory leaks.
- Set new message indication mode at system boot using `sAPI_SmsSetNewMsgInd` to control how incoming messages are delivered.
- Use PDU mode (format=0) for international/multilingual SMS where precise encoding control is needed; use TEXT mode (format=1) for simpler ASCII text.
- For long messages, use `sAPI_SmsSendLongMsg` which handles multi-part segmentation automatically; do not manually split messages.
- Validate the SIM index before read/delete operations to avoid `SC_SMS_INVALID_INDEX` errors.
- In production, replace the URC processor's UART output with application-level event dispatching.
- Deinitialize (option 99) SMS resources when the SMS feature is no longer needed to free OS resources (message queues and task).
