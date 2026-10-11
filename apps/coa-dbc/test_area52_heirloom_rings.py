import json
import os
import re
import struct
import subprocess
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import patch_area52_heirloom_rings as rings

ROOT = Path(__file__).resolve().parents[2]
SQL = ROOT / 'data/sql/updates/pending_db_world/rev_20261011_00_area52_heirloom_ring_stats.sql'


def binary(rows):
    return struct.pack('<4s4I', b'WDBC', len(rows), 22, 88, 1) + b''.join(
        struct.pack('<22I', *row) for row in rows) + b'\0'


class HeirloomRingsTest(unittest.TestCase):
    def test_additions_preserve_existing_profiles_and_are_idempotent(self):
        original = binary([[371] + [0] * 20 + [80]])
        candidate = rings.patch(original)
        self.assertEqual(struct.unpack_from('<I', candidate, 4)[0], 5)
        self.assertEqual(candidate[20:108], original[20:108])
        self.assertEqual(candidate[-1:], original[-1:])
        self.assertEqual(rings.patch(candidate), candidate)

    def test_rejects_conflicting_truncated_and_duplicate_records(self):
        row = list(rings.records().values())[0]
        bad = row[:]
        bad[11] += 1
        for data in [binary([bad]), binary([row, row]), binary([row])[:-1], b'']:
            with self.assertRaises(ValueError):
                rings.patch(data)

    def test_sql_and_client_definitions_match_and_xp_is_untouched(self):
        sql = SQL.read_text(encoding='utf-8')
        tuples = re.findall(r'^\(([^)]+)\)[,;]', sql, re.M)
        self.assertEqual(len(tuples), 4)
        for text in tuples:
            values = [int(value.strip()) & 0xffffffff for value in text.split(',')]
            self.assertEqual(values, rings.records()[values[0]])
        expected = [f"UPDATE `item_template` SET `ScalingStatDistribution` = {entry['distribution']}, "
                    f"`ScalingStatValue` = 4 WHERE `entry` = {entry['item']};" for entry in rings.definitions()]
        self.assertEqual([line for line in sql.splitlines() if line.startswith('UPDATE `item_template`')], expected)
        self.assertNotIn('spellid', sql.lower())
        self.assertNotIn('spelltrigger', sql.lower())

    def test_level_80_budget_and_all_level_growth(self):
        data = json.loads(rings.PROFILE.read_text(encoding='utf-8'))
        weights = {7: 2 / 3, 45: 6 / 7}
        for entry in rings.definitions():
            previous = [0] * 10
            for level in range(1, 81):
                budget = data['active_one_hand_budgets'][str(level)]
                values = [budget * modifier // 10000 for modifier in entry['modifiers']]
                self.assertTrue(all(new >= old for old, new in zip(previous, values)))
                previous = values
            actual = {str(kind): value for kind, value in zip(entry['types'], previous) if kind != -1}
            self.assertEqual(actual, entry['level80'])
            points = sum(value * weights.get(int(kind), 1) for kind, value in actual.items())
            self.assertGreaterEqual(points / entry['reference_points'], .89)
            self.assertLessEqual(points / entry['reference_points'], .91)
            self.assertEqual(entry['max_level'], 80)
        self.assertEqual({entry['item'] for entry in rings.definitions()}, {62035, 62036, 62037, 69891})

    @unittest.skipUnless(os.environ.get('AREA52_RING_MYSQL_CONTAINER'), 'Optional MySQL temporary-table fixture')
    def test_mysql_migration_on_temporary_tables(self):
        setup = '''
CREATE TEMPORARY TABLE ring_item_fixture AS
SELECT * FROM acore_world.item_template WHERE entry IN (50255,62035,62036,62037,69891);
CREATE TEMPORARY TABLE item_template LIKE ring_item_fixture;
INSERT INTO item_template SELECT * FROM ring_item_fixture;
CREATE TEMPORARY TABLE ring_distribution_fixture LIKE acore_world.scalingstatdistribution_dbc;
CREATE TEMPORARY TABLE scalingstatdistribution_dbc LIKE ring_distribution_fixture;
CREATE TEMPORARY TABLE version (cache_id INT);
INSERT INTO version VALUES (17);
'''
        checks = '''
SELECT COUNT(*) FROM scalingstatdistribution_dbc WHERE ID BETWEEN 12040 AND 12043 AND Maxlevel=80;
SELECT COUNT(*) FROM item_template WHERE entry IN (62035,62036,62037,69891)
AND ScalingStatDistribution BETWEEN 12040 AND 12043 AND ScalingStatValue=4 AND spellid_1=71354 AND spelltrigger_1=1;
SELECT COUNT(*) FROM item_template t JOIN ring_item_fixture b USING(entry)
WHERE t.entry=50255 AND t.ScalingStatDistribution=b.ScalingStatDistribution AND t.ScalingStatValue=b.ScalingStatValue;
SELECT cache_id FROM version;
'''
        result = subprocess.run(['docker', 'exec', '-i', os.environ['AREA52_RING_MYSQL_CONTAINER'], 'mysql',
                                 '--defaults-extra-file=/run/secrets/mysql-root.cnf', '-BN', 'acore_world'],
                                input=setup + SQL.read_text(encoding='utf-8') * 2 + checks,
                                text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.split(), ['4', '4', '1', '18'])


if __name__ == '__main__':
    unittest.main()
