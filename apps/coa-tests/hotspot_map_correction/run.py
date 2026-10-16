from pathlib import Path
import re
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def dbc(name):
    raw = (Path('/data/dbc') / name).read_bytes()
    magic, count, fields, size, strings = struct.unpack_from('<4s4I', raw)
    assert magic == b'WDBC' and size == fields * 4 and len(raw) == 20 + count * size + strings
    return [struct.unpack_from('<' + str(fields) + 'I', raw, 20 + i * size) for i in range(count)]


def main():
    source = (ROOT / 'src/server/coa/XPHotspots.cpp').read_text()
    allowed = re.search(r'bool Allowed\(Location const& r\)\n\{.*?\n\}', source, re.S).group()
    quota = re.search(r'size_t FactionQuota\(Location const& r\)\n\{.*?\n\}', source, re.S).group()
    start = source.index('            auto entry = sMapStore.LookupEntry(r.Map);', source.index('if (action == "newmap")'))
    end = source.index('            r.Continent = Continent', start)
    draft = source[start:end]
    draft = re.sub(r'\{\n\s+Send\(player,.*?return;\n\s+\}', '{ return false; }', draft, flags=re.S)
    areas = dbc('AreaTable.dbc')
    maps = dbc('Map.dbc')
    by_id = {row[0]: row for row in areas}
    for sid in (14, 17, 215):
        row = by_id[sid]
        assert row[1] == 1 and row[2] == 0
        wrong = by_id.get(row[3])
        assert wrong is None or wrong[1] != 1 or wrong[2] != 0
    code = '''#include <cassert>
#include <cmath>
#include <map>
#include <string>
using size_t = std::size_t;
struct Area { unsigned mapid, zone, team; };
struct Map { unsigned type; bool Instanceable() const { return type >= 1 && type <= 4; } };
template<class T> struct Store { std::map<unsigned,T> rows; T const* LookupEntry(unsigned id) const { auto i=rows.find(id); return i==rows.end()?nullptr:&i->second; } };
Store<Area> sAreaTableStore;
Store<Map> sMapStore;
enum { AREATEAM_ALLY=2, AREATEAM_HORDE=4, Alliance=0, Horde=1, Neutral=2, MaxContinent=3 };
struct Location { unsigned Map=1, Zone=14, Continent=0; bool Retired=false, Valid=true; std::string Kind="zone"; struct {double X=0,Y=0;} Center; };
'''
    code += allowed + '\n' + quota + '\nbool Draft(Location const& r) {\n' + draft + '\nreturn true; }\nint main() {\n'
    code += ''.join(f'sAreaTableStore.rows[{r[0]}] = {{{r[1]}, {r[2]}, {r[28]}}};\n' for r in areas)
    code += ''.join(f'sMapStore.rows[{r[0]}] = {{{r[2]}}};\n' for r in maps)
    code += 'Location r;\n'
    count = 0
    for row in areas:
        if row[1] not in (0, 1, 530, 571) or row[2]:
            continue
        expected = 0 if row[28] == 2 else 1 if row[28] == 4 else 2
        code += f'r.Map={row[1]}; r.Zone={row[0]}; assert(Draft(r)); assert(Allowed(r)); assert(FactionQuota(r)=={expected});\n'
        count += 1
    code += '''r.Map=1; r.Zone=14; assert(Draft(r));
r.Map=0; assert(!Draft(r)); assert(!Allowed(r));
r.Map=1; r.Zone=999999; assert(!Draft(r)); assert(!Allowed(r));
r.Zone=14; r.Center.X=NAN; assert(!Draft(r));
r.Center.X=INFINITY; assert(!Draft(r));
r.Center.X=20001; assert(!Draft(r));
r.Center.X=0; r.Center.Y=-20001; assert(!Draft(r));
r.Center.Y=0; r.Retired=true; assert(!Allowed(r));
r.Retired=false; r.Valid=false; assert(!Allowed(r));
r.Valid=true; r.Continent=4; assert(!Allowed(r));
r.Continent=0; sMapStore.rows[1].type=1; assert(!Draft(r)); assert(!Allowed(r));
sMapStore.rows[1].type=0; sAreaTableStore.rows[14].zone=17; assert(!Draft(r)); assert(!Allowed(r));
}'''
    with tempfile.TemporaryDirectory() as folder:
        p = Path(folder)
        (p / 'test.cpp').write_text(code)
        subprocess.run(['c++', '-std=c++17', '-Wall', '-Wextra', '-Werror', str(p / 'test.cpp'), '-o', str(p / 'test')], check=True)
        subprocess.run([str(p / 'test')], check=True)
    print(f'PASS: reproduced wrong exploration-flag lookup for all three reported zones; executed extracted draft, eligibility and faction quota code for {count} native outdoor zones and rejection boundaries. Store access mocked; no gameplay.')


if __name__ == '__main__':
    main()
