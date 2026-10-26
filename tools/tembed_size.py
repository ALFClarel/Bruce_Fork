#!/usr/bin/env python3
"""Measure actual artifacts. No size estimates, no third-party Python dependencies."""
import argparse, json, struct, subprocess
from pathlib import Path

def measure(directory):
    directory = Path(directory)
    raw = (directory / 'firmware.elf').read_bytes()
    if raw[:6] != b'\x7fELF\x01\x01':
        raise ValueError('Expected little-endian ELF32')
    header = struct.unpack_from('<16sHHIIIIIHHHHHH', raw)
    sections = [struct.unpack_from('<IIIIIIIIII', raw, header[6] + i * header[11]) for i in range(header[12])]
    strings = sections[header[13]]
    names = raw[strings[4]:strings[4] + strings[5]]
    dram = iram = 0
    selected = {}
    for name, kind, flags, address, offset, size, *_ in sections:
        if not flags & 2:
            continue
        label = names[name:names.index(b'\0', name)].decode()
        if 0x3FC80000 <= address < 0x3FE00000:
            dram += size; selected[label] = size
        elif 0x40370000 <= address < 0x40400000:
            iram += size
    return {'firmware_bytes': (directory / 'firmware.bin').stat().st_size,
            'static_dram_bytes': dram, 'iram_bytes': iram, 'dram_sections': selected}

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('build_dir')
    p.add_argument('--baseline')
    p.add_argument('--require-smaller', action='store_true')
    p.add_argument('--out', default='size-report.json')
    a = p.parse_args()
    result = {'target': measure(a.build_dir)}
    if a.baseline:
        result['baseline'] = measure(a.baseline)
        result['delta'] = {key: result['target'][key] - result['baseline'][key]
                           for key in ['firmware_bytes', 'static_dram_bytes', 'iram_bytes']}
    Path(a.out).write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))
    if a.require_smaller:
        if not a.baseline:
            p.error('--require-smaller requires --baseline')
        if result['delta']['firmware_bytes'] >= 0 or result['delta']['static_dram_bytes'] > 0:
            raise SystemExit('Size gate failed: firmware must shrink and static DRAM must not increase')
