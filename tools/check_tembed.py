#!/usr/bin/env python3
"""Host checks for hardware boundaries and the mandatory unchanged native mic menu."""
import json, subprocess, re
from pathlib import Path
root = Path(__file__).resolve().parents[1]
flags = ['-DBRUCE_TEMBED_BASE', '-include', 'include/tembed_capabilities.h']
subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-Iinclude',
                'tests/tembed/policy_test.cpp', '-o', '/tmp/bruce-tembed-policy'], cwd=root, check=True)
subprocess.run(['/tmp/bruce-tembed-policy'], check=True)
count = 0
for path in sorted((root / 'src').rglob('*.cpp')):
    content = path.read_text(encoding='utf-8-sig')
    if content.startswith('#if !defined(BRUCE_'):
        output = subprocess.check_output(['g++', '-E', '-P', *flags, str(path)], cwd=root, text=True)
        assert not output.strip(), f'Disabled source still emits code: {path}'
        count += 1
before = subprocess.check_output(['git', 'show', 'a59213f:src/core/menu_items/OthersMenu.cpp'], cwd=root, text=True)
after = (root / 'src/core/menu_items/OthersMenu.cpp').read_text()
pattern = r'void OthersMenu::micMenu\(\) \{.*?\n\}'
assert re.search(pattern, before, re.S).group() == re.search(pattern, after, re.S).group(), 'micMenu changed'
config = dict((section, dict(values)) for section, values in json.loads(subprocess.check_output(
    ['python', '-m', 'platformio', 'project', 'config', '--json-output'], cwd=root)))
env = config['env:lilygo-t-embed-cc1101']
assert config['platformio']['default_envs'] == ['lilygo-t-embed-cc1101']
assert all('-O2' not in flag for flag in env['build_flags'])
assert not any('LITE_VERSION' in flag or 'DISABLE_INTERPRETER' in flag for flag in env['build_flags'])
for name in ['RF24', 'Si4713', 'TinyGPS', 'OneWire', 'RadioLib', 'Chameleon', 'Amiibolink', 'MFRC522']:
    assert not any(name in dep for dep in env['lib_deps']), f'Unsupported dependency: {name}'
print(f'PASS: policy boundaries, {count} compiled-out sources, native micMenu, target configuration')
