# Constantes

$SrcPath = Join-Path -Path $PSScriptRoot -Child '..\..\Sources\'
$CppFiles = Get-ChildItem -Path $SrcPath -Recurse | Where { $_.Extension -eq '.cpp' }

$ListUndocumentedApi = "ZwQueryInformationProcess"									#Hardcode UndocumentedApi

$GlobalApiCounter = 0
$GlobalUndocumentedApiCounter = 0
$GloabRatio = 0

# Main

Write-Host "---------------------------------------"

Write-Host "PI-Defender: Test_Undocumented_API.ps1"

Write-Host "`nFILES ANALYSED"
Write-Host "--------------"

foreach ($File in $CppFiles)
{
    $UndocumentedApiCounter = 0

	Write-Host "[+] $File"

    $Content = $File | Get-Content
	
	foreach ($UndocumentedApi in $ListUndocumentedApi)
	{
        $OnePassCount = (Select-String -InputObject $Content -Pattern $UndocumentedApi -AllMatches).Matches.Count

        if ($OnePassCount -gt 0) {
			Write-Host "`t - $UndocumentedApi [$OnePassCount time(s)]"
            $UndocumentedApiCounter += $OnePassCount
        }
	}

    $GlobalApiCounter += $UndocumentedApiCounter
	
	foreach ($DocumentedApi in Get-Content -Path (Join-Path -Path $PSScriptRoot -ChildPath ListApiDriver.txt))
	{	
        $GlobalApiCounter += (Select-String -InputObject $Content -Pattern $DocumentedApi -AllMatches).Matches.Count
	}

	$GlobalUndocumentedApiCounter += $UndocumentedApiCounter	
}



# Result

Write-Host "`nRESULTS"
Write-Host "--------------"

Write-Host "$GlobalApiCounter WinAPI calls"
Write-Host "$GlobalUndocumentedApiCounter undocumented API found"

$Ratio = $GlobalUndocumentedApiCounter / $GlobalApiCounter * 100
"Ratio : {0:0.##}%" -f $Ratio | Out-Host

Write-Host "`n---------------------------------------"

if($Ratio -le 5)
{
	return 1
}
else {
	return 0
}

