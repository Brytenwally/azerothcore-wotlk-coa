from pathlib import Path
import json
import sqlite3

root = Path(__file__).resolve().parents[3]
rows = json.loads((root / 'docs/coa/area52-worldforged-boss-drops.json').read_text())
sql = (root / 'data/sql/updates/pending_db_world/rev_20261012_08_area52_worldforged_boss_drops.sql').read_text()
assert len(rows) == 33
assert all(r['chance'] == (100 if r['name'] in ('Shadow Word: Gore','Lunar Aspiration','Megafrost') else 50) for r in rows)
db = sqlite3.connect(':memory:')
db.executescript('''
    CREATE TABLE creature_template (entry INTEGER PRIMARY KEY, lootid INTEGER,
        difficulty_entry_1 INTEGER, difficulty_entry_2 INTEGER, difficulty_entry_3 INTEGER);
    CREATE TABLE creature_loot_template (Entry INTEGER, Item INTEGER, Reference INTEGER,
        Chance REAL, QuestRequired INTEGER, LootMode INTEGER, GroupId INTEGER,
        MinCount INTEGER, MaxCount INTEGER, Comment TEXT, PRIMARY KEY(Entry,Item));
    INSERT INTO creature_loot_template VALUES (1978660000,97866,0,25,0,1,0,1,1,'shared original');
    INSERT INTO creature_template VALUES (42,1978660000,0,0,0);
''')
for boss in {r['boss'] for r in rows}:
    if boss == 11957:
        db.execute('INSERT INTO creature_template VALUES (?,1978660000,0,0,0)',(boss,))
    else:
        db.execute('INSERT INTO creature_template VALUES (?,?,?,?,0)',(boss,boss,boss+100000,boss+200000))
        db.execute('INSERT INTO creature_template VALUES (?,?,0,0,0)',(boss+100000,boss))
        db.execute('INSERT INTO creature_template VALUES (?,?,0,0,0)',(boss+200000,boss+200000))
db.executescript(sql)
for r in rows:
    lootids = [990115] if r['boss']==11957 else [r['boss'],r['boss']+200000]
    for loot in lootids:
        assert db.execute('SELECT Chance,GroupId,MinCount,MaxCount FROM creature_loot_template '
                          'WHERE Entry=? AND Item=?',(loot,r['item'])).fetchone() == (r['chance'],0,1,1)
assert db.execute('SELECT COUNT(*) FROM creature_loot_template WHERE Entry=1978660000').fetchone()[0] == 1
assert db.execute('SELECT lootid FROM creature_template WHERE entry=42').fetchone()[0] == 1978660000
before = db.execute('SELECT * FROM creature_loot_template ORDER BY Entry,Item').fetchall()
db.executescript(sql)
assert db.execute('SELECT * FROM creature_loot_template ORDER BY Entry,Item').fetchall() == before
print('33 drops: default/documented rates, difficulty loot reuse, generic-table isolation and idempotency passed')
