[CmdletBinding(ConfirmImpact="None")]
PARAM
(
    [Parameter(Position=100,Mandatory=$false)]
	[ValidateNotNullOrEmpty()]
    [Int16] $MaxValue = 256
)
BEGIN
{
    if ($PSVersionTable.PSVersion.Major -lt 7)
    {
        Write-Error "$($PSVersionTable.PSVersion) is not supported. Please update to at least version 7.0.0 and try again."
    }
    if ($PSVersionTable.PSEdition -ne "Core")
    {
        Write-Error "Only PowerShell Core is supported. Please run using PowerShell Core."
    }
    Invoke-Expression '$IPAddress = (Get-NetIPConfiguration | Where-Object {$_.InterfaceAlias -eq "Wi-Fi"}).IPv4Address.IPAddress'

}
PROCESS
{
	#region - Template code - Core - Try Begin
	$ExitCode = 0
	try
	{
		#endregion

        # Get the ip prefix
        $IpPrefix = ([String] $IPAddress.Split(".")[0..2]).Replace(" ",".")

        $PingTable = @()
        for ($i = 0; $i -lt $MaxValue; $i++)
        {
            $Reply = Test-Connection -Ping "$IpPrefix.$i" -Count 1 -TimeoutSeconds 2
           
            Write-Verbose "$($Reply.Reply)"
            
            Write-Verbose "Status for $IpPrefix.$i, $($Reply.Reply.Status)"
            if ($Reply.Reply.Status -eq "Success")
            {
                $PingTable += , $Reply.Reply.Address.IPAddressToString
            }
                
        }
        return $PingTable
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