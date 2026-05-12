# demo_cjson

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_cjson.c

## Overview

This demo demonstrates JSON parsing and creation using the cJSON library on the ASR1603 module. It shows how to configure cJSON's memory hooks to use the module's heap allocator, parse a JSON string to extract values, and programmatically create a JSON object.

## Main Features

- Parse a JSON string and extract string values by key name
- Create a JSON object programmatically with string key-value pairs
- Configure cJSON memory allocation hooks to use module heap functions

## Key Header Dependencies

- `cJSON.h` -- Provides the cJSON library: cJSON_Parse, cJSON_GetObjectItem, cJSON_Delete, cJSON_CreateObject, cJSON_AddItemToObject, cJSON_CreateString, cJSON_Print, cJSON_InitHooks, cJSON_Hooks
- `simcom_os.h` -- Provides sAPI_Malloc, sAPI_Free (used indirectly via malloc/free hooks)
- `simcom_debug.h` -- Provides sAPI_Debug for debug output

## SDK APIs Observed

- `cJSON_InitHooks(&hooks)` -- Initialize cJSON with custom memory allocation hooks. The `hooks` structure provides `malloc_fn` and `free_fn` function pointers.
- `cJSON_Parse(data)` -- Parse a JSON string into a cJSON tree. Returns a `cJSON*` root object, or NULL on parse failure.
- `cJSON_GetObjectItem(root, key)` -- Get a child item from a cJSON object by key name. Returns NULL if not found.
- `cJSON_Delete(root)` -- Free a cJSON tree and all its children.
- `cJSON_CreateObject()` -- Create a new empty JSON object.
- `cJSON_AddItemToObject(root, key, item)` -- Add a cJSON item to an object with the given key.
- `cJSON_CreateString(value)` -- Create a cJSON string item.
- `cJSON_Print(root)` -- Render a cJSON tree to a formatted JSON string. The returned string must be freed by the caller.

## Usage

The demo runs automatically (no interactive menu). The `CjsonDemo()` function calls two sub-tests:

1. **CjsonParseTest**: Parses the JSON string `{"company":"simcom","module":"A7600E"}`, extracts the "company" and "module" values, prints them via `sAPI_Debug`, then frees the tree.
2. **CjsonCreateTest**: Creates a cJSON object, adds "company":"simcom" and "module":"A7600E" string items, prints the formatted JSON via `cJSON_Print` and `sAPI_Debug`, then frees the tree.

## Implementation Notes

- **Critical**: `cJSON_InitHooks` MUST be called before any other cJSON function. The hooks must point to `malloc` and `free` (or `sAPI_Malloc` and `sAPI_Free`). If this step is skipped, the module may crash or behave unpredictably because cJSON would attempt to use a default allocator that may not be available on the embedded platform.
- The demo uses standard `malloc`/`free` for the hooks rather than `sAPI_Malloc`/`sAPI_Free`. Both work because `sAPI_Malloc`/`sAPI_Free` are wrappers around the module's heap allocator which is compatible with standard malloc/free on this platform.
- `cJSON_GetObjectItem` returns a pointer to the item within the tree (not a copy). The item's `valuestring` field contains the string value for `cJSON_String` type items.
- `cJSON_Print` allocates a new string that must be freed by the caller. The demo does not free this string, which is a minor memory leak in the test code.
- The `cJSON_Hooks` struct uses `UINT32` for the malloc size parameter (from `simcom_os.h`), matching the module's type definitions.

## Best Practices

- Always call `cJSON_InitHooks` with appropriate memory hooks before using any cJSON function on the SIMCom platform.
- Check the return value of `cJSON_Parse` for NULL before accessing the parsed tree.
- Always call `cJSON_Delete` to free the entire cJSON tree when done. Do not free individual items separately.
- Check the return value of `cJSON_GetObjectItem` for NULL before accessing `valuestring` or other fields.
- Use `cJSON_AddStringToObject`, `cJSON_AddNumberToObject` convenience macros for simpler code.
- Free strings returned by `cJSON_Print` after use to prevent memory leaks.
- For production, consider using `sAPI_Malloc`/`sAPI_Free` directly in the hooks to ensure consistent memory tracking.
- Avoid parsing very large JSON documents due to limited heap memory on embedded systems.
