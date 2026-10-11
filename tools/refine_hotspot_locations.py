import argparse
from collections import defaultdict
import copy
import hashlib
import json
from pathlib import Path

from import_hotspot_locations import map_rows


def cross(a, b, c):
    return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])


def segment(a, b, p):
    return abs(cross(a, b, p)) < .0001 and all(
        min(a[i], b[i])-.0001 <= p[i] <= max(a[i], b[i])+.0001 for i in (0, 1))


def polygon(points):
    points = [tuple(p) for i, p in enumerate(points) if not i or tuple(p) != tuple(points[i-1])]
    if len(points) > 1 and points[0] == points[-1]:
        points.pop()
    changed = True
    while changed and len(points) > 3:
        changed = False
        for i, p in enumerate(points):
            if segment(points[i-1], points[(i+1) % len(points)], p):
                del points[i]
                changed = True
                break
    if not 3 <= len(points) <= 64 or any(abs(v) > 20000 for p in points for v in p):
        return None
    edges = list(zip(points, points[1:]+points[:1]))
    if abs(sum(a[0]*b[1]-b[0]*a[1] for a, b in edges)) <= 1:
        return None
    for i, (a, b) in enumerate(edges):
        for j, (c, d) in enumerate(edges[i+1:], i+1):
            if j == i+1 or i == 0 and j == len(points)-1:
                continue
            if (segment(a, b, c) or segment(a, b, d) or segment(c, d, a) or segment(c, d, b) or
                (cross(a, b, c) > 0) != (cross(a, b, d) > 0) and
                    (cross(c, d, a) > 0) != (cross(c, d, b) > 0)):
                return None
    options = []
    for ring in (points, points[::-1]):
        start = min(range(len(ring)), key=ring.__getitem__)
        options.append(tuple(ring[start:]+ring[:start]))
    return min(options)


def area_name(row, zones):
    zone = zones.get(row['zone'])
    if not zone:
        return 'Area '+str(row['zone'])
    bounds = zone.get('bounds', [])
    if len(bounds) != 4 or bounds[0] == bounds[1] or bounds[2] == bounds[3]:
        return zone['name']+' vicinity'
    y = (row['x']-bounds[0])/(bounds[1]-bounds[0])
    x = (row['y']-bounds[2])/(bounds[3]-bounds[2])
    ns = 'North' if y < .35 else 'South' if y > .65 else ''
    ew = 'west' if x < .35 else 'east' if x > .65 else ''
    direction = ns+ew if ns else ew.capitalize()
    return (zone['name']+' — '+(direction or 'Central'))[:100]


def refine(seed, maps, pois, vertices, holidays):
    rows = copy.deepcopy(seed)
    zones = {r['zone']: r for r in rows if r['kind'] == 'zone'}
    map_index = {int(r['ID']): r for r in maps}
    points = defaultdict(list)
    for quest, poi, idx, x, y in sorted(vertices):
        points[quest, poi].append((x, y))
    by_objective = defaultdict(list)
    additions = []
    for quest, poi, objective, world, area, floor in sorted(pois):
        m = map_index.get(area)
        if objective < 0 or floor or quest in holidays or not m or world not in (0, 1, 530, 571):
            continue
        z = int(m['AreaID'])
        if z not in zones or zones[z]['map'] != world:
            continue
        ring = polygon(points[quest, poi])
        if not ring:
            continue
        r = copy.deepcopy(zones[z])
        r.update(id=200000+len(additions), kind='polygon', source=f'quest-poi:{quest}:{poi}',
                 points=[dict(x=x, y=y) for x, y in ring], radius=0,
                 x=sum(p[0] for p in ring)/len(ring), y=sum(p[1] for p in ring)/len(ring),
                 enabled=True, valid=True, geometrySource='effective-quest-poi',
                 quest=quest, objective=objective, poi=poi)
        r['name'] = area_name(r, zones)
        additions.append(r)
        by_objective[quest, objective, world].append(r)
    for r in rows:
        if r['kind'] == 'zone':
            continue
        original = r['original']
        quest, objective = int(original['quest_id']), int(original['objective_idx'])
        r['name'] = area_name(r, zones)
        if quest in holidays:
            r.update(retired=True, enabled=False, retirementReason='holiday', ends=0, queued=0)
        elif by_objective[quest, objective, r['map']]:
            r.update(retired=True, enabled=False, ends=0, queued=0, retirementReason='precise-quest-poi',
                     replacedBy=[p['id'] for p in by_objective[quest, objective, r['map']]])
        else:
            r['geometrySource'] = 'approximate-spawn-circle'
    rows.extend(additions)
    unique = {}
    for r in rows:
        if r['kind'] == 'zone' or r.get('retired') or not r['valid']:
            continue
        geometry = (r['x'], r['y'], r['radius']) if r['kind'] == 'circle' else polygon(
            [(p['x'], p['y']) for p in r['points']])
        key = (r['map'], r['zone'], r['kind'], geometry)
        if key in unique:
            keeper = unique[key]
            keeper.setdefault('equivalentSources', []).append(r['source'])
            r.update(retired=True, enabled=False, ends=0, queued=0,
                     retirementReason='identical-boundary', duplicateOf=keeper['id'])
        else:
            unique[key] = r
    return rows


