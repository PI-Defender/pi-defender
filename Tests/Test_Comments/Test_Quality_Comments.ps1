# Variables
$SrcPath = Join-Path -Path $PSScriptRoot -Child '..\..\Sources\'
$SrcFiles = Get-ChildItem -Path $SrcPath -Recurse | Where { '.hpp','.cpp' -contains $_.Extension }

## Regex
$RegexSingleLineComment = '\/\/[^\n\r]+?'
$RegexBlankLine = '^\s*$'
$RegexMultiLineOpenComment = '^\s*/\*'
$RegexMultiLineCloseComment = '^\s*\*\/'


# Main
function Get-CommentRatio($SrcFiles)
{
	$TotalRatio = 0
	$NbFile = 0
	
	foreach ($File in $SrcFiles)
	{
		$NbFile += 1

		$LineCounter = 0
		$Ratio = 0
		$CommentLineCounter = 0
		
		foreach ($Line in Get-Content -Path $File.FullName)
		{
            # Skip blank lines
            if ($Line -match $RegexBlankLine) {
				continue
			}

            # Count lines
            $LineCounter += 1

            # Count "//" comments
            if ($Line -match $RegexSingleLineComment) {
			    $CommentLineCounter += 1
			}
			
            # Check for "/*" comments
			elseif ($Line -match $RegexMultiLineOpenComment) {
				$LineBeginMultiComment = $LineCounter
			}
			
            # End of multiline comments: (current_line - line_begin_comment)
			elseif ($Line -match $RegexMultiLineCloseComment) {
				$CommentLineCounter += $LineCounter - $LineBeginMultiComment
			}
		}

		$Ratio = $CommentLineCounter / $LineCounter * 100
		$TotalRatio += $Ratio
		"  $File`t{0:0.##}%" -f $Ratio | Out-Host
	}
	
	return $TotalRatio / $NbFile
}


Write-Host "PI-Defender: Test_Quality_Comments.ps1"
Write-Host "`n- PI-Defender -"
Write-Host "`nFILES ANALYSED"
Write-Host "--------------"
Write-Host "SOURCE FILES:"
$GlobalRatio = Get-CommentRatio $SrcFiles

"`nGlobal ratio : {0:0.##}%" -f $GlobalRatio | Out-Host

if($GlobalRatio -ge 20)
{
	return 1
}
else {
	return 0
}

