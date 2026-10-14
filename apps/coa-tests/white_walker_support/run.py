import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]

MAIN = r'''
#include "AscensionWhiteWalkerSupportRules.h"
#include <cassert>
using namespace AscensionWhiteWalkerSupport;
int main()
{
    static_assert(AthleticCostPercent == -4 && AthleticSchools == 17);
    static_assert(WindsOfWinterRanks.size() == 8 && WindsOfWinterRanks.front() == 978846 && WindsOfWinterRanks.back() == 978853);
    static_assert(IceSpearRanks.size() == 7 && IceSpearRanks.front() == 984935 && IceSpearRanks.back() == 984975);
    static_assert(WintersFury == 984936 && WintersVitality == 978525);
    static_assert(FrostbrandDamageHelpers.size() == 9 && FrostbrandDamageHelpers.front() == 965990 && FrostbrandDamageHelpers.back() == 965998);
    static_assert(FrostbrandSlowHelpers.size() == 9 && FrostbrandSlowHelpers.front() == 8034 && FrostbrandSlowHelpers.back() == 58799);
    static_assert(EviscerateClientCoefficient(1) == 2 && EviscerateClientCoefficient(5) == 50);
    assert(HonedIceSource(3, 512, 16, false, 1, true));
    assert(!HonedIceSource(3, 512, 16, true, 1, true));
    assert(!HonedIceSource(3, 512, 16, false, 0, true));
    assert(!HonedIceSource(3, 512, 16, false, 1, false));
    assert(!HonedIceSource(3, 1, 16, false, 1, true));
    assert(!HonedIceSource(3, 512, 4, false, 1, true));
    assert(!HonedIceSource(8, 512, 16, false, 1, true));
    assert(HonedIceCriticalBonus(2, 1) == 2);
    assert(HonedIceCriticalBonus(2, 3) == 6);
    assert(HonedIceCriticalBonus(2, 0) == 0);
    assert(WhiteWalkerActive(true, true, true));
    assert(!WhiteWalkerActive(false, true, true));
    assert(!WhiteWalkerActive(true, false, true));
    assert(!WhiteWalkerActive(true, true, false));
}
'''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dbc-dir')
    parser.parse_args()
    compiler = shutil.which('c++') or shutil.which('g++')
    if not compiler:
        raise SystemExit('UNAVAILABLE: C++ compiler required')
    with tempfile.TemporaryDirectory(prefix='white-walker-support-') as temporary:
        folder = Path(temporary)
        source = folder / 'rules.cpp'
        binary = folder / 'rules'
        source.write_text(MAIN)
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    print('PASS: White Walker support eligibility, source ownership and stack scaling')

if __name__ == '__main__':
    main()
