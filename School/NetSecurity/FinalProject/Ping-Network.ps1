[CmdletBinding(ConfirmImpact="None")]
PARAM
(
)
BEGIN
{
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
        for ($i = 0; $i -lt 50; $i++)
        {
            $Reply = Test-Connection -Ping "$IpPrefix.$i" -Count 1 -TimeoutSeconds 2

            Write-Verbose "Status for $IpPrefix.$i, $($Reply.Replies.Status)"
            if ($Reply.Replies.Status -eq "Success")
            {
                $PingTable += $Reply.Replies
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