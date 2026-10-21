from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def main():
    text = (ROOT / 'src/server/coa/AscensionMysticEnchant.cpp').read_text()
    functions = text[text.index('constexpr char ActivePresetSetting'):text.index('bool InvestmentMet')]
    send = text[text.index('void SendPresets'):text.index('bool NearAltar')]
    handle = text[text.index('bool Handle(WorldSession*'):text.index('    if (opcode == 0x617)')]
    handle += '    return false;\n}\n'
    header = (ROOT / 'src/server/coa/AscensionFreePick.h').read_text()
    spells = header[header.index('inline constexpr std::array<std::uint32_t, 20> SpecializationSpells'):]
    spells = spells[:spells.index(';') + 1]
    code = r'''#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
using Slots = std::array<std::uint32_t,17>;
struct Value { std::uint32_t value=0; };
struct WorldPacket {
    unsigned opcode; std::vector<std::uint32_t> data; std::string result;
    WorldPacket(unsigned o,unsigned=0):opcode(o){}
    WorldPacket& operator<<(std::uint32_t n) {data.push_back(n);return *this;}
    WorldPacket& operator<<(char const* s) {result=s;return *this;}
    unsigned GetOpcode() const{return opcode;}
    unsigned size() const{return data.size()*4;}
    template<class T> T read(unsigned n) const{return data.at(n/4);}
};
struct Player;
struct WorldSession {
    Player* player=nullptr; std::vector<WorldPacket> packets;
    Player* GetPlayer(){return player;}
    void SendPacket(WorldPacket* p){packets.push_back(*p);}
};
struct State { std::set<std::uint32_t> Known; };
constexpr char StateKey[]="state";
struct Player {
    unsigned native=0,count=1,saves=0,syncs=0; bool area=true,ready=true;
    std::set<unsigned> spells; std::map<std::string,std::vector<Value>> settings;
    WorldSession session; Slots applied{};
    struct Store {State state; template<class T> T* Get(char const*){return &state;}} CustomData;
    Player(){session.player=this;}
    unsigned GetActiveSpec()const{return native;}
    unsigned GetSpecsCount()const{return count;}
    bool HasSpell(unsigned s)const{return spells.contains(s);}
    auto FindPlayerSettings(std::string const& key)const->std::vector<Value> const* {
        auto i=settings.find(key);return i==settings.end()?nullptr:&i->second;
    }
    void UpdatePlayerSetting(std::string const& key,unsigned i,unsigned value){
        auto& v=settings[key];v.resize(std::max(v.size(),std::size_t(i+1)));v[i].value=value;
    }
    WorldSession* GetSession(){return &session;}
    void SaveToDB(bool,bool){++saves;}
};
namespace AscensionFreePick {
''' + spells + '\n}\n' + functions + send + r'''
bool Applies(Player const* p){return p&&p->area;}
bool CanChange(Player* p,State const*){return p->ready;}
void Synchronize(Player* p){p->applied=ReadSlots(p);++p->syncs;}
''' + handle + r'''
int main(){
    for(unsigned total=1;total<=20;++total){
        Player p;
        p.UpdatePlayerSetting("core.freepick.mystic.0",0,123);
        for(unsigned i=0;i<total;++i)p.spells.insert(AscensionFreePick::SpecializationSpells[i]);
        assert(PresetCount(&p)==total);
        SendPresets(&p);
        auto const& packet=p.session.packets[0];
        assert(packet.opcode==0x5FF&&packet.data[0]==total);
        assert(packet.data.size()==1+total*18&&packet.data[2]==123);
        assert(ActivePreset(&p)==0&&ReadSlots(&p)[0]==123);
        p.UpdatePlayerSetting("core.freepick.mystic."+std::to_string(total-1),1,456);
        auto saved=p.settings;
        WorldPacket request(0x60C);request<<std::uint32_t(total-1);
        assert(Handle(&p.session,request));
        assert(ActivePreset(&p)==total-1&&p.applied[1]==456&&p.saves==1);
        for(auto const& [key,values]:saved){
            assert(p.settings[key].size()==values.size());
            for(unsigned i=0;i<values.size();++i)assert(p.settings[key][i].value==values[i].value);
        }
        Player login;login.settings=p.settings;login.spells=p.spells;
        assert(ActivePreset(&login)==total-1&&ReadSlots(&login)[1]==456);
        request.data[0]=total;Handle(&p.session,request);
        assert(p.saves==1&&ActivePreset(&p)==total-1);
        request.data[0]=0;p.ready=false;Handle(&p.session,request);
        assert(p.saves==1&&ActivePreset(&p)==total-1);
        request.data.clear();Handle(&p.session,request);assert(p.saves==1);
        p.area=false;assert(!Handle(&p.session,request));
    }
    Player legacy;legacy.native=1;legacy.count=2;
    legacy.UpdatePlayerSetting("core.freepick.mystic.1",0,789);
    assert(PresetCount(&legacy)==2&&ReadSlots(&legacy)[0]==789);
    legacy.spells.insert(AscensionFreePick::SpecializationSpells[19]);
    assert(PresetCount(&legacy)==2);
}
'''
    assert 'AscensionCompatOpcodes::Claim(0x60C, Handle);' in text
    learn = text[text.index('void OnPlayerLearnSpell'):text.index('void OnPlayerForgotSpell')]
    assert 'SpecializationSpells' in learn and 'SendPresets(player)' in learn
    with tempfile.TemporaryDirectory() as directory:
        source = Path(directory) / 'check.cpp'
        binary = Path(directory) / 'check'
        source.write_text(code)
        subprocess.run(['g++', '-std=c++20', '-Wall', '-Wextra', '-Werror', str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    print('Native preset storage, packet layout, 20 unlock levels, selection, reload and refusal checks passed')


if __name__ == '__main__':
    main()
