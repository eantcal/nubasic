' test_native_posix_missing_lib.bas - dlopen() failure for a missing library
' PLATFORM: posix
' EXPECT_ERROR: Cannot load native library

Syntax Modern

Declare Function ghost_fn Lib "lib_does_not_exist_nubasic_test.so" () As Integer

Print ghost_fn()
