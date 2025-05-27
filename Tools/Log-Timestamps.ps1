# Log session start time
$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
Add-Content -Path "$HOME\PowerShellSessionLog.txt" -Value "Session started at: $timestamp"
 
# Optional: Log each command execution (advanced)
Register-EngineEvent -SourceIdentifier PowerShell.Exiting -Action {
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Add-Content -Path "$HOME\PowerShellSessionLog.txt" -Value "Session ended at: $timestamp"
}
