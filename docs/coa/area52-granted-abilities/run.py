from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
source = r"""
#include "Area52CertificationRules.h"
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <cassert>
int main()
{
    using namespace Area52Certification;
    std::istringstream input(R"({"schema":1,"entries":[
        {"kind":"advancement","id":1,"status":"VERIFIED"},
        {"kind":"mystic","id":1,"status":"VERIFIED"},
        {"kind":"granted","id":1,"ranks":[1,2],"status":"VERIFIED"},
        {"kind":"granted","id":3,"ranks":[3],"status":"NOT VERIFIED"}]})");
    boost::property_tree::ptree report;
    boost::property_tree::read_json(input, report);
    auto entries = ReadEligibility(report);
    assert(entries.size() == 3);
    assert(entries.contains(1) && entries.contains(MysticEntryMask | 1));
    assert(entries.contains(GrantedEntryMask | 1) && !entries.contains(GrantedEntryMask | 3));
    auto ranks = ReadGrantedRanks(report);
    assert(ranks.at(GrantedEntryMask | 1) == std::vector<std::uint32_t>({1,2}));
    assert(ParseConfirmation("C|1073741825|7") == (GrantedEntryMask | 1));
    auto bad = report;
    auto& rows = bad.get_child("entries");
    auto it = rows.begin(); ++it; ++it;
    it->second.put("id", GrantedEntryMask);
    bool rejected = false;
    try { ReadEligibility(bad); } catch (std::runtime_error const&) { rejected = true; }
    assert(rejected);
    bad = report;
    auto& row = (++(++bad.get_child("entries").begin()))->second;
    row.put("id", 100);
    rejected = false;
    try { ReadGrantedRanks(bad); } catch (std::runtime_error const&) { rejected = true; }
    assert(rejected);
}
"""
with tempfile.TemporaryDirectory() as directory:
    out=Path(directory)
    (out/'test.cpp').write_text(source)
    subprocess.run(['c++','-std=c++20','-I'+str(root/'src/server/coa'),str(out/'test.cpp'),'-o',str(out/'test')],check=True)
    subprocess.run([str(out/'test')],check=True)
print('Granted certification IDs, rank families and malformed ledger validation passed')
