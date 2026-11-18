# Code signing policy

nuBASIC is an open-source project maintained in the
[`eantcal/nubasic`](https://github.com/eantcal/nubasic) repository.

Free code signing provided by [SignPath.io](https://signpath.io/), certificate
by [SignPath Foundation](https://signpath.org/).

## Signed artifacts

When SignPath signing is enabled, signing requests will cover only nuBASIC
artifacts produced from this repository by the project's GitHub Actions
workflows. Each release signing request must be approved manually.

The project may distribute unsigned binaries from upstream open-source
projects as dependencies. These third-party binaries are not signed as
nuBASIC artifacts.

## Team roles

- **Committer and reviewer:** [Antonino Calderone (`@eantcal`)](https://github.com/eantcal)
- **Signing approver:** [Antonino Calderone (`@eantcal`)](https://github.com/eantcal)

Changes submitted by contributors who do not have direct commit access must
be reviewed before they are merged. Changes to build scripts, GitHub Actions
workflows, packaging, and signing configuration receive the same review as
source-code changes.

## Privacy policy

nuBASIC does not collect telemetry or automatically transmit personal data.

This program will not transfer any information to other networked systems
unless specifically requested by the user or the person installing or
operating it.

Programs executed through nuBASIC can access files, launch commands, or call
native libraries when explicitly instructed by their source code and allowed
by the operating system. Users should review untrusted BASIC programs before
running them; native library calls can be disabled with
`--disable-native-calls`.

## Reporting issues

Security-sensitive issues should be reported privately through
[GitHub's security advisory form](https://github.com/eantcal/nubasic/security/advisories/new).
Other defects can be reported through the
[public issue tracker](https://github.com/eantcal/nubasic/issues).
