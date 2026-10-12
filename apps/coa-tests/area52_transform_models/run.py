from pathlib import Path
import json
import os
import sqlite3
import subprocess
import re
import uuid

root = Path(__file__).resolve().parents[3]
sql = (root / 'data/sql/updates/pending_db_world/rev_20261012_11_area52_transform_models.sql').read_text(encoding='utf8')
repairs = json.loads((root / 'docs/coa/area52-transform-models/repairs.json').read_text(encoding='utf8'))
assert repairs['173524']['displays'] == [42853]
assert repairs['173525']['displays'] == [151196]
assert repairs['173523']['displays'] == [42854]
entries = sorted(map(int, repairs))
protected, spawned = [entry for entry in entries if entry not in (173523, 173524, 173525)][:2]
schema = '''
CREATE TEMPORARY TABLE creature_template (entry INTEGER PRIMARY KEY, name VARCHAR(100), faction INTEGER, unit_class INTEGER, type INTEGER);
CREATE TEMPORARY TABLE creature_template_model (CreatureID INTEGER, Idx INTEGER, CreatureDisplayID INTEGER, DisplayScale FLOAT, Probability FLOAT, PRIMARY KEY (CreatureID, Idx));
CREATE TEMPORARY TABLE creature (id INTEGER);
'''
fixture = f"INSERT INTO creature_template VALUES ({protected}, 'Preserved', 14, 1, 7);\nINSERT INTO creature_template_model VALUES ({protected},0,12345,2,1);\nINSERT INTO creature VALUES ({spawned});\n"
db = sqlite3.connect(':memory:')
db.executescript(schema + fixture)
assert not db.execute('SELECT 1 FROM creature_template WHERE entry IN (173524,173525)').fetchall()
portable = sql.replace('START TRANSACTION;', 'BEGIN;').replace('DROP TEMPORARY TABLE', 'DROP TABLE')
db.executescript(portable)
snapshot = db.execute('SELECT * FROM creature_template_model ORDER BY CreatureID,Idx').fetchall()
for entry, evidence in repairs.items():
    entry = int(entry)
    actual = [r[0] for r in db.execute('SELECT CreatureDisplayID FROM creature_template_model WHERE CreatureID=? ORDER BY Idx', (entry,))]
    expected = [12345] if entry == protected else [] if entry == spawned else list(dict.fromkeys(evidence['displays']))
    assert actual == expected, (entry, actual, expected)
assert db.execute('SELECT name,faction,type FROM creature_template WHERE entry=?', (protected,)).fetchone() == ('Preserved',14,7)
assert not db.execute('SELECT 1 FROM creature_template WHERE entry=?', (spawned,)).fetchall()
db.executescript(portable)
assert snapshot == db.execute('SELECT * FROM creature_template_model ORDER BY CreatureID,Idx').fetchall()
command = os.environ.get('COA_TRANSFORM_MYSQL_COMMAND')
if command:
    query = 'SELECT CreatureID,Idx,CreatureDisplayID,DisplayScale,Probability FROM creature_template_model ORDER BY CreatureID,Idx;'
    prefix = 'transform_test_' + uuid.uuid4().hex[:12] + '_'
    names = ('creature_template_model', 'creature_template', 'creature')
    script = schema.replace('CREATE TEMPORARY TABLE', 'CREATE TABLE') + fixture + sql + query + sql + query
    for name in names:
        script = re.sub(r'\b' + name + r'\b', prefix + name, script)
    try:
        result = subprocess.run(json.loads(command), input=script.encode(), capture_output=True)
    finally:
        cleanup = ''.join('DROP TABLE IF EXISTS ' + prefix + name + ';' for name in names)
        subprocess.run(json.loads(command), input=cleanup.encode(), capture_output=True, check=True)
    assert result.returncode == 0, result.stderr.decode()
    lines = result.stdout.decode().splitlines()
    expected = ['\t'.join(str(int(v)) if isinstance(v, float) and v.is_integer() else str(v) for v in row) for row in snapshot]
    assert lines == expected + expected, result.stdout.decode()
    print('MySQL migration parity and repeated application passed using isolated fixture tables.')
print(f'Checked {len(repairs)} mappings, Ascendance targets, idempotency, existing data preservation and spawn exclusion.')
