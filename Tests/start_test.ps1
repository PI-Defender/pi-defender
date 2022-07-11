#Variables
$Date = Get-Date -DisplayHint Date
$Version = "V1"

$CounterTotalTest = 0
$CounterTestSkip = 0
$CounterTestOK = 0
$CounterTestKO = 0


# Main

Write-Host "- PI-Defender: Test -"
Write-Host "[i] Date: $Date"
Write-Host "[i] Version: $Version"
Write-Host "=============================="


############################################

# VL-01-01/01

Start-Service -Name 'PI-Defender_UM'

If(Get-Service PI-Defender_UM | Where-Object Status -eq "Running")
{
	Write-Host -ForegroundColor Green "`n[+] VL-01-01/01 - OK"
	$CounterTestOK += 1
}
else {
	Write-Host -ForegroundColor Red "`n[-] VL-01-01/01 - KO"
	$CounterTestKO += 1
}

$CounterTotalTest += 1


############################################

# VL-01-01/02

Stop-Service -Name 'PI-Defender_UM'

If(Get-Service PI-Defender_UM | Where-Object Status -eq "Stopped")
{
	Write-Host -ForegroundColor Green "`n[+] VL-01-01/02 - OK" 
	$CounterTestOK += 1
}
else {
	Write-Host -ForegroundColor Red "`n[-] VL-01-01/02 - KO" 
	$CounterTestKO += 1
}

$CounterTotalTest += 1

Start-Service -Name 'PI-Defender_UM' # restart PI-Defender_UM after


############################################

# VL-02-01/01 TODO - Process Hollowing
# VL-02-01/02 TODO - Process Doppelgänging
# VL-02-01/03 TODO - Process Herpaderping
# VL-02-01/04 TODO - Process Ghosting
# VL-02-01/06 TODO - Process Overwriting
# VL-02-01/07 TODO - Dll Injection


############################################

# VL-02-01/09

Write-Host -ForegroundColor Magenta "`n[i] VL-02-01/09 - Lancer .\Test_ObCallback\Build\Release\Test_ObCallback.exe"
$CounterTestSkip += 1
$CounterTotalTest += 1


############################################

# VL-02-01/10 

Write-Host -ForegroundColor Magenta "`n[i] VL-02-01/09 - Lancer .\Test_ObCallback\Build\Release\Test_Performances.exe"
$CounterTestSkip += 1
$CounterTotalTest += 1


############################################

# VL-02-01/11

Write-Host -ForegroundColor Magenta "`n[i] VL-02-01/09 - Lancer .\Test_ObCallback\Build\Release\Test_Speed.exe"
$CounterTestSkip += 1
$CounterTotalTest += 1


############################################

# VL-02-01/12 TODO: A tester sur machine de test + rajouter utilisation du disque

$CpuLoadBefore = (Get-Counter '\Processeur(_total)\% temps processeur').CounterSamples.CookedValue
$CompObject = Get-WmiObject -Class WIN32_OperatingSystem
$RamUsedBefore = (($CompObject.TotalVisibleMemorySize - $CompObject.FreePhysicalMemory)/1024/1024)

Start-Service PI-Defender_UM

$CpuLoadAfter = (Get-Counter '\Processeur(_total)\% temps processeur').CounterSamples.CookedValue
$RamUsedAfter = (($CompObject.TotalVisibleMemorySize - $CompObject.FreePhysicalMemory)/1024/1024)

$RatioCpu = ($CpuLoadAfter - $CpuLoadBefore)
$RatioRam = ($RamUsedAfter - $RamUsedBefore)

If($RatioCpu -le 15 -and $RatioRam -le 1)
{
	Write-Host -ForegroundColor Green "`n[+] VL-02-01/12 - OK"
	$CounterTestOK += 1
}
else {
	Write-Host -ForegroundColor Red "`n[-] VL-02-01/12 - KO"
	$CounterTestKO += 1
}

$CounterTotalTest += 1



############################################

# VL-02-02/01

Show-EventLog

$Result = Read-Host -Prompt "`nAppuyer sur la touche `"o`", si aucune erreur est presente dans le journal d'evenements. Sinon, appuyer sur `"n`""

If($Result -eq "o")
{
	Write-Host -ForegroundColor Green "[+] VL-02-02/01 - OK"
	$CounterTestOK += 1
}
else {
	Write-Host -ForegroundColor Red "[-] VL-02-02/01 - KO"
	$CounterTestKO += 1
}

$CounterTotalTest += 1



############################################

# VL-02-02/02

Write-Host -ForegroundColor Yellow "`n[i] VL-02-02/02 - A tester sur une période longue (1 semaine sans interruption"

$CounterTestSkip += 1
$CounterTotalTest += 1



############################################

# VL-02-04/01

Write-Host -ForegroundColor Yellow "`n[i] VL-02-04/01 - Voir test VL-02-02/01"

$CounterTestSkip += 1
$CounterTotalTest += 1



############################################

# VL-02-04/02

Write-Host -ForegroundColor Yellow "`n[i] VL-02-04/02 - Non testable"

$CounterTestSkip += 1
$CounterTotalTest += 1



############################################

# VL-02-06/01

Get-Service PI-Defender_UM
Start-Service PI-Defender_UM
Stop-Service PI-Defender_UM
Write-Host -ForegroundColor Green "`n[+] VL-02-06/01 - OK"
$CounterTestOK += 1
$CounterTotalTest += 1



