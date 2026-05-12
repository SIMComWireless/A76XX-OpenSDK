# demo_audio

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_audio.c

## Overview
`demo_audio` provides a comprehensive interactive UART menu for exercising all audio playback and recording capabilities of the A7672 module via the SIMCom OpenSDK. It covers file-based playback (WAV, MP3, AMR), PCM/MP3/AMR stream playback, audio recording to file, PCM stream recording, volume/mic-gain control, PA (power amplifier) GPIO configuration, and echo suppression parameter tuning.

## Main Features
- Set audio sample rate (8 kHz or 16 kHz) via `sAPI_AudioPlaySampleRate`.
- Play audio files (WAV, MP3, AMR) with options for direct playback, single/repeat mode, and local/remote playback path via `sAPI_AudioPlay`.
- Stop file playback via `sAPI_AudioStop`.
- Record audio to AMR files with timed duration and callback notification via `sAPI_AudioRecord` and `sAPI_AudRec`.
- Play raw PCM data from a WAV file (skipping 44-byte header) via `sAPI_AudioPcmPlay`; stop with `sAPI_AudioPcmStop`.
- Stream-play MP3 data from a buffer loaded via `sAPI_AudioMp3StreamPlay`; stop with `sAPI_AudioMp3StreamStop`.
- Continuous MP3 playback with frame control via `sAPI_AudioPlayMp3Cont`.
- Stream-play AMR data from a buffer via `sAPI_AudioAmrStreamPlay`; stop with `sAPI_AudioAmrStreamStop`.
- Continuous AMR playback via `sAPI_AudioPlayAmrCont`.
- AMR stream recording with frame-level callback via `sAPI_AmrStreamRecord`.
- Set AMR encoder rate level (0-7) via `sAPI_AudioSetAmrRateLevel`.
- WAV high-sampling-rate file playback via `sAPI_AudioWavFilePlay`.
- Continuous WAV playback via `sAPI_AudioPlayWavCont`.
- Set and get audio volume (0-11) via `sAPI_AudioSetVolume` / `sAPI_AudioGetVolume`.
- Set and get microphone gain (0-7, or 0-23 when POC feature is enabled) via `sAPI_AudioSetMicGain` / `sAPI_AudioGetMicGain`.
- PA (power amplifier) control GPIO configuration via `sAPI_AudioSetPaCtrlConfig`.
- PCM stream recording with file write callback via `sAPI_PCMStreamRec` / `sAPI_PCMStopStreamRec`.
- Echo suppression parameter configuration (compiled out by default, guarded by `#if 0` blocks) via `sAPI_EchoSuppression_PARA` and `sAPI_EchoParaModeSet`.

## Key Header Dependencies
- `simcom_audio.h` — All audio API prototypes, enums (`AUD_SampleRate`, `AUD_Volume`, `AUD_RecordSrcPath`, `AudioFileType_t`), callback typedefs (`AudRecCallback`, `sAPI_GetAmrFrameCB`, `sAPI_GetPCMFrameCB`), and the `SC_Dspconfiguration` struct.
- `sc_dspgain.h` — Echo suppression DSP configuration (conditionally included, currently guarded out).
- `simcom_os.h` — OS primitives: `sAPI_Malloc`, `sAPI_TaskSleep`.
- `simcom_file.h` — File I/O APIs: `sAPI_fopen`, `sAPI_fread`, `sAPI_fclose`.
- `simcom_gpio.h` — GPIO return codes used by PA config.
- `uart_api.h` — Interactive UART input: `UartReadValue`, `UartReadLine`, `PrintfResp`, `PrintfOptionMenu`.

