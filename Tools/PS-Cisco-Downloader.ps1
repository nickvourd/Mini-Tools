$global:version = "1.0.0"

function Get-AbsolutePath {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory = $true)]
        [string]$FileName
    )

    if (-not (Test-Path $FileName)) {
        Write-Host "File not found: $FileName"
        return
    }

    $absolutePath = (Get-Item $FileName).FullName
    return $absolutePath
}


$ascii = @"

_________ .___  __________________  ________    _________ .____    .______________ __________________
\_   ___ \|   |/   _____/\_   ___ \ \_____  \   \_   ___ \|    |   |   \_   _____/ \      \__    ___/
/    \  \/|   |\_____  \ /    \  \/  /   |   \  /    \  \/|    |   |   ||    __)_  /   |   \|    |   
\     \___|   |/        \\     \____/    |    \ \     \___|    |___|   ||        \/    |    \    |   
 \______  /___/_______  / \______  /\_______  /  \______  /_______ \___/_______  /\____|__  /____|   
        \/            \/         \/         \/          \/        \/           \/         \/         

                                ~ Created with <3 by @nickvourd
                                ~ Version: $global:version

"@

Write-Host $ascii`n

Write-Host "[+] Download Cisco Anyconnect Client for Windows (MSI)`n"
Invoke-WebRequest -Uri https://olemiss.edu/helpdesk/vpn/_files/anyconnect-win-4.10.00093-core-vpn-predeploy-k9.msi -OutFile anyconnect-win-4.10.00093-core-vpn-predeploy-k9.msi

$fileName = "anyconnect-win-4.10.00093-core-vpn-predeploy-k9.msi"
$absolutePath = Get-AbsolutePath -FileName $fileName
Write-Host "[+] Your file has been saved here: $absolutePath`n"
