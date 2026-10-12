from pathlib import Path
import importlib.util
import shutil
import sqlite3
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
source = (root / 'src/server/coa/AscensionSpartanKick.cpp').read_text()
check = source[source.index('    bool Check('):source.index('    void Handle(')]
code = r'''
#include <cassert>
#include <set>
constexpr unsigned SpartanKick = 9901766;
struct SpellInfo { unsigned Id; SpellInfo const* GetFirstRankSpell() const { return this; } };
struct Unit {
    bool hostile = false;
    std::set<unsigned> auras;
    bool IsValidAttackTarget(Unit* target) { return target != this && target->hostile; }
    bool HasAura(unsigned id) { return auras.contains(id); }
};
struct ProcEventInfo {
    Unit* actor;
    Unit* target;
    SpellInfo const* spell;
    Unit* GetActor() { return actor; }
    Unit* GetActionTarget() { return target; }
    SpellInfo const* GetSpellInfo() { return spell; }
};
struct Fixture {
    Unit* owner;
    Unit* GetTarget() { return owner; }
''' + check + r'''
};
int main() {
    Unit owner, hostile, friendly;
    hostile.hostile = true;
    SpellInfo kick{1766}, empowered{9901766}, other{1752};
    Fixture f{&owner};
    ProcEventInfo event{&owner, &hostile, &kick};
    assert(!f.Check(event));
    for (unsigned stance : {2457u, 71u, 997743u}) {
        owner.auras = {stance};
        assert(f.Check(event));
    }
    event.spell = &empowered;
    assert(f.Check(event));
    event.spell = &kick;
    event.target = &friendly;
    assert(!f.Check(event));
    event.target = nullptr;
    assert(!f.Check(event));
    event.target = &hostile;
    event.actor = &hostile;
    assert(!f.Check(event));
    event.actor = &owner;
    event.spell = &other;
    assert(!f.Check(event));
    event.spell = nullptr;
    assert(!f.Check(event));
}
'''
with tempfile.TemporaryDirectory() as directory:
    p = Path(directory)
    (p / 'check.cpp').write_text(code)
    subprocess.run([shutil.which('c++'), '-std=c++20', '-Wall', '-Wextra', '-Werror',
                    str(p / 'check.cpp'), '-o', str(p / 'check')], check=True)
    subprocess.run([str(p / 'check')], check=True)

db = sqlite3.connect(':memory:')
db.executescript('CREATE TABLE spell_script_names(spell_id INTEGER, ScriptName TEXT); '
                "INSERT INTO spell_script_names VALUES(84445,'spell_area52_hoplite_support_proc'),(84445,'unrelated');")
columns = 'SpellId SchoolMask SpellFamilyName SpellFamilyMask0 SpellFamilyMask1 SpellFamilyMask2 ProcFlags SpellTypeMask SpellPhaseMask HitMask AttributesMask DisableEffectsMask ProcsPerMinute Chance Cooldown Charges'.split()
db.execute('CREATE TABLE spell_proc (' + ','.join(name + ' INTEGER' for name in columns) + ')')
sql = (root / 'data/sql/updates/pending_db_world/rev_20261010_91_area52_spartan_kick.sql').read_text()
for _ in range(2):
    db.executescript(sql)
    assert db.execute('SELECT ScriptName FROM spell_script_names ORDER BY ScriptName').fetchall() == [('aura_area52_spartan_kick',), ('unrelated',)]
    assert db.execute('SELECT Chance,Cooldown FROM spell_proc').fetchall() == [(100, 20000)]

spec = importlib.util.spec_from_file_location('spartan_patch', root / 'apps/coa-dbc/patch_area52_spartan_kick.py')
patcher = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patcher)
original = 'Your Kick ability now knocks players back. Requires Battle, Defensive or Phalanx Stance. This effect cannot occur more than once every 20 seconds.\n\n@ext:|cffffff00VERIFIED|r:ext@'
pool = b'\0' + original.encode() + b'\0'
kick = [0] * 234
kick[0] = 1766
kick[1] = 88
kick[30] = 12000
row = [0] * 234
row[0] = 84445
row[170] = 1
raw = struct.pack('<4s4I', b'WDBC', 2, 234, 936, len(pool)) + struct.pack('<234I', *row) + struct.pack('<234I', *kick) + pool
result = patcher.patch(raw)
assert patcher.patch(result) == result
pointer = struct.unpack_from('<I', result, 20 + 170 * 4)[0]
strings = result[20 + 3 * 936:]
text = strings[pointer:strings.index(0, pointer)].decode()
assert 'own 20-second cooldown' in text and text.endswith('\n\n@ext:|cffffff00VERIFIED|r:ext@')
replacement = struct.unpack_from('<234I', result, 20 + 2 * 936)
assert (replacement[0], replacement[1], replacement[24], replacement[29], replacement[30]) == (9901766, 0, 0, 20000, 0)
assert struct.unpack_from('<234I', result, 20 + 936) == tuple(kick)
print('PASS: production eligibility, all stances, non-Kick/friendly rejection, single binding, cooldown and tooltip preservation')

import re
sql = (root / 'data/sql/updates/pending_db_world/rev_20261010_92_area52_spartan_cooldown.sql').read_text()
columns = re.search(r'INSERT INTO `spell_dbc` \((.*?)\) VALUES', sql).group(1)
db.execute('CREATE TABLE spell_dbc (' + columns.replace('`, `', '` NUMERIC, `') + ' NUMERIC)')
for _ in range(2):
    db.executescript(sql)
    assert db.execute('SELECT ID,Category,RecoveryTime,CategoryRecoveryTime,CasterAuraSpell FROM spell_dbc').fetchall() == [(9901766,0,20000,0,84445)]
print('PASS: separate Kick cooldown record is idempotent and requires the enchant')
