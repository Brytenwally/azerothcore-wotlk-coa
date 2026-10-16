from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(os.environ.get('AREA52_PACKAGE_DIR',Path(__file__).resolve().parent))
TX=Path(os.environ.get('AREA52_TRANSACTION_DIR',ROOT/'offline-transaction'))
PLAN=json.loads((ROOT/'consolidation-plan-private.json').read_text(encoding='utf-8-sig'))
NATIVE_BOUNDS=json.loads((ROOT/'native-bounds.json').read_text(encoding='utf-8'))
COMMAND=['docker','exec','-i','ascension-freepick-database','mysql','--defaults-extra-file=/run/secrets/mysql-root.cnf','--default-character-set=utf8mb4','-BN','acore_world']

def sql(text):
 r=subprocess.run(COMMAND,input=text.encode(),capture_output=True)
 if r.returncode:raise RuntimeError('Hotspot SQL operation failed; transaction not committed: '+r.stderr.decode(errors='replace')[-400:])
 return r.stdout.decode()

def ids():return [int(i) for g in PLAN['replacements'] for i in g['source_ids']]
def targets():return [int(g['replacement']['id']) for g in PLAN['replacements']]
def csv(v):return ','.join(str(int(i)) for i in v)
def rows(wanted):
 out={}
 for line in sql('SELECT id,HEX(payload) FROM xp_hotspot_location WHERE id IN ('+csv(wanted)+');').splitlines():
  i,h=line.split('\t');out[int(i)]=bytes.fromhex(h).decode('utf-8')
 return out

def semantic(r):
 if isinstance(r,list):return [semantic(v) for v in r]
 if not isinstance(r,dict):return r
 out={}
 for k,v in r.items():
  if k in ('starts','ends','lastEnd','queued'):continue
  if k in ('enabled','valid','retired','icon','highlight'):v=v in (True,1,'1','true')
  elif k in ('id','revision','map','zone','continent','xp','duration','weight','cooldown','mapArea','quest','objective','poi'):v=int(v)
  elif k in ('x','y','radius'):v=float(v)
  elif k == 'bounds':v=[float(n) for n in v]
  elif k in ('points','parts') and v=='':v=[]
  out[k]=semantic(v)
 return out

def expected_matches(actual,expected):
 a,e=semantic(actual),semantic(expected)
 def subset(left,right):
  if isinstance(right,dict):
   if not isinstance(left,dict):return False
   for k,v in right.items():
    if k not in left:return False
    if k=='bounds' and 'zone' in right:
     canonical=NATIVE_BOUNDS.get(str(right['zone']))
     if left[k]!=v and (canonical is None or left[k]!=[float(n) for n in canonical]):return False
    elif not subset(left[k],v):return False
   return True
  if isinstance(right,list):return isinstance(left,list) and len(left)==len(right) and all(subset(x,y) for x,y in zip(left,right))
  return left==right
 return subset(a,e)

def preflight():
 actual=rows(ids());assert len(actual)==len(ids()),'Source rows missing'
 for i,text in actual.items():assert expected_matches(json.loads(text),PLAN['expected_sources'][str(i)]),'Hotspot changed: '+str(i)
 assert not rows(targets()),'Replacement ID already in use'
 values=dict((int(a),int(b)) for a,b in (s.split('\t') for s in sql('SELECT id,value FROM xp_hotspot_setting WHERE id IN (3,4,5,1000);').splitlines()))
 assert [values.get(k) for k in (3,4,5)]==[150,3600,3600],'Configured defaults changed'
 assert values.get(1000,0)<=PLAN['first_new_id'],'ID allocation changed'
 print('Consolidation preflight passed; source geometry/settings and defaults match; no writes.')

def backup():
 preflight();TX.mkdir(parents=True,exist_ok=True)
 p=TX/'hotspot-consolidation-backup.json';assert not p.exists(),'Backup already exists'
 data={'rows':rows(ids()),'floor':sql('SELECT value FROM xp_hotspot_setting WHERE id=1000;').strip()}
 p.write_text(json.dumps(data,ensure_ascii=False),encoding='utf-8')
 assert json.loads(p.read_text(encoding='utf-8'))==json.loads(json.dumps(data))

