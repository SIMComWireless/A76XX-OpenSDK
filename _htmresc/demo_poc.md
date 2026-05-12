# demo_poc

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_poc.c

## Overview
`demo_poc` demonstrates the POC (Push-to-Talk Over Cellular) low-level audio API on the A7672 module. Unlike the standard audio demo which uses higher-level file/stream playback, the POC API provides direct PCM-level audio control suitable for real-time voice communication. The demo shows how to play raw PCM data from a WAV file, stop playback, start/stop PCM recording, and read recorded PCM frames.

## Main Features
- Play raw PCM audio data from a WAV file using chunked reads via `sAPI_PocPlaySound`.
- Stop PCM playback via `sAPI_PocStopSound`.
- Start PCM recording with a specified mode via `sAPI_PocStartRecord`.
- Read recorded PCM data frames via `sAPI_PocPcmRead`.
- Stop PCM recording via `sAPI_PocStopRecord`.

## Key Header Dependencies
- `simcom_poc.h` — POC API prototypes, state enums (`POC_PCM_IDLE`, `POC_PCM_PLAYING`, `POC_PCM_RECORDING`), and callback typedefs (`sAPI_ProcessRecordCb`, `sAPI_NotifyBufStateCb` or custom variants for `CUS_GWSD`/`CUS_CORGET`/`CUS_BND`/`CUS_CY` builds).
- `simcom_os.h` — OS primitives: `sAPI_TaskSleep`.
- `simcom_file.h` — File I/O APIs: `sAPI_fopen`, `sAPI_fread`, `sAPI_fclose`.
- `uart_api.h` — Interactive UART input: `UartReadValue`, `PrintfResp`, `PrintfOptionMenu`.

## SDK APIs Observed
- `sAPI_PocPlaySound(data, length)` — Play a chunk of raw PCM data. `data` is a pointer to the PCM buffer, `length` is the number of bytes to play (demo uses 320 bytes per call). Returns an int status code.
- `sAPI_PocStopSound()` — Stop current PCM playback. Returns an int status code.
- `sAPI_PocStartRecord(callback, mode)` — Start PCM recording. `callback` can be NULL (the demo uses NULL and reads data manually with `sAPI_PocPcmRead`). `mode` specifies the recording mode (demo uses mode 2). Returns an int status code.
- `sAPI_PocPcmRead(buffer, dataLen)` — Read recorded PCM data into `buffer`. `dataLen` is the number of bytes to read (demo reads 320 bytes per call). Returns the number of bytes actually read.
- `sAPI_PocStopRecord()` — Stop PCM recording. Returns an int status code.

## Usage
1. Run `POCDemo()` to display the interactive UART menu.
2. Select option 1 (POC Play PCM):
   - The demo opens `c:/test.wav` in binary read mode.
   - It reads and discards the first 44 bytes (WAV file header).
   - It then reads the file in 320-byte chunks and plays each chunk via `sAPI_PocPlaySound`.
   - Every 10 chunks, it sleeps for 20 ms (`sAPI_TaskSleep(20)`) to pace playback and avoid buffer overrun.
   - After the entire file is read, it calls `sAPI_PocStopSound` to finalize playback.
3. Select option 2 (POC Stop PCM) to immediately stop any ongoing POC playback.
4. Select option 3 (POC Start record):
   - Calls `sAPI_PocStartRecord(NULL, 2)` to start recording with mode 2 and no callback.
   - Enters a loop of 200 iterations, sleeping 5 ms between reads.
   - Each iteration reads 320 bytes of recorded PCM data via `sAPI_PocPcmRead` into a 640-byte buffer.
5. Select option 4 (POC Stop record) to stop the recording session.
6. Select option 99 to return to the parent menu.

## Implementation Notes
- The POC API operates at the PCM level, bypassing the higher-level audio subsystem. This gives finer control over audio timing and buffer management, which is critical for real-time voice communication.
- Playback reads 320 bytes per chunk, which at 16-bit 8 kHz mono corresponds to 20 ms of audio (320 bytes / 2 bytes-per-sample / 8000 samples-per-second = 0.02 seconds). The `sAPI_TaskSleep(20)` every 10 chunks provides flow control.
- The WAV header is skipped by reading the first 44 bytes and discarding them before entering the playback loop.
- Recording uses a polling approach: the demo starts recording, then repeatedly calls `sAPI_PocPcmRead` in a tight loop with 5 ms sleep intervals. An alternative is to provide a callback to `sAPI_PocStartRecord` for interrupt-driven recording.
- The recording buffer is 640 bytes per read, but the demo requests 320 bytes per `sAPI_PocPcmRead` call.
- The `simcom_poc.h` header has conditional compilation for different customer builds (`CUS_GWSD`, `CUS_CORGET`, `CUS_BND`, `CUS_CY`), which change the callback function signatures.
- Additional POC-related APIs exist in the header but are not demonstrated: `sAPI_PocInitLib`, `sAPI_PocGetPcmAvail`, `sAPI_PocCleanBufferData`, `sAPI_TonePlay`, `sAPI_ToneStop`, `sAPI_FreqPlay`, `sAPI_SetToneVolume`, `sAPI_GetToneVolume`.
- The demo file header is a copy of the audio demo header (inaccurate), but the actual content is POC-specific.

## Best Practices
- Call `sAPI_PocInitLib` at system startup (not shown in this demo) to initialize the POC library before using play/record functions.
- Use the callback-based recording mode when possible (pass a valid callback to `sAPI_PocStartRecord`) instead of polling `sAPI_PocPcmRead`, as it is more efficient.
- Match the playback chunk size and sleep interval to the audio sample rate and bit depth. For 16 kHz 16-bit audio, 640 bytes = 20 ms; for 8 kHz 16-bit, 320 bytes = 20 ms.
- Always call `sAPI_PocStopSound` after playback and `sAPI_PocStopRecord` after recording to release POC resources.
- For production, check return values from all POC API calls and implement error recovery.
- Use `sAPI_PocCleanBufferData` to reset the POC buffer state when switching between playback and recording modes.
- Avoid overlapping POC playback and recording unless the platform explicitly supports full-duplex POC.
