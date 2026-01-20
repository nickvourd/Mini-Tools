Get-DomainComputer -Ping |
    Where-Object {
        $_.OperatingSystem -notmatch "Windows 10" -and
        $_.OperatingSystem -notmatch "Windows 11" -and
        $_.OperatingSystem -notmatch "Windows Server 2016" -and
        $_.OperatingSystem -notmatch "Windows Server 2019" -and
        $_.OperatingSystem -notmatch "Windows Server 2022" -and
        $_.OperatingSystem -notmatch "Windows Server 2025" -and
        $_.OperatingSystem -notmatch "Mac OS X" -and
        $_.OperatingSystem -notmatch "macOS"
    } |
    Select-Object Name, DistinguishedName, OperatingSystem, OperatingSystemVersion
