from pathlib import Path
import subprocess,tempfile
R=Path(__file__).resolve().parents[3]
s=(R/'src/server/game/Entities/Unit/Unit.cpp').read_text()
start=s.index('            coeff = bonus->direct_damage;',s.index('uint32 Unit::SpellDamageBonusDone'))
end=s.index('\n        }',start)
ap=s[start:end]
sp='        DoneTotal += int32(DoneAdvertisedBenefit * coeff * factorMod);'
assert sp in s
sql=(R/'data/sql/updates/pending_db_world/rev_20261028_50_archetype_verification.sql').read_text()
assert '(954250, 0.864, 0, 0.864, 0,' in sql
code=r'''
#include <cassert>
using int32=int;
using WeaponAttackType=int;
constexpr int BASE_ATTACK=0,RANGED_ATTACK=2,SPELL_DAMAGE_CLASS_MELEE=2;
constexpr int SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS=1,SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS=2;
struct Info {bool UseRangedAttackPowerForDamage=false;int DmgClass=0;bool IsRangedWeaponSpell()const{return false;}};
struct Bonus {float direct_damage=0.864f,ap_bonus=0.864f;};
struct Victim {int GetTotalAuraModifier(int)const{return 0;}};
struct Damage {
 float attack=0;
 float GetTotalAttackPowerValue(int)const{return attack;}
 float GetSpellAttackPowerCoefficientFlatBonus(Info const*)const{return 0;}
 int Calculate(float power){
  Info info;Info const* spellProto=&info;Bonus row;Bonus const* bonus=&row;
  Victim target;Victim* victim=&target;int DoneTotal=0;float coeff=0;
  float area52CoefficientScale=1,ApCoeffMod=1,stack=1,factorMod=1,DoneAdvertisedBenefit=power;
  BODY_AP
  BODY_SP
  return 100+DoneTotal;
 }
};
int main(){Damage d;assert(d.Calculate(0)==100);assert(d.Calculate(500)==532);
 d.attack=1000;assert(d.Calculate(0)==964);assert(d.Calculate(500)==1396);}
'''.replace('BODY_AP',ap).replace('BODY_SP',sp)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'damage.cpp').write_text(code)
 subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',str(p/'damage.cpp'),'-o',str(p/'damage')],check=True)
 subprocess.run([str(p/'damage')],check=True)
print('PASS Dragon Roar native AP/SP coefficient fragments; SQL coefficients and armor metadata checked separately')
