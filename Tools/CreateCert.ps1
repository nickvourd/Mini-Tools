param(
    [Parameter(Mandatory=$true)]
    [string]$CN
)

# Desktop path
$desktop = [Environment]::GetFolderPath("Desktop")
Write-Host "Desktop path: $desktop"

# Safe filename
$filename = $CN -replace '[^a-zA-Z0-9]', '_'

# Create certificate
$cert = New-SelfSignedCertificate -Subject "CN=$CN" -Type CodeSigningCert -KeyExportPolicy Exportable -KeyAlgorithm RSA -KeyLength 2048 -HashAlgorithm SHA256 -CertStoreLocation "Cert:\CurrentUser\My"

if (!$cert) { Write-Host "Certificate creation failed." -ForegroundColor Red; exit }

Write-Host "Certificate created with Thumbprint:" $cert.Thumbprint

# Export CER
$cerPath = "$desktop\$filename.cer"
Export-Certificate -Cert $cert -FilePath $cerPath

# Export PFX
$password = ConvertTo-SecureString "password" -AsPlainText -Force
$pfxPath = "$desktop\$filename.pfx"
Export-PfxCertificate -Cert $cert -FilePath $pfxPath -Password $password

Write-Host "CER saved to:" $cerPath
Write-Host "PFX saved to:" $pfxPath
