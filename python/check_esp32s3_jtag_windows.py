import json
import subprocess
import sys
from typing import Any, Dict, List


def run_powershell(command: str) -> str:
    """Run a PowerShell command and return stdout as text."""
    completed = subprocess.run(
        ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-Command", command],
        capture_output=True,
        text=True,
    )

    if completed.returncode != 0:
        raise RuntimeError(completed.stderr.strip() or "PowerShell command failed")

    return completed.stdout.strip()


def normalize_devices(raw: Any) -> List[Dict[str, Any]]:
    if raw is None:
        return []
    if isinstance(raw, list):
        return raw
    if isinstance(raw, dict):
        return [raw]
    return []


def get_vid_pid_devices(vid: str = "303A", pid: str = "1001") -> List[Dict[str, Any]]:
    instance_regex = f"VID_{vid}&PID_{pid}"
    ps = (
        "Get-PnpDevice -PresentOnly "
        f"| Where-Object {{ $_.InstanceId -match '{instance_regex}' }} "
        "| Select-Object Status,Class,FriendlyName,InstanceId "
        "| ConvertTo-Json -Depth 3"
    )

    out = run_powershell(ps)
    if not out:
        return []

    try:
        parsed = json.loads(out)
    except json.JSONDecodeError as exc:
        raise RuntimeError(f"Could not parse PowerShell JSON output: {exc}") from exc

    return normalize_devices(parsed)


def main() -> int:
    if sys.platform != "win32":
        print("ERROR: This script is Windows-only.")
        return 2

    try:
        devices = get_vid_pid_devices()
    except Exception as exc:
        print(f"ERROR: {exc}")
        return 2

    if not devices:
        print("ESP32-S3 USB device not found (VID_303A&PID_1001).")
        return 1

    has_mi00_serial = any("MI_00" in str(d.get("InstanceId", "")) for d in devices)
    has_mi02_jtag = any("MI_02" in str(d.get("InstanceId", "")) for d in devices)

    print("Detected interfaces for VID_303A&PID_1001:")
    for d in devices:
        status = d.get("Status", "")
        dev_class = d.get("Class", "")
        name = d.get("FriendlyName", "")
        instance_id = d.get("InstanceId", "")
        print(f"- [{status}] {dev_class} | {name} | {instance_id}")

    print()
    print(f"Serial MI_00 present: {'YES' if has_mi00_serial else 'NO'}")
    print(f"JTAG   MI_02 present: {'YES' if has_mi02_jtag else 'NO'}")

    if has_mi00_serial and has_mi02_jtag:
        print("OK: JTAG and Serial interfaces are present.")
        return 0

    print("FAIL: Missing MI_00 and/or MI_02 interface.")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
