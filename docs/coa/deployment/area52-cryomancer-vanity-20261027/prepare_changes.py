from pathlib import Path
import struct,json
T=Path(__file__).resolve().parent
source=T/'baseline/src/server/coa/Area52ArchetypeVanityData.h';text=source.read_text()
old='{9901097, {40, 53, 7712, 24858, 24859, 24983, 24984, 24985, 24986, 25033, 900597}}'
new='{9901097, {40, 53, 12911, 24858, 24859, 24983, 24984, 24985, 24986, 25033, 900597}}'
assert text.count(old)==1;text=text.replace(old,new)
old='{9901097, {116470, 1001755, 1001756, 1001757, 1001758, 1001759, 1001760, 1001761, 1001762, 1001763}}'
new='{9901097, {106075, 170002, 1001755, 1001756, 1001757, 1001758, 1001759, 1001760, 1001761, 1001762, 1001763}}'
assert text.count(old)==1;text=text.replace(old,new)
p=T/'source/src/server/coa/Area52ArchetypeVanityData.h';p.parent.mkdir(parents=True,exist_ok=True);p.write_text(text)
changes={}
for root in ['client','server']:
 (T/'candidate'/root).mkdir(parents=True,exist_ok=True)
 for name in ['CharacterCreationArchetypeDetails','VanityCollection','ItemSet']:
  p=T/root/(name+'.dbc')
  if not p.exists():continue
  b=p.read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);out=bytearray(b);changed=[]
  for i in range(n):
   row=list(struct.unpack_from('<'+'I'*f,b,20+i*z));before=row[:]
   if name=='CharacterCreationArchetypeDetails' and row[1]==97:
    assert row[27]==116470;row[27]=106075
   elif name=='ItemSet' and row[0]==90097:
    assert row[27]==116470;row[27]=106075
   elif name=='VanityCollection' and row[1]==9901097:
    assert row[18]==116470 and row[28]==0;row[18]=106075;row[28]=170002
   if row!=before:struct.pack_into('<'+'I'*f,out,20+i*z,*row);changed.append({'id':row[0],'fields':[j for j in range(f) if row[j]!=before[j]]})
  assert changed,name
  (T/'candidate'/root/(name+'.dbc')).write_bytes(out);changes[root+'/'+name]=changed
(T/'staged-changes.json').write_text(json.dumps(changes,indent=2)+'\n')
print('Staged Cryomancer weapon/preview/set membership and mount reward; no installed or server changes.')
