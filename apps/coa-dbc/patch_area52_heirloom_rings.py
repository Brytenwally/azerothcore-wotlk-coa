import argparse
import json
import struct
from pathlib import Path


PROFILE = Path(__file__).with_name('area52_heirloom_rings.json')


def definitions():
    return json.loads(PROFILE.read_text(encoding='utf-8'))['profiles']


def records():
    return {entry['distribution']: [entry['distribution'],
            *[value & 0xffffffff for value in entry['types']], *entry['modifiers'], entry['max_level']]
            for entry in definitions()}


def patch(data):
    if len(data) < 20:
        raise ValueError('Truncated DBC header')
    magic, count, fields, size, strings = struct.unpack_from('<4s4I', data)
    if magic != b'WDBC' or fields != 22 or size != 88 or len(data) != 20 + count * size + strings:
        raise ValueError('Expected complete ScalingStatDistribution.dbc')
    wanted = records()
    seen = set()
    for index in range(count):
        row = list(struct.unpack_from('<22I', data, 20 + index * size))
        if row[0] in seen:
            raise ValueError('Duplicate distribution ID')
        seen.add(row[0])
        if row[0] in wanted:
            if row != wanted[row[0]]:
                raise ValueError(f'Scaling distribution collision: {row[0]}')
            del wanted[row[0]]
    body_end = 20 + count * size
    additions = b''.join(struct.pack('<22I', *row) for row in wanted.values())
    header = struct.pack('<4s4I', magic, count + len(wanted), fields, size, strings)
    return header + data[20:body_end] + additions + data[body_end:]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.source.resolve() == args.output.resolve():
        parser.error('Write to a separate candidate file')
    result = patch(args.source.read_bytes())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(result)
    print(f'Wrote {args.output}; four ring profiles; not installed')


if __name__ == '__main__':
    main()
