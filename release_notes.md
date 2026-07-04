### ApexPad Host Daemon - Execution Hotfix (v1.1.1)

This minor release addresses a silent crash issue introduced in the previous version when executing Python scripts via the background daemon.

#### Patch Notes

- **Fixed `pythonw.exe` Execution Trap:** Background script routing on Windows has been reverted from `pythonw.exe` to standard `python.exe`. This resolves an issue where scripts attempting to use standard output (like polling USB hubs or printing UI logs) would instantly crash due to the lack of an attached console stream.
- **Invisible Execution Maintained:** Although the execution engine now uses the standard `python.exe` binary, the `subprocess.CREATE_NO_WINDOW` flag remains active. This guarantees that background scripts and batch wrappers still execute completely silently without momentarily flashing a terminal window on your screen.
