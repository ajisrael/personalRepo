[CmdletBinding(ConfirmImpact="None")]
PARAM
(
    [Parameter(Position=100,Mandatory=$false)]
	[ValidateNotNullOrEmpty()]
    [Int16] $MaxValue = 256,

    [Parameter(Position=101,Mandatory=$false)]
	[ValidateNotNullOrEmpty()]
    [String] $Interface = "enp0s3",

	[Parameter()]
    [Switch] $FullComparison,

    [Parameter()]
    [Switch] $LocalDB
)
BEGIN
{
    # Check for correct version and editon of PowerShell.
    if ($PSVersionTable.PSVersion.Major -lt 7)
    {
        Write-Error "$($PSVersionTable.PSVersion) is not supported. Please update to at least version 7.0.0 and try again."
    }
    if ($PSVersionTable.PSEdition -ne "Core")
    {
        Write-Error "Only PowerShell Core is supported. Please run using PowerShell Core."
    }
    $HostIsWindows = ($PSVersionTable.Platform -eq "Win32NT")

    # Progress Bar Setup
    try
    {
        $TotalMainSteps = (Get-Content $PSCommandPath | Select-String -Pattern "[StatusStepCountFlag]" -SimpleMatch).Length - 1 # Exclude this line of the pattern itself
        $MainActivity = "Mapping IP Address to Manufacturer:"
        $MainStatusText = '"Step $($MainStepNum.ToString().PadLeft($TotalMainSteps.Count.ToString().Length)) of $TotalMainSteps | $StepDescription"' # Single quotes need to be on the outside
        $MainStatusBlock = [ScriptBlock]::Create($MainStatusText) # This script block allows the string above to use the current values of embedded values each time it's run
        $MainProgressID = 1
    }
    catch
    {
        Write-Error "Error in Progress Bar Setup."
        return
    }

    # Progress Bar Details - Do not change this comment [StatusStepCountFlag]
    $MainStepNum = 1
    $StepDescription = "Getting OUI data from WireShark database."
    if ($LocalDB) {$StepDescription = "Getting OUI data from local copy of WireShark database."}
    Write-Progress -Id $MainProgressID -Activity $MainActivity -Status (& $MainStatusBlock) -PercentComplete (($MainStepNum  - 1) / $TotalMainSteps * 100)

    # Get host IP Address.
    if ($IsWindows)
    {
        Invoke-Expression '$PersonalIPAddress = (Get-NetIPConfiguration | Where-Object {$_.InterfaceAlias -eq "Wi-Fi"}).IPv4Address.IPAddress'
    }
    elseif($IsMacOS) 
    {
        Invoke-Expression '$RawIPAddressData = ifconfig -a | grep "inet "'
        $RawIPAddressList = $RawIPAddressData.Split('`n')
        $PersonalIPAddressList = @()
        foreach ($RawIPAddressData in $RawIPAddressList)
        {
            $PersonalIPAddressList += , $RawIPAddressData.Trim().Split(" ")[1]
        }
        #TODO: Figure out a better way to identify local IP address from list
        $PersonalIPAddress = $PersonalIPAddressList[1]
    }
    elseif($IsLinux)
    {
        Invoke-Expression '$RawIPAddressData = ip address show | grep "inet "'
        $RawIPAddressList = $RawIPAddressData.Split('`n')
        $PersonalIPAddressList = @()
        foreach ($RawIPAddressData in $RawIPAddressList)
        {
            $PersonalIPAddressList += , $RawIPAddressData.Trim().Split(" ")[1].Split("/")[0]
        }
        #TODO: Figure out a better way to identify local IP address from list
        $PersonalIPAddress = $PersonalIPAddressList[1]
    }
    else
    {
        Write-Error "$($PSVersionTable.OS) is not supported."
    }

    # Get a list of vendor OUI's from wireshark's database.
    if (-not($LocalDB))
    {
        $VendorListRaw = (Invoke-WebRequest -Uri "https://gitlab.com/wireshark/wireshark/raw/master/manuf").Content
        Set-Content -Path "$PSScriptRoot/wireSharkDB.txt" -Value $VendorListRaw
        $VendorListContent = $VendorListRaw.Split("`n") | Where-Object {-not ($_.StartsWith("#"))} | Where-Object {$_.Trim() -ne ""}
    }
    else 
    {
        $VendorListContent = Get-Content -Path "$PSScriptRoot/wireSharkDB.txt" | Where-Object {-not ($_.StartsWith("#"))} | Where-Object {$_.Trim() -ne ""}
    }
    

    # Separate the list into short and long OUI's.
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

    # Clean up memory so the large files aren't still allocated.
    $VendorListRaw = $null
    $VendorListContent = $null
}
PROCESS
{
	#region - Template code - Core - Try Begin
	$ExitCode = 0
	try
	{
		#endregion

        # Get the ip prefix.
        $IpPrefix = ([String] $PersonalIPAddress.Split(".")[0..2]).Replace(" ",".")

        # Progress Bar Details - Do not change this comment [StatusStepCountFlag]
        $MainStepNum++
        $StepDescription = "Scanning local network"
        Write-Progress -Id $MainProgressID -Activity $MainActivity -Status (& $MainStatusBlock) -PercentComplete (($MainStepNum  - 1) / $TotalMainSteps * 100)

        # Progress Bar Setup
        try
        {
            $TotalScanSteps = $MaxValue
            $ScanActivity = "Scanning local network..."
            $ScanStatusText = '"Step $($ScanStepNum.ToString().PadLeft($TotalScanSteps.Count.ToString().Length)) of $TotalScanSteps | $ScanDescription"' # Single quotes need to be on the outside
            $ScanStatusBlock = [ScriptBlock]::Create($ScanStatusText) # This script block allows the string above to use the current values of embedded values each time it's run
            $ScanProgressID = 2
        }
        catch
        {
            Write-Error "Error in Progress Bar Setup."
            return
        }

        # Get a list of active IP addresses on the network.
        $IPAddressList = @()
        $NeighborList = @()
        $ScanStepNum = 0
        for ($i = 0; $i -lt $MaxValue; $i++)
        {
            # Progress Bar Details - Do not change this comment [ScanCountFlag]
            $ScanStepNum++
            $ScanDescription = "Pinging $IpPrefix.$i"
            Write-Progress -Id $ScanProgressID -Activity $ScanActivity -Status (& $ScanStatusBlock) -PercentComplete ($ScanStepNum / $TotalScanSteps * 100)

            $Reply = Test-Connection -Ping "$IpPrefix.$i" -Count 1 -TimeoutSeconds 2
            
            Write-Verbose "$($IpPrefix.$i): Status = $($Reply.Reply.Status)"
            if ($Reply.Reply.Status -eq "Success")
            {
                $IPAddressList += , $Reply.Reply.Address.IPAddressToString
                if ($IsLinux -and $Reply.Reply.Address.IPAddressToString -ne $PersonalIPAddress)
                {
                    $Command = 'ip neighbor | grep "' + $Reply.Reply.Address.IPAddressToString + ' "'
                    $NeighborRaw = Invoke-Expression -Command $Command
                    $MACAddress = $NeighborRaw.Trim().Split(" ")[4]
                    $IPAddress = $NeighborRaw.Trim().Split(" ")[0]
                    $Neighbor = New-Object -TypeName "PSObject"
                    Add-Member -InputObject $Neighbor -NotePropertyName "LinkLayerAddress" -NotePropertyValue $MACAddress
                    Add-Member -InputObject $Neighbor -NotePropertyName "IPAddress" -NotePropertyValue $IPAddress
                    $NeighborList += $Neighbor
                }
            }
        }

        # Filter out host IPAddress.
        $IPAddressList = $IPAddressList | Where-Object {$_ -ne $PersonalIPAddress}

        # Progress Bar Details - Do not change this comment [StatusStepCountFlag]
        $MainStepNum++
        $StepDescription = "Aquiring MAC addresses for local network devices."
        Write-Progress -Id $MainProgressID -Activity $MainActivity -Status (& $MainStatusBlock) -PercentComplete (($MainStepNum  - 1) / $TotalMainSteps * 100)

        # Get the corresponding MAC address of the active IP addresses.
        if ($IsWindows)
        {
            $NeighborList = Get-NetNeighbor -IPAddress $IPAddressList
        }
        elseif ($IsMacOS)
        {
            Write-Error "Need to implement lookup for MACOS."
            return
        }
        
        # Progress Bar Details - Do not change this comment [StatusStepCountFlag]
        $MainStepNum++
        $StepDescription = "Generating report."
        Write-Progress -Id $MainProgressID -Activity $MainActivity -Status (& $MainStatusBlock) -PercentComplete (($MainStepNum  - 1) / $TotalMainSteps * 100)

        # Compare the MAC address against the OUI table and build an array of result objects to report on.
        $Report = @()
        foreach ($Neighbor in $NeighborList)
        {
            $MACAddress = $Neighbor.LinkLayerAddress.Replace("-",":")
            $MACPrefix = Join-String -InputObject $MACAddress[0..7]

            try
            {
                $HostName = (Test-Connection -Ping $Neighbor.IPAddress -Count 1 -timeoutSeconds 2 -ResolveDestination).Destination
            }
            catch
            {
                $HostName = "?"
                Write-Verbose $_
            }

            $Result = New-Object -TypeName "PSObject"
            Add-Member -InputObject $Result -NotePropertyName "Vendor"     -NotePropertyValue "N/A"
            Add-Member -InputObject $Result -NotePropertyName "IPAddress"  -NotePropertyValue $Neighbor.IPAddress
            Add-Member -InputObject $Result -NotePropertyName "MACAddress" -NotePropertyValue $MACAddress
            Add-Member -InputObject $Result -NotePropertyName "HostName"   -NotePropertyValue $HostName
            Add-Member -InputObject $Result -NotePropertyName "VendorFullName" -NotePropertyValue "N/A"
            
            $VendorData = $null
            if ($FullComparison)
            {
                $VendorData = $LongVendorTable."$MACAddress/36"
            }
            
            if ($null -eq $VendorData)
            {
                $VendorData = $ShortVendorTable.$MACPrefix
            }
            
            if ($VendorData)
            {
                $Result.Vendor = $VendorData[0]
                $Result.VendorFullName = $VendorData[1]
            }
        
            $Report += $Result
        }

        # Find the access point in the report
        $AccessPoint = $Report | Where-Object {$_.IPAddress -eq "$IpPrefix.1"}
        $Report = $Report | Where-Object {$_.IPaddress -ne "$IpPrefix.1"}

        foreach ($Result in $Report)
        {
            $PythonArgs  = (Resolve-Path -Path "$PSScriptRoot/PyDeauth.py").Path
            $PythonArgs += " " + $AccessPoint.MACAddress + " " + $Result.MACAddress + " " + $Interface
            $PythonCommand = [ScriptBlock]::Create("/usr/bin/python3 $PythonArgs")

            Write-Information (Invoke-Command -ScriptBlock $PythonCommand).ToString
        }

        # Return the results.
        Return $Report

		#region - Template code - Core - Try End
	}
	catch
	{
		Write-Host "Error on line $($_.InvocationInfo.ScriptLineNumber)."
		Write-Error $_
		$ExitCode = 1
	}
	#endregion
}
END
{
	Exit $ExitCode
}