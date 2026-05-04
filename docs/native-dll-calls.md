# Native Library Calls

nuBASIC can declare and call exported functions from native shared libraries:

| Platform     | Loader                  | Library suffix  |
|--------------|-------------------------|-----------------|
| Windows x64  | `LoadLibraryW`          | `.dll`          |
| Linux x86_64 | `dlopen` (`RTLD_LAZY`)  | `.so` / `.so.N` |
| macOS        | `dlopen` (`RTLD_LAZY`)  | `.dylib`        |

The feature is unsafe and is enabled by default on trusted local hosts.

Disable it explicitly from the CLI when running untrusted code:

```text
nubasic -t --disable-native-calls -e script.bas
```

With `--disable-native-calls`, any native declaration executed by the program
fails with:

```text
Native DLL calls are disabled by --disable-native-calls.
```

## Syntax

```basic
Declare Function <basic-name> Lib "<library-name>" _
    [Alias "<export-name>"] _
    [CallConv "default" | "cdecl" | "stdcall"] _
    (<param-name> As <native-type>, ...) As <native-type>
```

The `_` marker is nuBASIC line continuation: it joins the following physical
line into the same logical statement before parsing.

If `Alias` is omitted, the BASIC function name is used as the exported symbol.
The calling convention is parsed and stored, but on every supported platform
native calls currently use the default ABI (x64 SysV / Windows x64).

On POSIX, the `Lib` value is passed verbatim to `dlopen`. Use the platform's
canonical sonames, e.g. `libc.so.6` on Linux (glibc), `libSystem.B.dylib` on
macOS, or an absolute path.

## Supported Types

```text
Integer  -> signed 32-bit integer
DWORD    -> unsigned 32-bit integer
Long64   -> signed 64-bit integer
ULong64  -> unsigned 64-bit integer
Double   -> double
Bool     -> 32-bit BOOL-compatible integer
Pointer  -> integer-sized pointer value
String   -> const char* narrow string
Void     -> no return value
```

`String` maps to a narrow `const char*`, so use ANSI exports such as `lstrlenA`
instead of wide-character APIs such as `lstrlenW`.

`Pointer` is an integer-sized native address. nuBASIC also exposes a small set
of native memory helpers so scripts can pass writable buffers or simple
manually laid-out structures to native calls:

```text
NativeAlloc(size) -> Pointer
NativeFree(pointer)
NativeFill(pointer, offset, size, byte)
NativePokeB(pointer, offset, value)
NativePokeI16(pointer, offset, value)
NativePokeI32(pointer, offset, value)
NativePokeI64(pointer, offset, value)
NativePokePtr(pointer, offset, pointerValue)
NativePokeStr(pointer, offset, text, capacity)
NativePeekStr$(pointer, offset, capacity) -> String
```

These helpers are intentionally low level. They do not know the target ABI or
structure layout; the BASIC program must use the correct offsets for the target
platform.

## Examples

### Windows

```basic
Syntax Modern

Declare Function GetCurrentProcessId Lib "kernel32.dll" () As DWORD

Print GetCurrentProcessId()
```

```basic
Syntax Modern
Using runtime

Declare Function lstrlenA Lib "kernel32.dll" _
    Alias "lstrlenA" _
    (text As String) As Integer

Print lstrlenA("nuBASIC")
```

Expected output: `7`.

### Linux / macOS

```basic
Syntax Modern

' Linux (glibc)
Declare Function strlen Lib "libc.so.6" (text As String) As Integer

Print strlen("hello")
```

```basic
Syntax Modern

' macOS
Declare Function strlen Lib "libSystem.B.dylib" (text As String) As Integer

Print strlen("hello")
```

Expected output: `5`.

Pointer argument and mutable buffer:

```basic
Syntax Modern
Using runtime

Declare Function lstrlenA Lib "kernel32.dll" _
    Alias "lstrlenA" _
    (text As Pointer) As Integer

p = NativeAlloc(32)
NativePokeStr p, 0, "nuBASIC", 32
Print lstrlenA(p)
Print NativePeekStr$(p, 0, 32)
NativeFree p
```

For runnable examples, see `examples/native/native_dll_function.bas` for a
small DLL function call and `examples/native/native_open_file_dialog.bas` for a
complete Win32 common dialog.

## Build prerequisites

Native library invocation requires libffi at build time.

| Platform | Install |
|----------|---------|
| Windows  | `vcpkg install libffi:x64-windows` (CMake auto-detects vcpkg) |
| Debian/Ubuntu | `apt install libffi-dev` |
| Fedora/RHEL   | `dnf install libffi-devel` |
| macOS    | `brew install libffi` |

When libffi is not found, `Declare Function ... Lib "..."` is still parsed,
but invocation fails with a clear runtime error.

## Current Limitations

The current implementation does not yet support:

- `ByRef` / output buffers (use `NativeAlloc` + `NativePoke*` + `NativePeekStr$`
  in the meantime)
- wide strings / `LPCWSTR` / `wchar_t*`
- automatic struct marshalling, arrays, callbacks, COM, or variadic
  functions
- automatic header parsing
- sandboxing or library allowlists

These items are tracked in [`nubasic_native_calls_plan.md`](../nubasic_native_calls_plan.md).

Native calls can crash the process or corrupt memory if the declaration does not
exactly match the exported function signature.
