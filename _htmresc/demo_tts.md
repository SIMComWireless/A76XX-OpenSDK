# demo_tts

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_tts.c

## Overview
`demo_tts` demonstrates Text-to-Speech (TTS) functionality on the A7672 module. It provides an interactive UART menu that lets users play text as spoken audio, stop ongoing TTS playback, and configure TTS parameters such as volume, digit mode, pitch, and speed. The TTS feature requires the `FEATURE_SIMCOM_TTS_IFLY` or `FEATURE_SIMCOM_TTS_YOUNGTONE` compile-time flag to be enabled.

## Main Features
- Play TTS audio from user-provided text in either UNICODE or ASCII encoding, with local or remote playback path selection via `sAPI_TTSPlay`.
- Stop ongoing TTS playback via `sAPI_TTSStop`.
- Configure TTS parameters (system volume, TTS volume, digit mode, pitch, speed) via `sAPI_TTSSetParameters`.

## Key Header Dependencies
- `simcom_tts_api.h` — Declares `sAPI_TTSSetParameters` and conditionally includes `simcom_tts.h`. Guarded by `FEATURE_SIMCOM_TTS_IFLY` or `FEATURE_SIMCOM_TTS_YOUNGTONE`.
- `simcom_tts.h` — (included transitively) Provides `sAPI_TTSPlay` and `sAPI_TTSStop` prototypes.
- `simcom_common.h` — Common SDK types (`UINT32`, `BOOL`).
- `simcom_debug.h` — Debug logging via `sAPI_Debug`.
- `uart_api.h` — Interactive UART input: `UartReadValue`, `UartReadLine`, `PrintfResp`, `PrintfOptionMenu`.

## SDK APIs Observed
- `sAPI_TTSPlay(option, text, mode)` — Play TTS audio. `option`: 1 = UNICODE, 2 = ASCII encoding. `text`: the string to speak. `mode`: 0 = local playback, 1 = remote playback. Returns `TRUE` on success, `FALSE` on failure.
- `sAPI_TTSStop()` — Stop current TTS playback. Returns `TRUE` on success, `FALSE` on failure.
- `sAPI_TTSSetParameters(volume, tts_volume, digit_mode, pitch, speed)` — Configure TTS parameters. `volume`: system volume level. `tts_volume`: TTS-specific volume. `digit_mode`: digit reading mode. `pitch`: voice pitch. `speed`: speech speed. Returns `TRUE` on success, `FALSE` on failure.

## Usage
1. Run `TTSDemo()` to display the interactive UART menu.
2. Select option 1 (Play) to initiate TTS playback:
   - Choose encoding: 1 for UNICODE, 2 for ASCII.
   - Enter the text string to be spoken (up to 511 characters).
   - Choose playback mode: 0 for local speaker, 1 for remote (e.g., through a voice call).
3. Select option 2 (Stop) to immediately halt any ongoing TTS playback.
4. Select option 3 (SetPara) to configure TTS parameters:
   - Enter system volume level.
   - Enter TTS volume level.
   - Enter digit mode.
   - Enter pitch value.
   - Enter speed value.
5. Select option 99 to return to the parent menu.

## Implementation Notes
- The TTS text input buffer is 512 bytes. Text exceeding this limit will be truncated.
- The `sAPI_TTSPlay` function is blocking: it returns only after playback completes or is stopped.
- The demo uses `sAPI_Debug` for logging rather than `printf`, making it compatible with the SDK debug infrastructure.
- Feature flags `FEATURE_SIMCOM_TTS_IFLY` and `FEATURE_SIMCOM_TTS_YOUNGTONE` control which TTS engine is compiled in (iFlytek or Youngtone). At least one must be defined for TTS functionality to be available.
- The `simcom_tts_api.h` header is located in `sc_lib/inc/` (a shared include directory), not in the chip-specific include path, because TTS is a cross-platform feature.

## Best Practices
- Always call `sAPI_TTSStop()` before starting a new TTS playback to avoid conflicts with ongoing speech.
- Choose the correct encoding option matching the text content: use UNICODE (option 1) for multi-byte character sets (e.g., Chinese), and ASCII (option 2) for standard Latin text.
- For production applications, store TTS parameters persistently and apply them at boot rather than relying on interactive input.
- Use remote playback mode (mode=1) only when a voice call is active; otherwise, use local mode (mode=0).
- Handle the return value of `sAPI_TTSPlay` to detect and report playback failures, especially when the audio subsystem is busy with another operation.
