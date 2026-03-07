param(
[Parameter(Mandatory=$true)]
[string]$CN
)

# Desktop path

$desktop = [Environment]::GetFolderPath("Desktop")

# Create certificate

$cert = New-SelfSignedCertificate `-Subject "CN=$CN"`
-Type CodeSigningCert `-KeyExportPolicy Exportable`
-KeyAlgorithm RSA `-KeyLength 2048`
-HashAlgorithm sha256 `
-CertStoreLocation "Cert:\CurrentUser\My"

# Export certificate (.cer)

Export-Certificate `-Cert $cert`
-FilePath "$desktop$($CN -replace ' ','_').cer" | Out-Null

# Export private key (.pfx)

$password = ConvertTo-SecureString -String "password" -Force -AsPlainText

Export-PfxCertificate `-Cert $cert`
-FilePath "$desktop$($CN -replace ' ','_').pfx" `
-Password $password | Out-Null

Write-Host "Certificate created successfully!" -ForegroundColor Green
Write-Host "Files saved to: $desktop"
