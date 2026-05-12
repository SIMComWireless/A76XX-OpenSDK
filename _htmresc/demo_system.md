# demo_system

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_system.c

## Overview

This demo demonstrates system-level operations on the ASR1603 module, specifically the ability to query task stack usage information. It provides a way to monitor stack memory consumption of running tasks, which is essential for debugging memory issues and optimizing stack allocation in production applications.

## Main Features

- Query stack size, in-use amount, and peak usage for registered tasks
- Display task name alongside stack statistics

## Key Header Dependencies

- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `simcom_os.h` -- Provides OS types (sTaskRef, UINT32) and `sAPI_GetTaskStackInfo`, `sAPI_TaskSleep`
- `simcom_common.h` -- Provides common types and definitions
- `uart_api.h` -- Provides UartReadValue for menu input

## SDK APIs Observed

- `sAPI_GetTaskStackInfo(taskRef, &taskName, &stackSize, &stackInuse, &stackPeak)` -- Retrieves stack usage information for a given task reference. Outputs the task name string, total stack size, currently in-use stack bytes, and peak stack usage.
- `sAPI_TaskSleep(ms)` -- Sleeps the current task for the specified number of milliseconds (used to pace output).

## Usage

The demo is conditionally compiled under `SIMCOM_GET_TASKSTACK_INFO`. If the macro is not defined, the demo prints a message indicating the feature is not available and returns.

When enabled, the demo presents a menu:

1. **GetTaskStackInfo** (Option 1): Iterates through a predefined list of task references (`helloWorldProcesser` and `simcomUIProcesser`), calls `sAPI_GetTaskStackInfo` for each, and displays the results in the format: `taskName_stack(S:size U:inuse P:peak)`.
2. **Back** (Option 99): Return to parent menu.

## Implementation Notes

- The demo requires the `SIMCOM_GET_TASKSTACK_INFO` compile-time macro to be defined. Without it, the feature is completely excluded from the build.
- The task reference list is defined as a local array of `sTaskRef` containing `helloWorldProcesser` and `simcomUIProcesser`. These are declared as `extern` from other source files.
- The `gUrcProcessTask` reference is commented out in the task list.
- The demo uses a `for` loop to iterate through the `taskRefList` array, calling `sAPI_GetTaskStackInfo` for each task and formatting the output with `snprintf`.
- A `sAPI_TaskSleep(200*2)` call (400ms delay) between each task query prevents flooding the output.

## Best Practices

- Use `sAPI_GetTaskStackInfo` during development and testing to determine optimal stack sizes for each task. Allocate stack based on peak usage plus a safety margin (typically 20-30% extra).
- Monitor stack usage in production if the `SIMCOM_GET_TASK_STACK_INFO` feature is enabled. Peak usage approaching the allocated size indicates potential stack overflow risk.
- Keep the task reference list up to date. Add all application-created tasks to the monitoring list for comprehensive stack profiling.
- Call stack info queries periodically rather than in tight loops to avoid performance overhead.
- Use the peak usage metric (`stackPeak`) rather than the current in-use metric (`stackInuse`) for stack size decisions, as peak represents the worst-case scenario.
