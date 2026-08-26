> **Disclaimer:** This is an AI-assisted project developed using OpenAI Codex.

<h1 align="center">Headless LinkDev</h1>

<p align="center">
  <strong>Headless Remote Play device pairing for PlayStation 5</strong><br>
  Generate the PIN and Account ID entirely through an elfldr terminal.
</p>

<p align="center">
  <a href="https://github.com/blackbearreloaded/headless-linkdev/actions/workflows/build.yml?query=branch%3Amaster"><img src="https://github.com/blackbearreloaded/headless-linkdev/actions/workflows/build.yml/badge.svg?branch=master" alt="Build status"></a>
  <a href="https://github.com/blackbearreloaded/headless-linkdev/releases/latest"><img src="https://img.shields.io/github/v/release/blackbearreloaded/headless-linkdev?display_name=tag&amp;sort=semver&amp;label=latest%20release" alt="Latest release"></a>
  <a href="https://github.com/blackbearreloaded/headless-linkdev/releases"><img src="https://img.shields.io/github/downloads/blackbearreloaded/headless-linkdev/total?label=downloads" alt="GitHub downloads"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-GPL--3.0--or--later-blue" alt="License"></a>
  <img src="https://img.shields.io/badge/firmware-12.70%20tested-003791" alt="Firmware 12.70 tested">
  <img src="https://img.shields.io/badge/platform-PS5-003791?logo=playstation&amp;logoColor=white" alt="PlayStation 5">
</p>

A small PS5 payload that initializes the system Remote Play service, generates
a real eight-digit pairing PIN, reads the foreground user's Base64 Account ID,
and returns everything through the same `elfldr` connection used to launch it.

Headless LinkDev is a focused fork of
[LinkDev](https://github.com/ps5-payload-dev/linkdev). It preserves the original
project history while replacing the graphical BigApp with one direct,
terminal-only payload.

No web page, TV output, SDL application, hbldr stage, notification, or separate
logging connection is required for pairing.

> [!NOTE]
> Firmware **12.70** has been paired successfully with **chiaki-ng**. Other
> firmware versions are currently unverified by this project.

## Example session

```text
$ make send PS5_HOST=192.168.1.100
[RemotePlayPair] Initializing Remote Play...
[RemotePlayPair] READY | PIN: 12345678 | Account ID: AbCdEfGhIjk= | Timeout: 300s
[RemotePlayPair] Pairing completed successfully
```

## What is included

- One PS5 payload source file: [`main.c`](main.c).
- One build artifact: `headless-linkdev.elf`.
- A host self-test for Account ID encoding and registry slot calculations.
- A GitHub Actions workflow for checks, builds, artifacts, and tagged releases.
- No vendored SDK, Sony binaries, UI framework, or runtime dependency bundle.

## Status

| Capability | Status | Notes |
| --- | --- | --- |
| PS5 firmware 12.70 | Verified | Hardware pairing completed successfully |
| PIN generation | Verified | Uses `sceRemoteplayGeneratePinCode` |
| Account ID output | Verified | Base64 value accepted by chiaki-ng |
| elfldr stdout | Verified | Upload and payload output share port `9021` |
| Pairing completion | Verified | Success is detected before the payload exits |
| Other firmware | Unverified | Hardware reports are welcome |

## Requirements

- A jailbroken PS5 with `elfldr` listening on port `9021`.
- The target user logged in and active in the foreground.
- [`socat`](http://www.dest-unreach.org/socat/) on the sending computer.
- [PS5 Payload SDK](https://github.com/ps5-payload-dev/sdk) when building from
  source.

## Build and test

Run these commands from WSL or Linux at the repository root:

```sh
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
make test
make format-check
make
```

The PS5 build produces `headless-linkdev.elf`. Generated ELF files are
ignored by Git.

## Run on PS5

Replace `<PS5-IP>` with the console's address:

```sh
make send PS5_HOST=<PS5-IP>
```

The equivalent direct command is:

```sh
socat -t 99999999 - TCP:<PS5-IP>:9021 < headless-linkdev.elf
```

Keep the terminal connected while pairing. The PIN expires after 300 seconds;
send the payload again to generate a new one.

## Pair with chiaki-ng

Open the discovered console or manually register a PS5 using the values printed
in the terminal:

| Field | Value |
| --- | --- |
| Host | PS5 IP address |
| Console | PS5 |
| PSN Account-ID | Base64 `Account ID` printed by the payload |
| Remote Play PIN | Eight-digit `PIN` printed by the payload |

Complete registration before the timeout. See the
[chiaki-ng registration guide](https://streetpea.github.io/chiaki-ng/setup/configuration/)
for the current client UI flow.

## Behavior and safety

The payload sets the global Remote Play registry flag and, when available, the
foreground user's Remote Play flag to `1`. It reads the user's Account ID but
does not replace it, fake-activate the account, or modify save data.

The payload runs for one registration attempt and exits after success, a
pairing error, or the five-minute timeout.

## Troubleshooting

| Output or symptom | Meaning |
| --- | --- |
| Nothing appears | Keep `socat` connected and confirm elfldr is listening on port `9021` |
| `Failed to identify the foreground user` | Log into the intended PS5 user and leave it active |
| `Incorrect PIN` | Send the payload again and enter all eight digits of the new PIN |
| `Incorrect Account ID` | Copy the Base64 value exactly, including a trailing `=` |
| `Pairing expired` | Send the payload again to start a new five-minute attempt |

## Repository layout

| Path | Purpose |
| --- | --- |
| [`main.c`](main.c) | Payload and bounded host self-test |
| [`Makefile`](Makefile) | Test, format, PS5 build, and elfldr send targets |
| [`.github/workflows/build.yml`](.github/workflows/build.yml) | CI artifact and tagged release workflow |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | Validation and pull-request checklist |
| [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) | Upstream attribution |

## Project references

| Project | Role |
| --- | --- |
| [LinkDev](https://github.com/ps5-payload-dev/linkdev) | Parent project and original PS5 pairing implementation |
| [ActRemoteLink](https://github.com/francoataffarel/ActRemoteLink) | Direct Remote Play API implementation used for the headless payload |
| [ps5-remoteplay-get-pin](https://github.com/idlesauce/ps5-remoteplay-get-pin) | Earlier headless PIN-generation research |
| [OffAct](https://github.com/ps5-payload-dev/offact) | Offline account and registry research |
| [PS5 Payload SDK](https://github.com/ps5-payload-dev/sdk) | Prospero compiler, linker, headers, and ELF runtime |
| [chiaki-ng](https://github.com/streetpea/chiaki-ng) | Open-source Remote Play client used for pairing validation |

The SDK is a build-time prerequisite and is not copied into this repository.
See [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) before redistributing
modified builds.

## CI and releases

The [build workflow](.github/workflows/build.yml) runs formatting, the host
self-test, and the PS5 build on every push and pull request. Successful builds
upload the ELF as a workflow artifact.

Tags beginning with `v` build the tagged commit, create a GitHub Release with
generated notes, and attach `headless-linkdev.elf`:

```sh
git tag v1.0.0
git push origin v1.0.0
```

Rerunning a tag workflow replaces the existing ELF asset.

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for the development checks and
PS5-validation expectations.

## License

Headless LinkDev is licensed under the
[GNU General Public License v3.0 or later](LICENSE). Third-party work retains
its respective attribution; see [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

This project is not affiliated with Sony Interactive Entertainment,
PlayStation, or chiaki-ng. Use it only with hardware and accounts you own.
