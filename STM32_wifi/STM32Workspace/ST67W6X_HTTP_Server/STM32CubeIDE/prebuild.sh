#!/bin/bash - 
#prebuild script 
echo prebuild.sh : HTML generation started
script="$1/../Html/html_to_h.py"
config="$1/../Html/conf.txt"
output="$1/../Appli/App"

if command -v python3 >/dev/null 2>&1 && python3 --version >/dev/null 2>&1; then
  set -- python3 "$script" --config "$config" --output "$output"
elif command -v python >/dev/null 2>&1 && python --version >/dev/null 2>&1; then
  set -- python "$script" --config "$config" --output "$output"
elif command -v py >/dev/null 2>&1 && py -3 --version >/dev/null 2>&1; then
  set -- py -3 "$script" --config "$config" --output "$output"
elif command -v powershell.exe >/dev/null 2>&1; then
  python_exe=$(powershell.exe -NoProfile -Command 'Get-ChildItem -Path $env:LOCALAPPDATA -Recurse -Filter python.exe -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notlike "*WindowsApps*" } | Select-Object -First 1 -ExpandProperty FullName' | tr -d '\r')
  if [ -n "$python_exe" ] && command -v cygpath >/dev/null 2>&1; then
    python_exe=$(cygpath -u "$python_exe")
  fi
  if [ -n "$python_exe" ]; then
    set -- "$python_exe" "$script" --config "$config" --output "$output"
  else
    set -- false
  fi
else
  set -- false
fi

echo "$@"
"$@"
ret=$?

if [ $ret != 0 ]; then
  #an error
  echo "$@" : failed
  exit 1
fi

exit 0