def guard_table(current):
 values=','.join('('+str(i)+",'"+hashlib.sha256(text.encode()).hexdigest()+"')" for i,text in current.items())
 return "CREATE TEMPORARY TABLE expected_hotspots(id INT UNSIGNED PRIMARY KEY,digest CHAR(64));INSERT INTO expected_hotspots VALUES "+values+";CREATE TEMPORARY TABLE hotspot_guard(ok INT CHECK(ok=1));START TRANSACTION;SELECT h.id FROM xp_hotspot_location h JOIN expected_hotspots e ON e.id=h.id FOR UPDATE;INSERT INTO hotspot_guard SELECT IF((SELECT COUNT(*) FROM xp_hotspot_location h JOIN expected_hotspots e ON e.id=h.id AND SHA2(h.payload,256)=e.digest)="+str(len(current))+",1,0);"

def inserts(records):
 return 'INSERT INTO xp_hotspot_location(id,payload) VALUES '+','.join('('+str(i)+",CONVERT(0x"+text.encode().hex()+" USING utf8mb4))" for i,text in records.items())+';'

def apply():
 saved=json.loads((TX/'hotspot-consolidation-backup.json').read_text(encoding='utf-8'))
 originals={int(i):v for i,v in saved['rows'].items()}
 new={int(g['replacement']['id']):json.dumps(g['replacement'],ensure_ascii=False,separators=(',',':')) for g in PLAN['replacements']}
 command=guard_table(originals)
 command+='INSERT INTO hotspot_guard SELECT IF((SELECT COUNT(*) FROM xp_hotspot_location WHERE id IN ('+csv(targets())+'))=0,1,0);'
 command+='INSERT INTO hotspot_guard SELECT IF((SELECT value FROM xp_hotspot_setting WHERE id=1000)='+str(int(saved['floor']))+',1,0);'
 command+='DELETE h FROM xp_hotspot_location h JOIN expected_hotspots e ON h.id=e.id;'+inserts(new)
 command+='UPDATE xp_hotspot_setting SET value='+str(PLAN['next_id'])+' WHERE id=1000;COMMIT;'
 sql(command)

def verify():
 assert not rows(ids()),'Original entries remain'
 actual=rows(targets());assert len(actual)==len(targets()),'Replacement entries missing'
 for g in PLAN['replacements']:
  r=g['replacement'];assert expected_matches(json.loads(actual[int(r['id'])]),r),'Replacement geometry/settings differs: '+str(r['id'])
 assert int(sql('SELECT value FROM xp_hotspot_setting WHERE id=1000;'))>=PLAN['next_id']
 print('Verified: original rows removed, all replacement geometries/settings present, allocation advanced.')

def rollback():
 saved=json.loads((TX/'hotspot-consolidation-backup.json').read_text(encoding='utf-8'));originals={int(i):v for i,v in saved['rows'].items()}
 old,new=rows(ids()),rows(targets())
 if old==originals and not new:return
 assert not old and len(new)==len(targets()),'Unexpected partial/external edits; refusing destructive rollback'
 for g in PLAN['replacements']:
  r=g['replacement'];assert expected_matches(json.loads(new[int(r['id'])]),r),'Replacement edited since apply: '+str(r['id'])
 command=guard_table(new)+'DELETE h FROM xp_hotspot_location h JOIN expected_hotspots e ON h.id=e.id;'+inserts(originals)
 command+='INSERT INTO hotspot_guard SELECT IF((SELECT value FROM xp_hotspot_setting WHERE id=1000)='+str(PLAN['next_id'])+',1,0);'
 command+='UPDATE xp_hotspot_setting SET value='+str(int(saved['floor']))+' WHERE id=1000;COMMIT;'
 sql(command);assert rows(ids())==originals and not rows(targets()),'Rollback readback mismatch'
 print('Exact source rows and ID floor restored.')

if __name__=='__main__':
 if len(sys.argv)!=2 or sys.argv[1] not in ('preflight','backup','apply','verify','rollback'):raise RuntimeError('Unknown consolidation phase')
 globals()[sys.argv[1]]()

