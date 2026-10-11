import argparse
from pathlib import Path
import struct


DESCRIPTION = (
    '@req:6343@\nYour Sword and Board procs now reset Thunder Clap\'s cooldown instead of Shield Slam\'s. '
    'Your next Thunder Clap costs no rage, generates 20% bonus threat, and attempts to dispel 1 beneficial '
    'magic effect from your selected target if Thunder Clap hits it. Other targets are not dispelled.'
    '\n\n@ext:|cffff0000NOT VERIFIED|r:ext@'
)


def patch(source, destination):
    data = bytearray(Path(source).read_bytes())
    magic, count, fields, size, strings = struct.unpack_from('<4s4I', data)
    if magic != b'WDBC' or fields != 234 or size != fields * 4:
        raise ValueError('Expected the Ascension WDBC Spell layout')
    if len(data) != 20 + count * size + strings:
        raise ValueError('Invalid Spell.dbc length')
    row = next((20 + index * size for index in range(count)
                if struct.unpack_from('<I', data, 20 + index * size)[0] == 979777), None)
    if row is None:
        raise ValueError('Sword and Thunder 979777 is missing')
    encoded = DESCRIPTION.encode('utf-8') + b'\0'
    struct.pack_into('<I', data, row + 170 * 4, strings)
    struct.pack_into('<I', data, 16, strings + len(encoded))
    data.extend(encoded)
    Path(destination).write_bytes(data)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    patch(args.input, args.output)
