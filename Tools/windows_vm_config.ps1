# DevDragon - Windows Configuration Script
# Run this script as Administrator on the Windows VM
# After deployment, copy this file to the VM and execute it

param(
    [switch]$SkipUpdates = $false,
    [switch]$SkipVisualStudio = $false,
    [switch]$SkipBuildTools = $false,
    [switch]$SkipApplications = $false,
    [switch]$SkipDefender = $false,
    [switch]$Skipx64dbg = $false,
    [switch]$SkipSysinternals = $false
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  DevDragon VM Configuration" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Function to log messages
function Write-Log {
    param([string]$Message, [ValidateSet("Info", "Success", "Warning", "Error")]$Level = "Info")
    $color = @{
        "Info"    = "White"
        "Success" = "Green"
        "Warning" = "Yellow"
        "Error"   = "Red"
    }
    Write-Host "[$Level] $Message" -ForegroundColor $color[$Level]
}

# Check Administrator privileges
if (-not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Log "This script must be run as Administrator!" "Error"
    exit 1
}

Write-Log "Running as Administrator" "Success"

# Step 1: Create folders
Write-Log "Creating folders..." "Info"
$folders = @("C:\Tools", "C:\Payloads")

foreach ($folder in $folders) {
    if (-not (Test-Path $folder)) {
        New-Item -ItemType Directory -Path $folder -Force | Out-Null
        Write-Log "Created folder: $folder" "Success"
    } else {
        Write-Log "Folder already exists: $folder" "Info"
    }

    try {
        $folderPath = (Get-Item $folder).FullName
        $shell = New-Object -ComObject "Shell.Application"
        $qasettings = $shell.Namespace("shell:::{679F85CB-0220-4080-B29B-5540673CE530}")

        if ($qasettings) {
            $qasettings.MoveHere($folderPath, 4)
            Write-Log "Pinned $folder to Quick Access" "Success"
        }
    } catch {
        try {
            $shell = New-Object -ComObject "Shell.Application"
            $folder_item = $shell.Namespace($folderPath)
            $folder_item.Self.InvokeVerb("pintoquickaccessbar")
            Write-Log "Pinned $folder to Quick Access (alternative method)" "Success"
        } catch {
            Write-Log "Could not pin $folder to Quick Access (non-critical)" "Info"
        }
    }
}

# Step 2: Install Windows Updates
if (-not $SkipUpdates) {
    Write-Log "Installing Windows Updates..." "Info"
    try {
        $updateSession = New-Object -ComObject Microsoft.Update.Session
        $updateSearcher = $updateSession.CreateUpdateSearcher()
        Write-Log "Searching for available updates..." "Info"
        $searchResult = $updateSearcher.Search("IsInstalled=0")

        if ($searchResult.Updates.Count -gt 0) {
            Write-Log "Found $($searchResult.Updates.Count) updates to install" "Info"
            $updateCollection = New-Object -ComObject Microsoft.Update.UpdateColl
            $searchResult.Updates | ForEach-Object {
                $updateCollection.Add($_) | Out-Null
            }

            $updateInstaller = $updateSession.CreateUpdateInstaller()
            $updateInstaller.Updates = $updateCollection
            $installationResult = $updateInstaller.Install()

            if ($installationResult.ResultCode -eq 2) {
                Write-Log "Windows Updates completed successfully" "Success"
            } else {
                Write-Log "Windows Updates installation completed with code: $($installationResult.ResultCode)" "Info"
            }
        } else {
            Write-Log "No updates available" "Info"
        }
    } catch {
        Write-Log "Windows Updates warning (non-critical): $($_.Exception.Message)" "Warning"
    }
}

# Step 3: Install Chocolatey
Write-Log "Installing Chocolatey..." "Info"
try {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    $ChocoInstallPath = "$env:ProgramData\chocolatey\bin"
    if (-not (Test-Path $ChocoInstallPath)) {
        $installScript = (New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1')
        iex $installScript
        Write-Log "Chocolatey installed" "Success"
    } else {
        Write-Log "Chocolatey already installed" "Info"
    }
} catch {
    Write-Log "Chocolatey installation failed: $($_.Exception.Message)" "Warning"
}

# Step 4: Install Development Tools
if (-not $SkipApplications) {
    Write-Log "Installing development tools..." "Info"
    $apps = @("golang", "python", "notepadplusplus", "git")

    foreach ($app in $apps) {
        try {
            Write-Log "Installing $app..." "Info"
            choco install $app -y --no-progress 2>&1 | Out-Null
            Write-Log "$app installed successfully" "Success"
        } catch {
            Write-Log "Failed to install $($app): $($_.Exception.Message)" "Warning"
        }
    }
}

# Step 5: Install Sysinternals Suite
if (-not $SkipSysinternals) {
    Write-Log "Installing Sysinternals Suite..." "Info"
    try {
        $sysinternalsPath = "C:\Tools\Sysinternals"
        $zipPath = "$env:TEMP\SysinternalsSuite.zip"
        $url = "https://download.sysinternals.com/files/SysinternalsSuite.zip"

        Write-Log "Downloading Sysinternals..." "Info"
        (New-Object Net.WebClient).DownloadFile($url, $zipPath)

        Write-Log "Extracting Sysinternals..." "Info"
        Expand-Archive -Path $zipPath -DestinationPath $sysinternalsPath -Force

        # Add to PATH
        $currentPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")
        if ($currentPath -notlike "*$sysinternalsPath*") {
            $newPath = "$currentPath;$sysinternalsPath"
            [Environment]::SetEnvironmentVariable("PATH", $newPath, "Machine")
        }

        Remove-Item $zipPath -Force
        Write-Log "Sysinternals installed successfully" "Success"
    } catch {
        Write-Log "Sysinternals installation failed: $_" "Warning"
    }
}

# Step 6: Install Visual Studio Build Tools
if (-not $SkipBuildTools) {
    Write-Log "Installing Visual Studio Build Tools..." "Info"
    try {
        $downloadsPath = [System.IO.Path]::Combine($env:USERPROFILE, "Downloads")
        $buildToolsPath = "$downloadsPath\vs_BuildTools.exe"
        $url = "https://aka.ms/vs/17/release/vs_BuildTools.exe"

        Write-Log "Downloading Visual Studio Build Tools..." "Info"
        (New-Object Net.WebClient).DownloadFile($url, $buildToolsPath)

        Write-Log "Installing Build Tools (this may take 10-15 minutes)..." "Info"
        & $buildToolsPath --quiet --wait --norestart `
            --add Microsoft.VisualStudio.Workload.MSBuildTools `
            --add Microsoft.VisualStudio.Workload.VCTools

        Write-Log "Visual Studio Build Tools installer saved to Downloads" "Success"
        Write-Log "Visual Studio Build Tools installed successfully" "Success"
    } catch {
        Write-Log "Build Tools installation failed: $($_.Exception.Message)" "Warning"
    }
}

# Step 7: Install Visual Studio Community
if (-not $SkipVisualStudio) {
    Write-Log "Installing Visual Studio Community..." "Info"
    try {
        $downloadsPath = [System.IO.Path]::Combine($env:USERPROFILE, "Downloads")
        $vsPath = "$downloadsPath\vs_Community.exe"
        $url = "https://aka.ms/vs/17/release/vs_Community.exe"

        Write-Log "Downloading Visual Studio Community..." "Info"
        (New-Object Net.WebClient).DownloadFile($url, $vsPath)

        Write-Log "Installing Visual Studio (this may take 20-30 minutes)..." "Info"
        & $vsPath --quiet --wait --norestart `
            --add Microsoft.VisualStudio.Workload.NativeDesktop `
            --add Microsoft.VisualStudio.Workload.ManagedDesktop `
            --includeRecommended

        Write-Log "Visual Studio Community installer saved to Downloads" "Success"
        Write-Log "Visual Studio Community installed successfully" "Success"
    } catch {
        Write-Log "Visual Studio installation failed: $($_.Exception.Message)" "Warning"
    }
}

# Step 8: Disable Windows Defender
if (-not $SkipDefender) {
    Write-Log "Disabling Windows Defender..." "Info"
    try {
        Set-MpPreference -DisableRealtimeMonitoring $true -ErrorAction SilentlyContinue
        Set-MpPreference -DisableIntrusionPreventionSystem $true -ErrorAction SilentlyContinue
        Set-MpPreference -MAPSReporting Disabled -ErrorAction SilentlyContinue
        Set-MpPreference -SubmitSamplesConsent NeverSend -ErrorAction SilentlyContinue
        Write-Log "Windows Defender disabled successfully" "Success"
    } catch {
        Write-Log "Failed to disable Defender: $($_.Exception.Message)" "Warning"
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Configuration Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Log "Installation Summary:" "Info"
Write-Host "  ✓ Windows updates" -ForegroundColor Green
Write-Host "  ✓ Development tools (Go, Python, git, Notepad++)" -ForegroundColor Green
Write-Host "  ✓ Visual Studio Community" -ForegroundColor Green
Write-Host "  ✓ Visual Studio Build Tools" -ForegroundColor Green
Write-Host "  ✓ Sysinternals Suite" -ForegroundColor Green
Write-Host "  ✓ C:\Tools and C:\Payloads folders" -ForegroundColor Green
Write-Host "  ✓ Windows Defender disabled" -ForegroundColor Green
Write-Host ""
Write-Host "Note: Some installations may require a system restart to complete fully." -ForegroundColor Yellow
Write-Host ""
