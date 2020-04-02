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

    $VendorListRaw = (Invoke-WebRequest -Uri "https://gitlab.com/wireshark/wireshark/raw/master/manuf").Content
    $VendorListContent = $VendorListRaw.Split("`n") | Where-Object {-not ($_.StartsWith("#"))} | Where-Object {$_.Trim() -ne ""}

    $LongVendorTable = @{}
    $ShortVendorTable = @{}
    foreach ($Entry in $VendorListContent)
    {
        $SplitData = [regex]::split($Entry, '[\s]')
        $Description = Join-String -InputObject $SplitData[2..($SplitData.Length-1)] -Separator " "
        if ($SplitData[0].Length -eq 8)
        {
            $ShortVendorTable.($SplitData[0]) = @($SplitData[1], $Description)
        }
        elseif ($SplitData[0].Length -eq 20)
        {
            $LongVendorTable.($SplitData[0]) = @($SplitData[1], $Description)
        }
        
    }
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
        $IpAddrList = @()
        for ($i = 0; $i -lt $MaxValue; $i++)
        {
            $Reply = Test-Connection -Ping "$IpPrefix.$i" -Count 1 -TimeoutSeconds 2
           
            Write-Verbose "$($Reply.Reply)"
            
            Write-Verbose "Status for $IpPrefix.$i, $($Reply.Reply.Status)"
            if ($Reply.Reply.Status -eq "Success")
            {
                $PingTable += , $Reply.Reply.Address.IPAddressToString
                $IpAddrList += , "$IpPrefix.$i"
            }
        }

        $NeighborList = Get-NetNeighbor -IPAddress $PingTable
        $MacList = $NeighborList.LinkLayerAddress.Replace("-",":")
        $MacPrefixList = @()
        foreach ($MAC in $MacList)
        {
            $MacPrefixList += Join-String -InputObject $MAC[0..7]
        }
        
        $MatchedVendorList = @()
        $i = 0
        foreach ($MacPrefix in $MacPrefixList)
        {
            $MatchedVendor = $ShortVendorTable.$MacPrefix
            if ($MatchedVendor -ne $null)
            {
                $MatchedVendorList += $MatchedVendor[0]
                $MatchedIPList += , $PingTable[$i]
            }
            else 
            {
                Write-Warning "Unable to find vendor for prefix $MacPrefix."    
            }
            $i++
        }

        $Results = @()
        $i = 0
        foreach ($MatchedVendor in $MatchedVendorList)
        {
            $Result = New-Object -TypeName "PSObject"
            Add-Member -InputObject $Result -NotePropertyName "Vendor" -NotePropertyValue $MatchedVendor
            Add-Member -InputObject $Result -NotePropertyName "IpAddr" -NotePropertyValue $MatchedIPList[$i]
            Add-Member -InputObject $Result -NotePropertyName "MACAddr" -NotePropertyValue (Get-NetNeighbor -IPAddress $MatchedIPList[$i]).LinkLayerAddress
            $i++
            $Results += $Result
        }

        Return $Results
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