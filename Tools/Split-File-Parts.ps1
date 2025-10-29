# Example: split myfile.iso into 50MB chunks
$in  = "C:\Users\nikos\Desktop\myfile.iso"
$out = "C:\Users\nikos\Desktop\output"
$chunkSize = 50MB

$fs = [System.IO.File]::OpenRead($in)
$buffer = New-Object byte[] $chunkSize
$index = 0
while (($read = $fs.Read($buffer, 0, $buffer.Length)) -gt 0) {
    $outFile = "{0}\part_{1:D3}.bin" -f $out, $index
    [System.IO.File]::WriteAllBytes($outFile, $buffer[0..($read-1)])
    $index++
}
$fs.Close()
