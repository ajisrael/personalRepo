function Test-AddressSearch
{
	[CmdletBinding(ConfirmImpact="None")]
	PARAM
	(
		[Parameter(Position=100)]
		[ValidateNotNullOrEmpty()]
		[String] $FullName,

		[Parameter(Position=101)]
		[ValidateNotNullOrEmpty()]
		[String] $State
	)
	BEGIN
	{
		$FirstName = $FullName.Split()[0]
		$LastName = $FullName.Split()[1]

		$StateID = $StateTable.Keys | Where-Object {$StateTable.$_ -eq  $State}

		$ADSearchURL = "https://www.addresssearch.com/results.php?type=forward&fname=$FirstName&lname=$LastName&state=$StateID&x=62&y=30"

		$Resp = Invoke-WebRequest -Uri $ADSearchURL

		$Paragraphs = $Resp.RawContent.Split("<p>")
		
		$MissingMessage = "We're sorry, we couldn't find any results.</p>"

		$Result = $false
		if ($Paragraphs[1] -ne $MissingMessage)
		{
			$Result = $true
		}

		return $Result
	}
}
Export-ModuleMember -Function Test-AddressSearch

function Test-BeenVerified
{
	[CmdletBinding(ConfirmImpact="None")]
	PARAM
	(
		[Parameter(Position=100)]
		[ValidateNotNullOrEmpty()]
		[String] $FullName,

		[Parameter(Position=101)]
		[ValidateNotNullOrEmpty()]
		[String] $State
	)
	BEGIN
	{
		$FirstName = $FullName.Split()[0]
		$LastName = $FullName.Split()[1]

		$StateID = $StateTable.Keys | Where-Object {$StateTable.$_ -eq  $State}

		$BeenVerifiedURL = "https://www.beenverified.com/app/search/person?fn=$FirstName&ln=$LastName&optout=true&state=$StateID"

		$Resp = Invoke-WebRequest -Uri $BeenVerifiedURL

		### TODO: Determine how to tell a hit from a miss based on response
		
		return $Result
	}
}
Export-ModuleMember -Function Test-BeenVerified

function Test-BeenVerified
{
	[CmdletBinding(ConfirmImpact="None")]
	PARAM
	(
		[Parameter(Position=100)]
		[ValidateNotNullOrEmpty()]
		[String] $FullName,

		[Parameter(Position=101)]
		[ValidateNotNullOrEmpty()]
		[String] $State
	)
	BEGIN
	{
		$FirstName = $FullName.Split()[0]
		$LastName = $FullName.Split()[1]

		$StateID = $StateTable.Keys | Where-Object {$StateTable.$_ -eq  $State}

		$BeenVerifiedURL = "https://www.beenverified.com/app/search/person?fn=$FirstName&ln=$LastName&optout=true&state=$StateID"

		$Resp = Invoke-WebRequest -Uri $BeenVerifiedURL

		### TODO: Determine how to tell a hit from a miss based on response
		
		return $Result
	}
}
Export-ModuleMember -Function Test-BeenVerified

$StateTable = @{
	AL = "Alabama"
	AK = "Alaska"
	AZ = "Arizona"
	AR = "Arkansas"
	CA = "California"
	CO = "Colorado"
	CT = "Connecticut"
	DC = "Washington DC"
	DE = "Delaware"
	FL = "Florida"
	GA = "Georgia"
	HI = "Hawaii"
	IA = "Iowa"
	ID = "Idaho"
	IL = "Illinois"
	IN = "Indiana"
	KS = "Kansas"
	KY = "Kentucky"
	LA = "Louisiana"
	ME = "Maine"
	MD = "Maryland"
	MA = "Massachusetts"
	MI = "Michigan"
	MN = "Minnesota"
	MS = "Mississippi"
	MO = "Missouri"
	MT = "Montana"
	NE = "Nebraska"
	NV = "Nevada"
	NH = "New Hampshire"
	NJ = "New Jersey"
	NM = "New Mexico"
	NY = "New York"
	NC = "North Carolina"
	ND = "North Dakota"
	OH = "Ohio"
	OK = "Oklahoma"
	OR = "Oregon"
	PA = "Pennsylvania"
	RI = "Rhode Island"
	SC = "South Carolina"
	SD = "South Dakota"
	TN = "Tennessee"
	TX = "Texas"
	UT = "Utah"
	VT = "Vermont"
	VA = "Virginia"
	WA = "Washington"
	WV = "West Virginia"
	WI = "Wisconsin"
	WY = "Wyoming"
}