from pathlib import Path
import json
import math
import sqlite3

root = Path(__file__).resolve().parents[3]
batch = json.loads((root / 'docs/coa/area52-worldforged-placement-batch.json').read_text())
sql = (root / 'data/sql/updates/pending_db_world/rev_20261012_07_area52_worldforged_cache_placements.sql').read_text()
sql = sql.replace('ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`)', 'ON CONFLICT(entry) DO NOTHING')
rows = batch['placements']
assert len(rows) == 22
assert len({r['entry'] for r in rows}) == len(rows)
assert len({r['guid'] for r in rows}) == len(rows)
assert batch['bind_on_pickup'] and not batch['permanent_claim']
for r in rows:
    a, p = r['anchor'], r['placement']
    assert a['map'] == p['map'] and p['spawn_mask'] == 1 and p['phase_mask'] == 1
    assert abs(math.hypot(a['x'] - p['x'], a['y'] - p['y']) - 3) < 0.001
    assert 'reconstructed' in r['provenance']

db = sqlite3.connect(':memory:')
db.executescript('''
    CREATE TABLE gameobject_template (
        entry INTEGER PRIMARY KEY, type INTEGER, displayId INTEGER, name TEXT,
        size REAL, data0 INTEGER, data1 INTEGER, ScriptName TEXT);
    CREATE TABLE gameobject (
        guid INTEGER PRIMARY KEY, id INTEGER, map INTEGER, spawnMask INTEGER, phaseMask INTEGER,
        position_x REAL, position_y REAL, position_z REAL, orientation REAL,
        rotation2 REAL, rotation3 REAL, spawntimesecs INTEGER, state INTEGER,
        ScriptName TEXT, Comment TEXT);
    INSERT INTO gameobject_template (entry, ScriptName) VALUES (990100, 'unrelated');
    INSERT INTO gameobject (guid, id, Comment) VALUES (9791001, 42, 'unrelated');
''')
db.executescript(sql)
assert db.execute('SELECT COUNT(*) FROM gameobject').fetchone()[0] == 21
assert db.execute('SELECT ScriptName FROM gameobject_template WHERE entry=990100').fetchone()[0] == 'unrelated'
assert db.execute('SELECT id FROM gameobject WHERE guid=9791001').fetchone()[0] == 42
before = db.execute('SELECT * FROM gameobject ORDER BY guid').fetchall()
db.executescript(sql)
assert db.execute('SELECT * FROM gameobject ORDER BY guid').fetchall() == before
assert all(r[0] == batch['required_handler'] for r in db.execute(
    'SELECT ScriptName FROM gameobject WHERE id != 42'))
print('22 reconstructed placements: mappings, normal-mode scope, collision protection and idempotency passed')
