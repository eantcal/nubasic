# Security policy

## Supported versions

Security fixes are made on the `main` branch and shipped in the next nuBASIC
release. Only the latest release is supported.

## Reporting a vulnerability

Please do not report security-sensitive issues in public issues.

Report them privately through
[GitHub's security advisory form](https://github.com/eantcal/nubasic/security/advisories/new),
including the affected version, the steps to reproduce, and the impact you
observed.

Other defects can be reported through the
[public issue tracker](https://github.com/eantcal/nubasic/issues).

## Running untrusted programs

BASIC programs run with the permissions of the user who starts them and can
access files, launch commands, or call native libraries. Review untrusted
programs before running them, and use `--disable-native-calls` to block
native library calls.
