# fix_stm32cubeide_paths.ps1
# Ejecutar desde la carpeta raiz del proyecto:
# ...\STM32Workspace\ST67W6X_HTTP_Server
#
# Esa carpeta debe contener:
# Appli, Core, Drivers, Middlewares, ST67W6X, STM32CubeIDE, Utilities

$ErrorActionPreference = "Stop"

$cproject = ".\STM32CubeIDE\.cproject"

if (!(Test-Path $cproject)) {
    Write-Error "No encuentro .\STM32CubeIDE\.cproject. Ejecuta este script desde la carpeta raiz del proyecto."
}

# Limpia builds viejos
Remove-Item ".\STM32CubeIDE\Debug" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item ".\STM32CubeIDE\Release" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item ".\Debug" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item ".\Release" -Recurse -Force -ErrorAction SilentlyContinue

# Arregla rutas relativas absurdamente largas del ejemplo copiado
$txt = Get-Content $cproject -Raw

$txt = $txt.Replace("../../../../../../../Drivers", "../../Drivers")
$txt = $txt.Replace("../../../../../../../Middlewares", "../../Middlewares")
$txt = $txt.Replace("../../../../../../../Utilities", "../../Utilities")

$txt = $txt.Replace("..\..\..\..\..\..\..\Drivers", "..\..\Drivers")
$txt = $txt.Replace("..\..\..\..\..\..\..\Middlewares", "..\..\Middlewares")
$txt = $txt.Replace("..\..\..\..\..\..\..\Utilities", "..\..\Utilities")

# Si el proyecto se ha renombrado, descomenta esta linea:
# $txt = $txt.Replace("ST67W6X_HTTP_Server", "Window_control_wifi")

Set-Content -Path $cproject -Value $txt -NoNewline

Write-Host "OK: .cproject parcheado."
Write-Host "Ahora abre STM32CubeIDE, refresca el proyecto, Project > Clean y Build."
