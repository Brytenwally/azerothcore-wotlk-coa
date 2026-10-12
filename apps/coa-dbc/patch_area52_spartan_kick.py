import argparse
from pathlib import Path
import struct


def patch(raw):
    magic, count, fields, size, pool_size = struct.unpack_from('<4s4I', raw)
    if magic != b'WDBC' or fields < 226 or size != fields * 4 or len(raw) != 20 + count * size + pool_size:
        raise ValueError('Unsupported Spell.dbc layout')
    rows = {struct.unpack_from('<I', raw, 20 + i * size)[0]: bytearray(raw[20 + i * size:20 + (i + 1) * size]) for i in range(count)}
    pool = bytearray(raw[20 + count * size:])
    if 1766 not in rows or 84445 not in rows:
        raise ValueError('Missing Kick or Spartan Kick record')

    def description(row):
        pointer = struct.unpack_from('<I', row, 170 * 4)[0]
        return pool[pointer:pool.index(0, pointer)].decode('utf-8')

    def set_description(row, text):
        value = text.encode('utf-8') + b'\0'
        pointer = pool.find(value)
        if pointer < 0:
            pointer = len(pool)
            pool.extend(value)
        struct.pack_into('<I', row, 170 * 4, pointer)

    text = description(rows[84445])
    new = 'Your Kick ability now knocks enemies back. While equipped, Kick has its own 20-second cooldown and does not share a cooldown with other interrupts. Knockback requires Battle, Defensive or Phalanx Stance.'
    start = text.find('Your Kick ability')
    end = text.find('\n\n', start)
    if start < 0 or end < 0:
        raise ValueError('Unexpected Spartan Kick tooltip')
    set_description(rows[84445], text[:start] + new + text[end:])
    replacement = bytearray(rows[1766])
    for field, value in {0: 9901766, 1: 0, 24: 0, 29: 20000, 30: 0}.items():
        struct.pack_into('<I', replacement, field * 4, value)
    set_description(replacement, 'A quick kick that interrupts spellcasting and prevents any spell in that school from being cast for $d. |cffFFFFFFThis spell cannot miss.|r\n\n@ext:Spartan Kick: independent 20-second cooldown. Requires Spartan Kick to remain equipped. Knockback requires Battle, Defensive or Phalanx Stance.\n\n|cffffff00VERIFIED|r:ext@')
    rows[9901766] = replacement
    return struct.pack('<4s4I', magic, len(rows), fields, size, len(pool)) + b''.join(rows.values()) + pool


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.source.resolve() == args.output.resolve():
        parser.error('Use a separate output file')
    args.output.write_bytes(patch(args.source.read_bytes()))
