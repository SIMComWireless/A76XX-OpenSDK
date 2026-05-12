# demo_call

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_call.c

## Overview
`demo_call` demonstrates voice call operations on the A7672 module using the SIMCom OpenSDK call API. It provides an interactive UART menu for dialing, answering, ending calls, querying call state, and configuring auto-answer. The demo uses an asynchronous message queue pattern where call operations return results via a `sMsgQRef` queue. A conditional CSD (Circuit-Switched Data) section is also present.

## Main Features
- Dial a voice call to a user-specified phone number via `sAPI_CallDialMsg`.
- Answer an incoming call via `sAPI_CallAnswerMsg`.
- End (hang up) the current call via `sAPI_CallEndMsg`.
- Query current call state via `sAPI_CallStateMsg`.
- Configure automatic answer with a configurable delay (0-255 seconds) via `sAPI_CallAutoAnswer`.
- CSD (Circuit-Switched Data) receive data handling (compiled under `CSD_SUPPORT`).

## Key Header Dependencies
- `simcom_call.h` — Call API prototypes (`sAPI_CallDialMsg`, `sAPI_CallAnswerMsg`, `sAPI_CallEndMsg`, `sAPI_CallStateMsg`, `sAPI_CallAutoAnswer`), return code enum `SC_CALLReturnCode` (`SC_CALL_SUCESS`, `SC_CALL_FAIL`), command type enum, and the `SC_CALL_MAX_NUMBER_LENGTH` constant (40).
- `simcom_common.h` — Common types and the `SIM_MSG_T` message structure.
- `simcom_os.h` — OS primitives: `sAPI_MsgQCreate`, `sAPI_MsgQRecv`, `sAPI_MsgQDelete`, `sAPI_TaskCreate`, `sAPI_apMsgQInit`.
- `simcom_debug.h` — Debug logging via `sAPI_Debug`.
- `simcom_uart.h` — UART definitions.
- `uart_api.h` — Interactive UART input: `UartReadValue`, `UartReadLine`, `PrintfResp`, `PrintfOptionMenu`.

## SDK APIs Observed
- `sAPI_CallDialMsg(dialstring, msgQ)` — Initiate a voice call. `dialstring` is the phone number (up to `SC_CALL_MAX_NUMBER_LENGTH` = 40 chars). `msgQ` receives the result asynchronously. Equivalent to ATD command. Returns `SC_CALL_SUCESS` or `SC_CALL_FAIL`.
- `sAPI_CallAnswerMsg(msgQ)` — Answer an incoming call. Equivalent to ATA command. Returns `SC_CALL_SUCESS` or `SC_CALL_FAIL`.
- `sAPI_CallEndMsg(msgQ)` — End the current call. Equivalent to AT+CHUP. Returns `SC_CALL_SUCESS` or `SC_CALL_FAIL`.
- `sAPI_CallStateMsg()` — Get current call state synchronously. Returns a `UINT8` value: 0 = Active, 1 = Held, 2 = Dialing, 3 = Alerting, 4 = Incoming, 5 = Waiting, 6 = Offering, 7 = Disconnecting, 8 = End.
- `sAPI_CallAutoAnswer(seconds, msgQ)` — Configure auto-answer. `seconds` = 0 disables auto-answer; 1-255 sets the delay in seconds before the module auto-answers an incoming call. Returns `SC_CALL_SUCESS` or `SC_CALL_FAIL`.
- `sAPI_MsgQCreate(&msgQ, name, msgSize, msgCount, order)` — Create a message queue for asynchronous result delivery.
- `sAPI_MsgQRecv(msgQ, &msg, timeout)` — Receive a message from the queue with a timeout (`CALL_URC_RECIVE_TIME_OUT` = 20000 ms).
- `sAPI_MsgQDelete(msgQ)` — Delete the message queue on demo exit.

## Usage
1. Run `CALLDemo()` to create the message queue and display the interactive UART menu.
2. Select option 1 (Dial): enter a phone number at the prompt. The demo sends the dial request and waits for a result message (up to 20 seconds). The result code from `msg.arg2` is displayed.
3. Select option 2 (Answer): answers the current incoming call and displays the result code.
4. Select option 3 (End): hangs up the active call and displays the result code.
5. Select option 4 (State): queries and displays the current call state as a numeric value.
6. Select option 5 (Auto Answer): enter a delay value (0-255 seconds). Setting 0 disables auto-answer; any other value enables it with the specified delay.
7. Select option 99 to exit the demo, which deletes the message queue.

## Implementation Notes
- All call operations (dial, answer, end, auto-answer) follow an asynchronous pattern: the API returns `SC_CALL_SUCESS` if the request was accepted, and the actual result arrives later as a `SIM_MSG_T` on the message queue. The demo uses `sAPI_MsgQRecv` with a 20-second timeout to wait for results.
- The result code from call operations is stored in `msg.arg2` of the `SIM_MSG_T` structure.
- The message queue `g_call_demo_msgQ` is created once when `CALLDemo()` is entered and deleted when the user selects option 99 (back).
- The `callStateMsg()` function is synchronous (no message queue needed); it directly returns the call state value.
- For certain module variants (`SIMCOM_A7680C_V5_01`, `SIMCOM_A7670C_V701`, etc.), the UART is redefined to `SC_UART4`.
- The `CALL_URC_RECIVE_TIME_OUT` is set to 20000 ms (20 seconds) to accommodate call setup time and network response delays.
- A conditional `CSD_SUPPORT` section demonstrates receiving CSD data via a dedicated task and message queue, using `sAPI_apMsgQInit` for application-level message queue initialization.

## Best Practices
- Always create the message queue before calling any call API that requires it; passing an uninitialized `msgQ` will cause failures.
- Handle the case where `sAPI_MsgQRecv` returns `SC_FAIL` (timeout) to avoid hanging indefinitely when the network does not respond.
- For auto-answer, choose a delay value that gives the user enough time to manually answer if desired; values that are too high risk the caller hanging up first.
- In production, register URC (Unsolicited Result Code) handlers for incoming call notifications rather than relying solely on polling `sAPI_CallStateMsg`.
- Delete the message queue when no longer needed to free OS resources.
- Validate phone number input length against `SC_CALL_MAX_NUMBER_LENGTH` before calling `sAPI_CallDialMsg`.
