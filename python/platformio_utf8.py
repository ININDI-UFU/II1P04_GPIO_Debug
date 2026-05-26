import os
from pathlib import Path


os.environ["PYTHONIOENCODING"] = "utf-8"
os.environ["PYTHONUTF8"] = "1"
os.environ["PLATFORMIO_DISABLE_PROGRESSBAR"] = "true"

Import("env")


def package_tool(*parts):
    return Path.home().joinpath(".platformio", "packages", *parts)


def force_esp32s3_gdb():
    gdb = package_tool(
        "tool-xtensa-esp-elf-gdb",
        "bin",
        "xtensa-esp32s3-elf-gdb.exe",
    )
    if gdb.is_file():
        env.Replace(GDB=str(gdb))
        return

    print(
        "Aviso: GDB do ESP32-S3 ainda nao foi encontrado. "
        "Rode o build uma vez para o PlatformIO instalar os pacotes."
    )


def apply_esp32s3_defaults():
    env.AppendUnique(CPPDEFINES=["ARDUINO_USB_CDC_ON_BOOT=0"])

    flags = env.get("UPLOADERFLAGS", [])
    if "write-flash" in flags and "--no-progress" not in flags:
        env.Append(UPLOADERFLAGS=["--no-progress"])


force_esp32s3_gdb()
apply_esp32s3_defaults()