#############################################

# VL-02-07/01

$Result = Select-String -Path "..\Sources\PI-Defender\K_Communication.cpp" -Pattern "FltCreateCommunicationPort" -Quiet
If($?)
{
	Write-Host -ForegroundColor Green "`n[+] VL-02-07/01 - OK"
	$CounterTestOK += 1
}
else{
	Write-Host -ForegroundColor Red "`n[-] VL-02-07/01 - KO"
	$CounterTestKO += 1
}

$CounterTotalTest += 1



############################################
 
# VL-03-01/01 - Non testable
 
Write-Host -ForegroundColor Yellow "`n[i] VL-03-01/01 - Non testable"
 
$CounterTestSkip += 1
$CounterTotalTest += 1



############################################
 
# VL-03-01/02 - TODO
 
# Write-Host "`n[i] VL-03-01/02 - TODO"
 
 

############################################
 
# VL-03-04/01 - Non testable

Write-Host -ForegroundColor Yellow "`n[i] VL-03-04/01 - Non testable"

$CounterTestSkip += 1
$CounterTotalTest += 1



###########################################
 
# VL-03-04/02 - Non testable

Write-Host -ForegroundColor Yellow "`n[i] VL-03-04/02 - Non testable"

$CounterTestSkip += 1
$CounterTotalTest += 1



##########################################
 
# VL-03-04/03

$Result = &"./Test_API/Test_Undocumetend_API.ps1" | select -Last 1

if($Result -eq 1)
{
	Write-Host -ForegroundColor Green "`n[+] VL-03-04/03 - OK"
	$CounterTestOK += 1
}
else {
	Write-Host -ForegroundColor Red "`n[-] VL-03-04/03 - KO"
	$CounterTestKO += 1
}

$CounterTotalTest += 1



##########################################

# VL-03-04/04 - Non testable

Write-Host -ForegroundColor Yellow "`n[i] VL-03-04/04 - Non testable"

$CounterTestSkip += 1
$CounterTotalTest += 1



##########################################

# VL-03-06/01

$Result = &".\Test_Comments\Test_Quality_Comments.ps1"

if($Result -eq 1)
{
	Write-Host -ForegroundColor Green "`n[+] VL-03-06/01 - OK"
	$CounterTestOK += 1
}
else {
	Write-Host -ForegroundColor Red "`n[-] VL-03-06/01 - KO"
	$CounterTestKO += 1
}

$CounterTotalTest += 1



##########################################
 
# VL-03-08/01 - Non testable

Write-Host -ForegroundColor Yellow "`n[i] VL-03-08/01 - Non testable"

$CounterTestSkip += 1
$CounterTotalTest += 1



##########################################
 
# VL-03-08/02 - Non testable

Write-Host -ForegroundColor Yellow "`n[i] VL-03-08/02 - Non testable"

$CounterTestSkip += 1
$CounterTotalTest += 1



##########################################
 
# VL-03-08/03 - Non testable

Write-Host -ForegroundColor Yellow "`n[i] VL-03-08/03 - Non testable"

$CounterTestSkip += 1
$CounterTotalTest += 1



##########################################
 
# VL-03-18/01

$LicenseLGPLv3 = "GNU LESSER GENERAL PUBLIC LICENSE ..."

$Result = Get-ChildItem -Path "..\" -Filter "LICENSE.txt"

if ($Result -ne $null)
{
	$Match = Select-String -Path "..\LICENSE.txt" -Pattern "$LicenseLGPLv3"
	if ($Match -ne $null)
	{
		Write-Host -ForegroundColor Green "`n[+] VL-03-18/01 - OK"
		$CounterTestOK += 1
	}
}

if ($Result -eq $null -or $Match -eq $null)
{
	Write-Host -ForegroundColor Red "`n[-] VL-03-18/01 - KO"
	$CounterTestKO += 1
}

$CounterTotalTest += 1



##########################################
 
# VL-03-31/01

$RefHashDriver = "3FA68566D068850E1DC431A9CE15A7915919666A5D48BD23250A892AF9230950"
$HashDriver = Get-FileHash "..\Build\x64\release\PI-Defender.sys" -Algorithm SHA256 | Select-Object -ExpandProperty Hash

if ($RefHashDriver -ne $HashDriver)
{
	Write-Host -ForegroundColor Red "`n[-] VL-03-31/01 - KO"
	$CounterTestKO += 1
}
else{
	$RefHashService = "E9335B164D4D457A3B5B4C7972CCA8FE3102919565A7ED96AFC316006D1F03BC"
	$HashService = Get-FileHash "..\Build\x64\release\PI-Defender_UM.exe" -Algorithm SHA256 | Select-Object -ExpandProperty Hash
	if ($RefHashService -ne $HashService)
	{
		Write-Host -ForegroundColor Red "`n[-] VL-03-31/01 - KO"
		$CounterTestKO += 1
	}
	else {
		Write-Host -ForegroundColor Green "`n[+] VL-03-31/01 - OK"
		$CounterTestOK += 1
	}
}

$CounterTotalTest += 1



##########################################

Write-Host "`nFINAL RESULT"
Write-Host "-------------------"
Write-Host "Test OK: $CounterTestOK"
Write-Host "Test KO: $CounterTestKO"
Write-Host "Test Skip: $CounterTestSkip"