def encoded(value):
    return 'CONVERT(0x'+json.dumps(value, ensure_ascii=False, separators=(',', ':')).encode().hex()+' USING utf8mb4)'


def migration(seed, rows):
    before = {r['id']: r for r in seed}
    sql = ['START TRANSACTION;']
    for r in rows:
        old = before.get(r['id'])
        if old == r:
            continue
        if not old:
            sql.append('INSERT IGNORE INTO `xp_hotspot_location` (`id`, `payload`) '
                       f"VALUES ({r['id']}, {encoded(r)});")
            continue
        changes = {k: v for k, v in r.items() if k not in old or old[k] != v}
        changes['revision'] = 2
        args = ', '.join("'$."+k+"', JSON_EXTRACT("+encoded(v)+", '$')"
                         for k, v in changes.items())
        sql.append('UPDATE `xp_hotspot_location`\nSET `payload` =\n    JSON_SET(`payload`, '+args+
                   f")\nWHERE `id`={r['id']} AND\n    JSON_UNQUOTE(JSON_EXTRACT(`payload`, '$.revision'))='1'\n"
                   "    AND JSON_UNQUOTE(JSON_EXTRACT(`payload`, '$.name'))=JSON_UNQUOTE("+encoded(old['name'])+");")
    sql.append('COMMIT;')
    return '\n'.join(sql)+'\n'


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--seed', type=Path, required=True)
    p.add_argument('--maps', type=Path, required=True)
    p.add_argument('--export', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    seed = json.loads(args.seed.read_text())
    def table(name):
        return [tuple(map(int, line.split('\t'))) for line in (args.export/(name+'.tsv')).read_text().splitlines()]
    holidays = {r[0] for r in table('holidays')}
    for line in (args.export/'quests.tsv').read_text().splitlines():
        quest, sort, title = line.split('\t')
        if -int(sort) in {22, 364, 366, 369, 370, 374, 375, 376}:
            holidays.add(int(quest))
    rows = refine(seed, map_rows(args.maps), table('poi'), table('points'), holidays)
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output/'locations.json').write_text(json.dumps(rows, indent=2)+'\n', encoding='utf-8')
    (args.output/'rev_20261011_02_hotspot_boundaries.sql').write_text(migration(seed, rows), encoding='utf-8')
    report = dict(total=len(rows), active_catalog=sum(not r.get('retired') for r in rows),
                  polygons=sum(r['kind']=='polygon' and not r.get('retired') for r in rows),
                  retired=sum(bool(r.get('retired')) for r in rows),
                  barrens=sum(r['zone']==17 and not r.get('retired') for r in rows),
                  hashes={f.name: hashlib.sha256(f.read_bytes()).hexdigest() for f in args.export.glob('*.tsv')})
    (args.output/'provenance.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
