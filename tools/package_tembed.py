#!/usr/bin/env python3
"""Prepare self-hostable ESP Web Tools assets without deploying or flashing a device."""
import hashlib, json, shutil, subprocess
from pathlib import Path
root = Path(__file__).resolve().parents[1]
build = root / '.pio/build/lilygo-t-embed-cc1101'
output = root / 'dist/tembed'
output.mkdir(parents=True, exist_ok=True)
firmware = root / 'Bruce-lilygo-t-embed-cc1101.bin'
if not firmware.is_file():
    raise SystemExit('Build the target first: merged image is missing')
shutil.copy2(firmware, output / 'firmware-merged.bin')
shutil.copy2(build / 'firmware.bin', output / 'firmware-ota.bin')
revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
manifest = {'name': 'Bruce T-Embed CC1101 base', 'version': revision[:12],
            'builds': [{'chipFamily': 'ESP32-S3', 'parts': [{'path': 'firmware-merged.bin', 'offset': 0}]}]}
(output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
checksums = '\n'.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}'
                       for p in sorted(output.glob('*.bin')))
(output / 'SHA256SUMS').write_text(checksums + '\n')
for name in ['LICENSE', 'THIRD_PARTY.md']:
    shutil.copy2(root / name, output / name)
(output / 'SOURCE.txt').write_text(f'https://github.com/ALFClarel/Bruce_Fork/tree/{revision}\n')
print(output)
