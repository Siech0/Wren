$p = 'd:\dev\oss\c++\renderer\projects\libs\rhi\api\include\wren\rhi\api\features.hpp'
$c = [IO.File]::ReadAllText($p, [Text.Encoding]::UTF8)
$c = $c.Replace([char]0x2019, [char]0x27).Replace([char]0xFEFF, '')
[IO.File]::WriteAllText($p, $c, (New-Object Text.UTF8Encoding($false)))
Write-Host 'done'
