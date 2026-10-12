import os
from pathlib import Path
import re
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.environ.get('XPHOT_DISPOSABLE_MYSQL') == '1', 'Disposable MySQL fixture not selected')
class HotspotDatabaseTests(unittest.TestCase):
    def sql(self, statement):
        result = subprocess.run(['mysql', '--host=127.0.0.1', '--user=root', '--batch', '--skip-column-names',
                               'hotspot_test'], input=statement, text=True, capture_output=True,
                              check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout.strip()

    def test_migration_preserves_edits_and_runtime_state_on_reapply(self):
        self.assertEqual(self.sql('SELECT COUNT(*) FROM information_schema.tables WHERE table_schema=DATABASE();'), '0')
        migration = (ROOT / 'data/sql/updates/pending_db_world/rev_20261011_00_hotspots.sql').read_text()
        self.sql(migration)
        self.assertEqual(self.sql('SELECT COUNT(*), SUM(JSON_VALID(payload)) FROM xp_hotspot_location;'), '3129\t3129')
        self.assertEqual(self.sql("SELECT COUNT(*) FROM xp_hotspot_location WHERE "
                                  "JSON_UNQUOTE(JSON_EXTRACT(payload,'$.kind'))='circle';"), '2715')
        self.sql("UPDATE xp_hotspot_location SET payload=JSON_SET(payload,'$.name','GM edited',"
                 "'$.ends',2000000000) WHERE id=1;")
        self.sql('UPDATE xp_hotspot_setting SET value=0 WHERE id=1;')
        self.sql(migration)
        self.assertEqual(self.sql("SELECT JSON_UNQUOTE(JSON_EXTRACT(payload,'$.name')),"
                                  "JSON_EXTRACT(payload,'$.ends') FROM xp_hotspot_location WHERE id=1;"),
                         'GM edited\t2000000000')
        self.assertEqual(self.sql('SELECT value FROM xp_hotspot_setting WHERE id=1;'), '0')
        self.assertEqual(self.sql('SELECT COUNT(*) FROM xp_hotspot_location;'), '3129')
        refinement = (ROOT / 'data/sql/updates/pending_db_world/rev_20261011_02_hotspot_boundaries.sql').read_text()
        self.sql("UPDATE xp_hotspot_location SET payload=JSON_SET(payload,'$.name','Custom GM boundary',"
                 "'$.revision',2,'$.radius',75) WHERE id=100052;")
        self.sql("UPDATE xp_hotspot_location SET payload=JSON_SET(payload,'$.ends',2000000000) WHERE id=100126;")
        self.sql(refinement)
        count = self.sql('SELECT COUNT(*) FROM xp_hotspot_location;')
        self.assertGreater(int(count), 3129)
        self.assertEqual(self.sql("SELECT JSON_UNQUOTE(JSON_EXTRACT(payload,'$.name')),"
                                 "JSON_EXTRACT(payload,'$.radius') FROM xp_hotspot_location WHERE id=100052;"),
                         'Custom GM boundary\t75')
        self.assertEqual(self.sql("SELECT JSON_EXTRACT(payload,'$.retired') "
                                 "FROM xp_hotspot_location WHERE id=102536;"), 'true')
        self.assertEqual(self.sql("SELECT JSON_EXTRACT(payload,'$.ends') "
                                 "FROM xp_hotspot_location WHERE id=102536;"), '0')
        self.sql(refinement)
        self.assertEqual(self.sql('SELECT COUNT(*) FROM xp_hotspot_location;'), count)
        self.assertEqual(self.sql("SELECT JSON_UNQUOTE(JSON_EXTRACT(payload,'$.name')) "
                                 "FROM xp_hotspot_location WHERE id=100052;"), 'Custom GM boundary')

        values = [1, 5, 200, 7200, 1800, 2, 2, 1, 1, 2, 2, 3, 2]
        assignments = ','.join(f'({index + 1},{value})' for index, value in enumerate(values))
        self.sql('START TRANSACTION; REPLACE INTO xp_hotspot_setting (id,value) VALUES ' + assignments +
                 "; UPDATE xp_hotspot_location SET payload=JSON_SET(payload,'$.xp',200,'$.duration',7200,"
                 "'$.cooldown',1800) WHERE id=1; COMMIT;")
        self.assertEqual(self.sql('SELECT value FROM xp_hotspot_setting ORDER BY id;'),
                         '\n'.join(str(value) for value in values))
        self.assertEqual(self.sql("SELECT JSON_EXTRACT(payload,'$.xp'),JSON_EXTRACT(payload,'$.duration'),"
                                 "JSON_EXTRACT(payload,'$.cooldown') FROM xp_hotspot_location WHERE id=1;"),
                         '200\t7200\t1800')
        self.sql("START TRANSACTION; REPLACE INTO xp_hotspot_setting (id,value) VALUES (2,0),(13,3); "
                 "UPDATE xp_hotspot_location SET payload=JSON_SET(payload,'$.xp',999) WHERE id=1; ROLLBACK;")
        self.assertEqual(self.sql('SELECT value FROM xp_hotspot_setting WHERE id IN (2,13) ORDER BY id;'), '5\n2')
        self.assertEqual(self.sql("SELECT JSON_EXTRACT(payload,'$.xp') FROM xp_hotspot_location WHERE id=1;"), '200')
        self.sql(migration)
        self.assertEqual(self.sql('SELECT value FROM xp_hotspot_setting ORDER BY id;'),
                         '\n'.join(str(value) for value in values))
        source = (ROOT / 'src/server/database/Database/Implementation/WorldDatabase.cpp').read_text()
        delete = re.search(r'PrepareStatement\(WORLD_DEL_XP_HOTSPOT, "([^"]+)"', source).group(1)
        remove = delete.replace('?', '1')
        self.sql('START TRANSACTION; ' + remove + '; '
                 'REPLACE INTO xp_hotspot_setting (id,value) VALUES (1000,1000001); ROLLBACK;')
        self.assertEqual(self.sql('SELECT COUNT(*) FROM xp_hotspot_location WHERE id=1;'), '1')
        self.assertEqual(self.sql('SELECT COUNT(*) FROM xp_hotspot_setting WHERE id=1000;'), '0')
        self.sql('START TRANSACTION; ' + remove + '; '
                 'REPLACE INTO xp_hotspot_setting (id,value) VALUES (1000,1000001); COMMIT;')
        self.assertEqual(self.sql('SELECT COUNT(*) FROM xp_hotspot_location WHERE id=1;'), '0')
        self.assertEqual(self.sql('SELECT value FROM xp_hotspot_setting WHERE id=1000;'), '1000001')
        self.assertEqual(self.sql('SELECT COUNT(*) FROM xp_hotspot_location;'), str(int(count) - 1))


if __name__ == '__main__':
    unittest.main()
