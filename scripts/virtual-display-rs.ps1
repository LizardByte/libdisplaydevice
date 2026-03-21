param (
    [Parameter(Mandatory = $true)]
    [int]$DisplayCount
)

# each display gets a distinct resolution so Windows can distinguish them
$resolutions = @(
    @{ width = 1920; height = 1080; refresh_rates = @(60) },
    @{ width = 1280; height = 720;  refresh_rates = @(60) },
    @{ width = 1600; height = 900;  refresh_rates = @(60) }
)

$monitors = @()
for ($i = 0; $i -lt $DisplayCount; $i++) {
    $res = $resolutions[$i % $resolutions.Count]
    $monitors += [ordered]@{
        id      = [uint32]$i
        name    = "VDD$i"
        enabled = $true
        modes   = @(
            [ordered]@{
                width         = [uint32]$res.width
                height        = [uint32]$res.height
                refresh_rates = @([uint32]$res.refresh_rates[0])
            }
        )
    }
}

Write-Information "Payload: $json" -InformationAction Continue

# retry connecting to the named pipe until the UMDF driver is fully loaded
$connected = $false
$deadline = (Get-Date).AddSeconds(30)
while (-not $connected -and (Get-Date) -lt $deadline) {
    try {
        Write-Information "Connecting to named pipe..." -InformationAction Continue
        $pipe = New-Object System.IO.Pipes.NamedPipeClientStream(
            ".", "virtualdisplaydriver", [System.IO.Pipes.PipeDirection]::InOut)
        $pipe.Connect(2000)
        $pipe.ReadMode = [System.IO.Pipes.PipeTransmissionMode]::Message
        $connected = $true
        Write-Information "Connected" -InformationAction Continue
    } catch {
        Write-Information "Pipe not ready, retrying... ($($_.Exception.Message))" -InformationAction Continue
        Start-Sleep -Seconds 2
    }
}

if (-not $connected) {
    Write-Error "Timed out waiting for the virtual display driver named pipe."
    exit 1
}

$bytes = [System.Text.Encoding]::UTF8.GetBytes($json)
$pipe.Write($bytes, 0, $bytes.Length)
$pipe.Flush()
$pipe.Dispose()

Write-Information "Sent monitor config for $DisplayCount display(s)" -InformationAction Continue
