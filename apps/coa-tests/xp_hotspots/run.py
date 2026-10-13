from pathlib import Path
import json
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
hotspots = (root/'src/server/coa/XPHotspots.cpp').read_text()
assert 'GetAreaEntryByAreaID(r.Zone)' not in hotspots
assert hotspots.count('sAreaTableStore.LookupEntry(r.Zone)') == 3
assert 'AreaTableEntryfmt[] = "n' in (root/'src/server/shared/DataStores/DBCfmt.h').read_text()

source = r'''
#include "XPHotspotRules.h"
#include "XPHotspotPainting.h"
#include <cassert>
#include <limits>
#include <iostream>
int main()
{
    using namespace XPHotspots;
    for (auto action : {"new", "newmap", "merge", "brush", "shape", "point", "center", "clear"})
    {
        assert(CanManage(2, action));
        assert(CanManage(3, action));
    }
    for (auto action : {"general", "edit", "save", "stop", "next", "enable", "disable", "announce", "teleport"})
    {
        assert(CanManage(2, action));
        assert(!CanManage(1, action));
    }
    assert(!CanManage(2, "delete"));
    assert(CanManage(3, "delete"));
    RequestWindow requests;
    for (int i = 0; i < 8; ++i)
        assert(requests.Accept(5000));
    assert(!requests.Accept(5000));
    assert(!requests.Accept(5999));
    assert(requests.Accept(6000));
    GeneralSettings settings;
    assert(ValidSettings(settings));
    std::array<uint32_t, SettingCount> counts{};
    assert(QuotaAvailable(settings, counts, EasternKingdoms, Alliance));
    settings.Values[Alliance] = 1;
    counts[Alliance] = 1;
    assert(!QuotaAvailable(settings, counts, EasternKingdoms, Alliance));
    assert(QuotaAvailable(settings, counts, EasternKingdoms, Horde));
    counts[EasternKingdoms] = 20;
    assert(!QuotaAvailable(settings, counts, EasternKingdoms, Horde));
    assert(QuotaAvailable(settings, counts, Kalimdor, Horde));
    settings.Values[ActiveCount] = 0;
    assert(ValidSettings(settings));
    assert(!QuotaAvailable(settings, counts, Kalimdor, Horde));
    settings.Values[ActiveCount] = 21;
    assert(!ValidSettings(settings));
    settings.Values[ActiveCount] = 3;
    settings.Values[DefaultDuration] = 59;
    assert(!ValidSettings(settings));
    settings.Values[DefaultDuration] = 86400;
    settings.Values[DefaultCooldown] = 604801;
    assert(!ValidSettings(settings));
    settings.Values[DefaultCooldown] = 604800;
    assert(ValidSettings(settings));
    assert(ContinentQuota(530,3430) == EasternKingdoms);
    assert(ContinentQuota(530,3524) == Kalimdor);
    assert(ContinentQuota(530,3483) == Outland);
    assert(ContinentQuota(571,65) == Northrend);
    Location r;
    r.Name = "Test";
    r.Map = 0;
    r.Zone = 44;
    r.Enabled = true;
    assert(Valid(r));
    assert(Contains(r, 0, 44, {150,0}));
    assert(!Contains(r, 0, 44, {150.01,0}));
    assert(!Contains(r, 1, 44, {0,0}));
    r.Kind = "zone";
    assert(Contains(r, 0, 44, {10000,10000}));
    assert(!Contains(r, 0, 45, {0,0}));
    r.Kind = "polygon";
    r.Points = {{0,0},{10,0},{10,10},{0,10}};
    assert(Valid(r) && Contains(r,0,44,{5,5}) && Contains(r,0,44,{10,5}));
    assert(!Contains(r,0,44,{11,5}));
    Location painted = r;
    painted.Points.clear();
    painted.Id = 55;
    painted.Revision = 7;
    assert(Paint(painted, {0,0}, {100,0}, 20, false));
    assert(Valid(painted) && Contains(painted,0,44,{50,0}));
    assert(!Contains(painted,0,44,{50,25}));
    assert(Paint(painted, {100,0}, {100,100}, 20, true));
    assert(Contains(painted,0,44,{110,90}) && Contains(painted,0,44,{0,0}));
    assert(!Contains(painted,0,44,{50,50}));
    auto beforePaint = painted.Points;
    assert(!Paint(painted, {1000,1000}, {1100,1000}, 20, false));
    assert(painted.Points.size() == beforePaint.size());
    assert(!Paint(painted, {0,0}, {0,0}, std::numeric_limits<double>::quiet_NaN(), false));
    assert(!Paint(painted, {0,0}, {6000,0}, 20, false));
    assert(ChangeShape(painted, "circle"));
    assert(painted.Id == 55 && painted.Revision == 7 && painted.Kind == "circle");
    for (Point point : beforePaint)
        assert(Contains(painted,0,44,point));
    assert(ChangeShape(painted, "polygon") && Valid(painted));
    Location denied = r;
    denied.Kind = "zone";
    assert(!Paint(denied, {0,0}, {0,0}, 20, false));
    assert(!ChangeShape(denied, "circle"));
    Location batch = r;
    auto originalPoints = batch.Points;
    assert(!ApplyLocalPaint(batch, "B 0 0 10 0 20 0;B 9000 9000 9010 9000 20 0;"));
    assert(batch.Points.size() == originalPoints.size());
    for (size_t i = 0; i < originalPoints.size(); ++i)
        assert(batch.Points[i].X == originalPoints[i].X && batch.Points[i].Y == originalPoints[i].Y);
    assert(ApplyLocalPaint(batch, "B 0 0 20 0 10 0;"));
    assert(Contains(batch, 0, 44, {20,0}));
    assert(ApplyLocalPaint(batch, "T;P 0 0;P 20 0;P 0 20;"));
    assert(!Contains(batch,0,44,{19,19}));
    assert(!ApplyLocalPaint(batch, "T;P 0 0;"));
    assert(!ApplyLocalPaint(batch, "B 0 0 0 0 20 0 garbage;"));
    Location compound = r;
    compound.Kind = "union";
    Location second = r;
    second.Kind = "circle";
    second.Center = {100,100};
    second.Radius = 10;
    compound.Parts = {r, second};
    assert(Valid(compound));
    assert(Contains(compound,0,44,{5,5}));
    assert(Contains(compound,0,44,{100,100}));
    assert(!Contains(compound,0,44,{50,50}));
    assert(!Contains(compound,1,44,{5,5}));
    compound.Parts[1].Zone = 45;
    assert(!Valid(compound));
    compound.Parts = {r, compound};
    assert(!Valid(compound));
    compound.Parts.assign(9, r);
    assert(!Valid(compound));
    Location retired = r;
    retired.Retired = true;
    assert(!Eligible(retired,0,10000));
    r.Points = {{0,0},{10,10},{0,10},{10,0}};
    assert(!Valid(r));
    r.Points = {{0,0},{10,0},{5,0},{5,5}};
    assert(!Valid(r));
    r.Kind = "circle";
    assert(Eligible(r,0,10000));
    r.Continent = 1;
    assert(!Eligible(r,0,10000) && Eligible(r,1,10000));
    r.LastEnd = 9000;
    assert(!Eligible(r,1,10000));
    r.Queued = 9999;
    assert(Eligible(r,1,10000));
    r.Weight = 0;
    assert(Eligible(r,1,10000));
    r.Queued = 0;
    assert(!Eligible(r,1,10000));
    r.Queued = 9999;
    r.Ends = 10001;
    assert(!Eligible(r,1,10000));
    r.Ends = 0;
    r.Enabled = false;
    assert(!Eligible(r,1,10000));
    r.Radius = std::numeric_limits<double>::quiet_NaN();
    assert(!Valid(r));
    Location a, b;
    a.Id = 1;
    a.Weight = 1;
    b.Id = 2;
    b.Weight = 3;
    std::vector<Location> candidates{a,b};
    assert(Select(candidates,1)->Id == 1);
    assert(Select(candidates,2)->Id == 2);
    assert(Select(candidates,4)->Id == 2);
    assert(!Select(candidates,0) && !Select(candidates,5));
    candidates[0].Queued = 10;
    candidates[1].Queued = 11;
    assert(Select(candidates,1)->Id == 2);
    candidates[0].Queued = 12;
    assert(Select(candidates,4)->Id == 1);
    size_t vertices = 0;
    while (std::cin >> vertices)
    {
        Location imported;
        imported.Name = "Imported boundary";
        imported.Kind = "polygon";
        for (size_t i = 0; i < vertices; ++i)
        {
            Point point;
            std::cin >> point.X >> point.Y;
            imported.Points.push_back(point);
        }
        assert(Valid(imported));
    }
}
'''
with tempfile.TemporaryDirectory() as directory:
    out = Path(directory)
    (out/'test.cpp').write_text(source)
    subprocess.run(['c++','-std=c++20','-I'+str(root/'src/server/coa'),str(out/'test.cpp'),
                    '-o',str(out/'test')],check=True)
    migration = (root/'data/sql/updates/pending_db_world/rev_20261011_02_hotspot_boundaries.sql').read_text()
    rows = [json.loads(bytes.fromhex(value)) for value in
            re.findall(r'VALUES \(\d+, CONVERT\(0x([0-9a-f]+) USING', migration)]
    geometry = '\n'.join(str(len(row['points']))+' '+
                         ' '.join(str(p[k]) for p in row['points'] for k in ('x', 'y'))
                         for row in rows if row['kind'] == 'polygon')
    subprocess.run([str(out/'test')],check=True,input=geometry,text=True)
print('Hotspot geometry, realm restrictions and eligibility passed')
