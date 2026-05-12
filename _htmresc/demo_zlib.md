# demo_zlib

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_zlib.c

## Overview

This demo demonstrates ZIP file compression and decompression on the ASR1603 module using the zlib and MiniZip libraries. It provides functionality to extract files from ZIP archives and create new ZIP archives from directory contents, operating on the module's file system.

## Main Features

- Extract (unzip) a ZIP archive to a specified directory
- Create (zip) a new ZIP archive from all files in a directory
- Support for deflate compression with configurable compression level
- Handles both files and subdirectories within ZIP archives

## Key Header Dependencies

- `zlib.h` -- Provides zlib compression constants and types: Z_DEFLATED, Z_BEST_COMPRESSION, Z_OK, z_stream, compress, uncompress, deflate, inflate
- `zip.h` -- Provides MiniZip creation APIs: zipOpen, zipOpenNewFileInZip, zipWriteInFileInZip, zipCloseFileInZip, zipClose, zipFile, ZIP_OK, APPEND_STATUS_CREATE
- `unzip.h` -- Provides MiniZip extraction APIs: unzOpen, unzClose, unzFile, do_extract (extern), do_open (extern), do_close (extern), UNZ_OK
- `simcom_file.h` -- Provides sAPI_fopen, sAPI_fread, sAPI_fwrite, sAPI_fclose, sAPI_fsize, sAPI_opendir, sAPI_readdir, sAPI_closedir for file system access
- `simcom_common.h` -- Provides common types
- `simcom_debug.h` -- Provides sAPI_Debug
- `uart_api.h` -- Provides UartReadValue, UartReadLine

## SDK APIs Observed

- `zipOpen(pathname, append)` -- Create/open a ZIP file. `append` mode: 0 = create new, 1 = append after, 2 = add to existing. Returns a `zipFile` handle.
- `zipOpenNewFileInZip(zf, filename, zipfi, extrafield, size, extrafield_global, size, comment, method, level)` -- Open a new file entry inside the ZIP. Method: Z_DEFLATED for deflate compression. Level: Z_BEST_COMPRESSION (9) for maximum compression.
- `zipWriteInFileInZip(zf, buf, len)` -- Write data to the current file inside the ZIP.
- `zipCloseFileInZip(zf)` -- Close the current file entry in the ZIP.
- `zipClose(zf, comment)` -- Close the ZIP file and write the central directory.
- `do_open(path)` -- Open a ZIP file for extraction (extern function, likely implemented in a separate minizip utility file).
- `do_extract(uf, opt_extract_without_path, opt_overwrite, password)` -- Extract all files from the opened ZIP archive.
- `do_close(uf)` -- Close the ZIP file after extraction.
- `sAPI_fopen`, `sAPI_fread`, `sAPI_fclose`, `sAPI_fsize` -- File I/O for reading source files to compress.
- `sAPI_opendir`, `sAPI_readdir`, `sAPI_closedir` -- Directory traversal for zip operation.

## Usage

The demo presents an interactive menu:

1. **unzip** (Option 1): Enter the path to a ZIP file. The demo opens the archive with `do_open`, extracts all files with `do_extract(uf, 0, 0, NULL)` (preserving paths, not overwriting, no password), then closes with `do_close`.
2. **zip** (Option 2): Enter the desired ZIP filename, the append mode (0-2), and the source directory path. The demo opens the directory, iterates through all entries, and for each file opens it via `sAPI_fopen`, reads in 2048-byte chunks, writes to the ZIP via `zipWriteInFileInZip`, and closes. Subdirectories are added as empty directory entries.
3. **back** (Option 99): Return to parent menu.

## Implementation Notes

- The `zipwritefile` helper function handles the file-to-ZIP transfer: opens the source file, reads in 2048-byte chunks, writes each chunk to the ZIP, then closes both the file entry and source file.
- Directory entries in the ZIP are identified by `info_dir->type == 2` and have a trailing slash appended via `scAddSpritInPathTail`.
- File entries in the ZIP are identified by `info_dir->type == 1`.
- The `scStrcatHead` helper prepends the directory path to the file name, so the ZIP entry includes the relative path from the source directory.
- The filename passed to `zipOpenNewFileInZip` uses `&info_dir->name[3]` or `&filename[3]` to strip the drive letter prefix (e.g., "C:/") from the path, making the ZIP entry path relative.
- The extern functions `do_extract`, `do_open`, and `do_close` are declared but implemented in a separate source file (likely a minizip utility).
- The unzip operation uses `password = NULL`, meaning encrypted ZIPs are not supported.

## Best Practices

- Check the return value of `zipOpen` for NULL before proceeding with ZIP operations.
- Always close file entries (`zipCloseFileInZip`) before opening new ones or closing the ZIP.
- Always close the ZIP file (`zipClose`) to ensure the central directory is written correctly.
- Use appropriate buffer sizes (2048 bytes as shown) for file read operations to balance memory usage and I/O efficiency.
- Handle errors at each step: file open failures, read errors, and ZIP write errors should be caught and handled gracefully.
- For unzip operations, verify the ZIP file is valid by checking the return of `do_open` before calling `do_extract`.
- Be cautious with the `append` mode parameter: mode 2 (APPEND_STATUS_ADDINZIP) requires that the file already exists and is a valid ZIP.