## SDK APIs Observed
- `sAPI_AudioPlaySampleRate(rate)` — Set playback sample rate (0 = 8 kHz, 1 = 16 kHz).
- `sAPI_AudioPlay(file, direct, isSingle)` — Play an audio file; `direct` starts immediately, `isSingle` for single-file mode.
- `sAPI_AudioStop()` — Stop current file playback.
- `sAPI_AudioRecord(enable, path, file)` — Start (enable=1) or stop (enable=0) recording to file.
- `sAPI_AudRec(oper, file, duration, callback)` — Timed recording with completion callback; AMR 1-180s, WAV/PCM 1-15s.
- `sAPI_AudioPcmPlay(buffer, len, direct)` — Play raw PCM data from buffer (WAV header must be stripped).
- `sAPI_AudioPcmStop()` — Stop PCM playback.
- `sAPI_AudioMp3StreamPlay(buffer, len, direct)` — Stream-play MP3 data from a memory buffer.
- `sAPI_AudioMp3StreamStop()` — Stop MP3 stream playback.
- `sAPI_AudioPlayMp3Cont(file, startplay, frame)` — Load MP3 file for continuous playback; `startplay=1` begins playback; `frame` controls frame mode (0-2).
- `sAPI_AudioAmrStreamPlay(buffer, len, direct)` — Stream-play AMR data from a memory buffer.
- `sAPI_AudioAmrStreamStop()` — Stop AMR stream playback.
- `sAPI_AudioPlayAmrCont(file, startplay)` — Continuous AMR file playback.
- `sAPI_AmrStreamRecord(callback)` — Start AMR stream recording with per-frame callback.
- `sAPI_AmrStopStreamRecord()` — Stop AMR stream recording.
- `sAPI_AudioSetAmrRateLevel(level)` — Set AMR encoder rate (0-7).
- `sAPI_AudioWavFilePlay(file, direct)` — Play WAV file with high sampling rate support.
- `sAPI_AudioPlayWavCont(file, startplay)` — Continuous WAV file playback.
- `sAPI_AudioSetVolume(volume)` / `sAPI_AudioGetVolume()` — Set/get volume level (0-11).
- `sAPI_AudioSetMicGain(micgain)` / `sAPI_AudioGetMicGain()` — Set/get microphone gain.
- `sAPI_AudioSetPlayPath(path)` — Set playback path: 0 = local, 1 = remote.
- `sAPI_AudioStatus()` — Get current audio status (0 = idle, 1 = busy).
- `sAPI_AudioSetPaCtrlConfig(gpioNum, activeLevel)` — Configure PA control GPIO pin and active level.
- `sAPI_PCMStreamRec(callback)` / `sAPI_PCMStopStreamRec()` — Start/stop PCM stream recording with data callback.
- `sAPI_EchoSuppression_PARA(dspconfig)` — Apply echo suppression DSP parameters (conditional).
- `sAPI_EchoParaModeSet(SaveMode, Level)` — Set echo suppression mode (conditional).

## Usage
1. Run `AudioDemo()` to display the interactive UART menu with 28+ options.
2. Select option 1 to set the audio sample rate (8 kHz or 16 kHz).
3. Select option 8 to set the volume level (0-11), or option 9 to read the current volume.
4. Select option 12 to set microphone gain, or option 13 to read it.
5. For file playback: select option 2 (play file), enter the filename (e.g., `c:/test.wav`), choose direct playback mode and single/repeat mode, then select the playback path (local or remote). Stop with option 3.
6. For PCM playback: select option 6, enter the WAV filename, the system reads the file into a buffer, skips the 44-byte WAV header, and plays the raw PCM. Stop with option 7.
7. For streaming playback (MP3/AMR buffer): select option 14 or 17, enter the filename, and the file is loaded into a 100 KB buffer for streaming.
8. For recording: select option 4, enter a filename, recording starts. Stop with option 5. Or use option 10 (`sAPI_AudRec`) for timed recording with a callback.
9. Option 16 configures the PA GPIO pin number and active level for external audio amplifiers.
10. Options 27/28 start/stop PCM stream recording where raw PCM data is received via a callback and can be written to a file.

## Implementation Notes
- The demo allocates a 100 KB buffer (`sAPI_Malloc(100 * 1024)`) at startup for file I/O operations across multiple playback modes.
- WAV file playback via PCM mode skips the first 44 bytes (standard WAV header) to extract raw PCM samples.
- Continuous playback modes (MP3/AMR/WAV cont) use a `do...while` loop to allow loading multiple file segments before triggering playback with `startplay=1`.
- The `recordCallBac_Test` callback is a simple stub that prints the status value; production code should implement meaningful post-recording logic.
- The `GetPCMRec` callback receives PCM frames and can optionally write them to a file via `sAPI_fwrite` (commented out in the demo).
- Echo suppression features are conditionally compiled with `#if 0` guards and require the `sc_dspgain.h` header.
- AMR buffer/continuous playback options are conditionally compiled unless `CUS_CY` is defined.
- After each operation, the demo queries `sAPI_AudioStatus()` to verify the audio subsystem state.
- The `sAPI_AudioSetPlayPath` call before playback controls whether audio routes to the local speaker or to a remote party (e.g., during a call).

## Best Practices
- Always check the return value of audio APIs and verify `sAPI_AudioStatus()` after operations to ensure the audio subsystem is in the expected state.
- Use `sAPI_AudioStop()` or the appropriate stop function before starting a new playback to avoid conflicts.
- For large files, prefer streaming APIs (`sAPI_AudioMp3StreamPlay`, `sAPI_AudioAmrStreamPlay`) over loading the entire file into memory.
- When recording, validate the file path and ensure sufficient storage space before starting.
- In production, avoid hardcoding GPIO pin numbers for PA control; retrieve them from a configuration store.
- Set the sample rate before playing audio to match the content's encoding rate.
- For voice call scenarios, use `sAPI_AudioSetPlayPath(1)` (remote path) so audio routes through the call channel.
