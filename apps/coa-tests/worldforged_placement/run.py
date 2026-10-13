from pathlib import Path
import sqlite3

root = Path(__file__).resolve().parents[3]
sql = (root / 'data/sql/updates/pending_db_world/rev_20261012_06_area52_flameburst_cache.sql').read_text()

def database():
    db = sqlite3.connect(':memory:')
    db.executescript('''
        CREATE TABLE gameobject (
            guid INTEGER PRIMARY KEY, id INTEGER, map INTEGER, zoneId INTEGER, areaId INTEGER,
            spawnMask INTEGER, phaseMask INTEGER, position_x REAL, position_y REAL, position_z REAL,
            orientation REAL, rotation0 REAL, rotation1 REAL, rotation2 REAL, rotation3 REAL,
            spawntimesecs INTEGER, animprogress INTEGER, state INTEGER, ScriptName TEXT, Comment TEXT);
        CREATE TABLE gameobject_template (entry INTEGER, ScriptName TEXT);
        CREATE TABLE gameobject_loot_template (Entry INTEGER, Item INTEGER, Chance REAL);
        CREATE TABLE item_template (entry INTEGER, ScriptName TEXT);
        INSERT INTO gameobject_template VALUES (1345173, 'worldforged_pickup');
        INSERT INTO gameobject_loot_template VALUES (1345173, 100369, 100);
        INSERT INTO item_template VALUES (100369, 'item_area52_identified_mystic');
    ''')
    return db

db = database()
db.executescript(sql)
first = db.execute('SELECT * FROM gameobject').fetchall()
assert len(first) == 1 and first[0][:3] == (6910429, 1345173, 389)
db.executescript(sql)
assert db.execute('SELECT * FROM gameobject').fetchall() == first

for table in ('gameobject_template', 'gameobject_loot_template', 'item_template'):
    db = database()
    db.execute('DELETE FROM ' + table)
    db.executescript(sql)
    assert db.execute('SELECT COUNT(*) FROM gameobject').fetchone()[0] == 0

for guid, entry, location in ((6910429, 42, 1), (7000000, 1345173, 389)):
    db = database()
    db.execute('INSERT INTO gameobject (guid,id,map,Comment) VALUES (?,?,?,?)',
               (guid, entry, location, 'existing content'))
    before = db.execute('SELECT * FROM gameobject').fetchall()
    db.executescript(sql)
    assert db.execute('SELECT * FROM gameobject').fetchall() == before

print('Placement SQL: stable identity, idempotency, binding gates and existing-spawn preservation passed')
