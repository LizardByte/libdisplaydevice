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
    $monitors += @{
        id      = [uint32]$i
        name    = "VDD$i"
        enabled = $true
        modes   = @(@{
            width         = [uint32]$res.width
            height        = [uint32]$res.height
            refresh_rates = @([uint32]$res.refresh_rates[0])
        })
    }
}

# send {"DriverNotify": [...monitors...]} over the named pipe
$json = [System.Text.Json.JsonSerializer]::Serialize(
    @{ DriverNotify = $monitors },
    [System.Text.Json.JsonSerializerOptions]@{ WriteIndented = $false }
)

Write-Information "Payload: $json" -InformationAction Continue

Write-Information "Connecting to named pipe..." -InformationAction Continue
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream(".", "virtualdisplaydriver", [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(5000)
$pipe.ReadMode = [System.IO.Pipes.PipeTransmissionMode]::Message
Write-Information "Connected" -InformationAction Continue

$bytes = [System.Text.Encoding]::UTF8.GetBytes($json)
$pipe.Write($bytes, 0, $bytes.Length)
$pipe.Flush()
$pipe.Dispose()

Write-Information "Sent monitor config for $DisplayCount display(s)" -InformationAction Continue
