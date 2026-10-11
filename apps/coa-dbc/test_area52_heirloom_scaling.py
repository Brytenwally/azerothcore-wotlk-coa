import json
import statistics
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import patch_area52_heirloom_scaling as tuning


class HeirloomScalingTest(unittest.TestCase):
    def setUp(self):
        self.profile = json.loads(tuning.PROFILE.read_text(encoding='utf-8'))
        self.rows = [[100 + level, level] + [10] * 22 for level in range(1, 68)]
        self.rows += [self.profile['levels'][str(level)]['before'] for level in range(68, 81)]

    def binary(self, rows):
        return struct.pack('<4s4I', b'WDBC', len(rows), 24, 96, 1) + b''.join(
            struct.pack('<24I', *row) for row in rows) + b'\0'

    def test_rebalances_endpoint_and_preserves_other_fields(self):
        data = self.binary(self.rows)
        result = tuning.patch(data)
        self.assertEqual(result[:20 + 67 * 96], data[:20 + 67 * 96])
        for level in range(68, 81):
            row = struct.unpack_from('<24I', result, 20 + (level - 1) * 96)
            self.assertEqual(list(row), self.profile['levels'][str(level)]['after'])
            for column in set(range(24)) - tuning.MUTABLE_COLUMNS:
                self.assertEqual(row[column], self.rows[level - 1][column])
        self.assertEqual(tuning.patch(result), result)

    def test_budget_progression_and_endpoint(self):
        previous = self.profile['level67']
        for level in range(68, 81):
            row = self.profile['levels'][str(level)]['after']
            for column in tuning.MUTABLE_COLUMNS:
                self.assertGreaterEqual(row[column], previous[column])
            previous = row
        self.assertEqual(previous[2], 203)
        self.assertEqual(previous[16], 283)
        self.assertEqual(previous[2] * 1904 // 10000, 38)
        self.assertEqual(previous[2] * 4080 // 10000, 82)

    def test_rejects_unknown_missing_duplicate_or_truncated_data(self):
        bad = [row[:] for row in self.rows]
        bad[-1][2] += 1
        for rows in [bad, self.rows[:-1], self.rows + [self.rows[-1]]]:
            with self.assertRaises(ValueError):
                tuning.patch(self.binary(rows))
        with self.assertRaises(ValueError):
            tuning.patch(self.binary(self.rows)[:-1])

    def test_rejects_armor_changes(self):
        self.profile['levels']['80']['after'][9] += 1
        with self.assertRaises(ValueError):
            tuning.patch(self.binary(self.rows), self.profile)

    def test_endpoint_against_dungeon_reference_budgets(self):
        reference = json.loads(tuning.PROFILE.with_name('area52_heirloom_reference.json').read_text(encoding='utf-8'))
        weights = {7: 2 / 3, 38: .5, 39: .5, 45: 6 / 7, 43: 2.5}
        for column, sample in reference['columns'].items():
            value = self.profile['levels']['80']['after'][int(column)]
            if 'reference' in sample:
                ratio = value / sample['reference']
            else:
                ratios = []
                for item in sample['samples']:
                    points = sum((value * modifier // 10000) * weights.get(kind, 1)
                                 for kind, modifier in zip(item['types'], item['weights'])
                                 if kind not in (0, 4294967295))
                    ratios.append(points / item['peer_points'])
                ratio = statistics.median(ratios)
            self.assertGreaterEqual(ratio, .89, column)
            self.assertLessEqual(ratio, .91, column)


if __name__ == '__main__':
    unittest.main()
