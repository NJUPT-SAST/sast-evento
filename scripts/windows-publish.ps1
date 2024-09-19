# 外部环境变量包括:
# winSdkDir: ${{ steps.build.outputs.winSdkDir }}
# winSdkVer: ${{ steps.build.outputs.winSdkVer }}
# vcToolsInstallDir: ${{ steps.build.outputs.vcToolsInstallDir }}
# vcToolsRedistDir: ${{ steps.build.outputs.vcToolsRedistDir }}
# msvcArch: ${{ matrix.msvc_arch }}

$scriptDir = $PSScriptRoot
$currentDir = Get-Location
Write-Host "currentDir" $currentDir
Write-Host "scriptDir" $scriptDir

function Main() {
    New-Item -ItemType Directory dist
    # 拷贝exe
    Copy-Item build\src\Release\* dist -Force -Recurse | Out-Null
    Copy-Item build\src\Tray\Release\* dist -Force -Recurse | Out-Null
    # 拷贝依赖
    $windeployqt = 'windeployqt'
    if (${env:QT_HOST_PATH}.Length -ne 0) {
        $windeployqt = Join-Path -Path ${env:QT_HOST_PATH} -ChildPath 'bin\windeployqt'
    }
    & $windeployqt dist\sast-evento-tray.exe
    # 删除不必要的文件
    $excludeList = @("*.qmlc", "*.ilk", "*.exp", "*.lib", "*.pdb")
    Remove-Item -Path dist -Include $excludeList -Recurse -Force
    # 拷贝vcRedist dll
    $redistDll="{0}{1}\*.CRT\*.dll" -f $env:vcToolsRedistDir.Trim(),$env:msvcArch
    Copy-Item $redistDll dist\
    # 拷贝WinSDK dll
    $sdkDll="{0}Redist\{1}ucrt\DLLs\{2}\*.dll" -f $env:winSdkDir.Trim(),$env:winSdkVer.Trim(),$env:msvcArch
    Copy-Item $sdkDll dist\
    # 打包zip
    Compress-Archive -Path dist "sast-evento-$env:msvcArch.zip"
}

Main