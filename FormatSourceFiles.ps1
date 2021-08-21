$PathToVS = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
$ClangFormat = Join-Path $PathToVS -ChildPath "VC\Tools\Llvm\bin\clang-format.exe"
$SourcePath = ".\Plugins\DistanceMatching\Source"
$FileExtensions = @(".h", ".cpp")

foreach ($File in Get-ChildItem -Recurse -Path $SourcePath | Where-Object {$_.extension -in $FileExtensions})
{
    & $ClangFormat -i $File.FullName
    Write-Host $File.FullName
}
