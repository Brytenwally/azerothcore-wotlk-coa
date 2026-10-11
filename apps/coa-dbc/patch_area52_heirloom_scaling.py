import argparse
import json
import struct
from pathlib import Path


PROFILE = Path(__file__).with_name('area52_heirloom_scaling.json')
MUTABLE_COLUMNS = frozenset((2, 3, 4, 5, 10, 11, 12, 13, 14, 15, 16, 17, 18))


def patch(data, profile=None):
    profile = profile or json.loads(PROFILE.read_text(encoding='utf-8'))
    if len(data) < 20:
        raise ValueError('Truncated DBC header')
    magic, count, fields, size, strings = struct.unpack_from('<4s4I', data)
    if magic != b'WDBC' or fields != 24 or size != 96 or len(data) != 20 + count * size + strings:
        raise ValueError('Expected a complete 24-field ScalingStatValues DBC')
    result = bytearray(data)
    seen = set()
    for index in range(count):
        offset = 20 + index * size
        row = list(struct.unpack_from('<24I', data, offset))
        level = str(row[1])
        if level not in profile['levels']:
            continue
        if level in seen:
            raise ValueError(f'Duplicate scaling level {level}')
        seen.add(level)
        change = profile['levels'][level]
        if row != change['before'] and row != change['after']:
            raise ValueError(f'Unexpected source definition at level {level}')
        before, after = change['before'], change['after']
        if len(before) != 24 or len(after) != 24 or not 68 <= int(level) <= 80:
            raise ValueError('Invalid profile row')
        if before[1] != int(level) or after[1] != int(level):
            raise ValueError('Profile level mismatch')
        if any(before[column] != after[column] for column in range(24) if column not in MUTABLE_COLUMNS):
            raise ValueError('Profile changes a protected column')
        struct.pack_into('<24I', result, offset, *after)
    if seen != {str(level) for level in range(68, 81)}:
        raise ValueError('Missing required levels 68 through 80')
    return bytes(result)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.source.resolve() == args.output.resolve():
        parser.error('Write to a separate candidate file')
    candidate = patch(args.source.read_bytes())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(candidate)
    print(f'Wrote {args.output}; levels 68–80 only; not installed')


if __name__ == '__main__':
    main()
