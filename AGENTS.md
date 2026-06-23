# Agent Guide: wifi-coprocessor

This file helps AI coding agents work safely and quickly in this repository.

## Scope

- Primary firmware target is ESP32-C6 in [main](main).
- Host-side STM32 project lives in [host/stm32-host-esp32-wcp](host/stm32-host-esp32-wcp) and uses a separate build flow.
- Do not mix ESP-IDF and STM32 toolchains in one change unless explicitly requested.

## Read First

- Protocol and command format: [README.md](README.md)
- ESP-IDF project setup: [CMakeLists.txt](CMakeLists.txt)
- Firmware component registration: [main/CMakeLists.txt](main/CMakeLists.txt)
- Runtime entry and init path: [main/main.c](main/main.c)
- QSPI interface contracts: [main/qspi.h](main/qspi.h)
- Board config options (pins, Wi-Fi TX power): [main/Kconfig.projbuild](main/Kconfig.projbuild)
- Flash partition layout: [partitions.csv](partitions.csv)
- Workspace ESP-IDF target/port settings: [.vscode/settings.json](.vscode/settings.json)

## Build, Flash, Monitor

For agent runs in VS Code, prefer the ESP-IDF command integration over raw shell commands.

- Build: use ESP-IDF build command
- Flash: use ESP-IDF flash command
- Monitor: use ESP-IDF monitor command
- Configure: use ESP-IDF menuconfig command
- Clean rebuild after environment/toolchain changes: use ESP-IDF full clean command, then build again

If shell commands are necessary, use standard ESP-IDF commands from repository root:

- idf.py build
- idf.py -p COM11 flash
- idf.py -p COM11 monitor
- idf.py menuconfig
- idf.py fullclean

## Project Conventions

- Keep firmware changes under [main](main) unless task explicitly targets host code.
- Respect existing C style in firmware files (header metadata block, grouped sections, explicit typedef naming).
- Use Kconfig values instead of hardcoded GPIO/power constants when options exist.
- Treat [build](build) as generated output; do not commit manual edits there.

## Known Pitfalls

- On Windows, linker failures mentioning collect2.exe CreateProcess can be policy/security blocking, not source errors.
- After changing ESP-IDF path/toolchain, stale generated files can keep old paths (for example in build.ninja). Run full clean before re-building.
- Ensure target/toolchain matches ESP32-C6 expectations in workspace settings before diagnosing compile errors.

## Boundaries And Dependencies

- ESP firmware uses ESP-IDF components declared in [main/CMakeLists.txt](main/CMakeLists.txt) (driver, esp_driver_gpio, esp_driver_spi, esp_wifi, nvs_flash, esp_rom).
- Storage partition is littlefs-backed per [partitions.csv](partitions.csv) and managed components in [managed_components](managed_components).

## When Editing

- Keep changes minimal and focused on requested scope.
- Avoid protocol changes unless [README.md](README.md) is updated in the same task.
- If behavior depends on board pin mapping or TX power, include corresponding Kconfig updates.
