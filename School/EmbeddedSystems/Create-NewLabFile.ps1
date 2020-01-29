[CmdletBinding(ConfirmImpact="None")]
PARAM
(
	[Parameter(Position=100,Mandatory=$false)]
	[ValidateNotNullOrEmpty()]
    [String] $TemplatePath = "$SchoolPath/School/EmbeddedSystems/_Template-Core.c",
    
    [Parameter(Position=101,Mandatory=$true)]
	[ValidateNotNullOrEmpty()]
    [String] $NewFile,
    
    [Parameter(Position=102,Mandatory=$false)]
	[ValidateNotNullOrEmpty()]
    [String] $Author = "Alex Israels",

	[Parameter]
    [Switch] $FullPath
)
PROCESS
{
	#region - Template code - Core - Try Begin
	$ExitCode = 0
	try
	{
		#endregion

        if (-not($FullPath))
        {
            $NewFilePath = "$SchoolPath/School/EmbeddedSystems/Lab/$NewFile"
        }
        else 
        {
            $NewFilePath = $NewFile    
        }

        $Origin = Get-Date -UFormat "%Y.%m.%d"
        $Template = Get-Content -Path $TemplatePath

        foreach ($Line in $Template)
        {
            if ($Line.Trim() -clike "*Orig:*")
            {
                $OIndex = $Template.IndexOf($Line)
                $Template[$OIndex] = "$($Line.Trim())  $Origin - $Author"
            }
        }

        Set-Content -Path $NewFilePath -Value $Template

		#region - Template code - Core - Try End
	}
	catch
	{
		Write-Information "Error on line $($_.InvocationInfo.ScriptLineNumber)."
		Write-Error $_
		$ExitCode = 1
	}
	#endregion
}
END
{
	Exit $ExitCode
}