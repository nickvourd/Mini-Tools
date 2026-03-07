param(
    [Parameter(Mandatory=$true)]
    [string]$CN,

    [Parameter(Mandatory=$false)]
    [string]$Password = "test"
)

# Desktop path
$desktop = [Environment]::GetFolderPath("Desktop")
Write-Host "Desktop path: $desktop"

# Safe filename (replace spaces/illegal chars)
$filename = $CN -replace '[^a-zA-Z0-9]', '_'

# Create a self-signed Code Signing certificate
$cert = New-SelfSignedCertificate `
    -Subject "CN=$CN" `
    -Type CodeSigningCert `
    -KeyExportPolicy Exportable `
    -KeyAlgorithm RSA `
    -KeyLength 2048 `
    -HashAlgorithm SHA256 `
    -CertStoreLocation "Cert:\CurrentUser\My"

if (!$cert) {
    Write-Host "Certificate creation failed." -ForegroundColor Red
    exit
}

Write-Host "Certificate created with Thumbprint: $($cert.Thumbprint)"

# Export CER (public certificate)
$cerPath = "$desktop\$filename.cer"
Export-Certificate -Cert $cert -FilePath $cerPath
Write-Host "CER saved to: $cerPath"

# Export PFX (certificate + private key) with user-supplied password
$securePassword = ConvertTo-SecureString $Password -AsPlainText -Force
$pfxPath = "$desktop\$filename.pfx"
Export-PfxCertificate -Cert $cert -FilePath $pfxPath -Password $securePassword
Write-Host "PFX saved to: $pfxPath"

Write-Host "[+] All files created successfully!"
