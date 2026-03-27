#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"

to_windows_path() {
  local path="$1"

  if command -v cygpath >/dev/null 2>&1; then
    cygpath -w "$path"
    return
  fi

  if command -v wslpath >/dev/null 2>&1; then
    wslpath -w "$path"
    return
  fi

  printf '%s\n' "$path"
}

find_powershell() {
  if command -v powershell.exe >/dev/null 2>&1; then
    command -v powershell.exe
    return
  fi

  if command -v pwsh.exe >/dev/null 2>&1; then
    command -v pwsh.exe
    return
  fi

  if command -v powershell >/dev/null 2>&1; then
    command -v powershell
    return
  fi

  if command -v pwsh >/dev/null 2>&1; then
    command -v pwsh
    return
  fi

  printf 'PowerShell was not found. Use fw.ps1 directly or install PowerShell.\n' >&2
  exit 1
}

script_path_win="$(to_windows_path "$script_dir/fw.ps1")"
powershell_bin="$(find_powershell)"

exec "$powershell_bin" -ExecutionPolicy Bypass -File "$script_path_win" "$@"
