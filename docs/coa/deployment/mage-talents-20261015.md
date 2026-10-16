# Deployed Mage talent correction — October 15, 2026

Package mage-talent-targeting-correction-20261015 completed activation with announcement and server readiness checks. The sibling patch is the exact eight-file deployed change. Replay against deployment backups reconstructed all packaged source bytes; apply only to that matching baseline, not unrelated newer source.

Server SHA256: 986c4802622c0c9af0d9003f9c24be0fb7da04274da991bd41aa1fdf32f82649.
Client SHA256: 439a445a9696ec2dac20b1a93f1f01ec5f17364469169e773722cca00362b88e.

Matching compilation/unit checks, 22 source checks and focused production-function offline tests passed. Tests cover modifier orders/removal, Power Surge ranks/schools, damage plus absorbs, completed/interrupted Invocation, mana rounding, death and rank removal. Gameplay certification is not claimed. Three rank descriptions display 2/4/6%; all status labels and unrelated client records are preserved.

This source publication preserves a deployed-baseline patch instead of overwriting newer repository files with unrelated baseline differences. Use tools/verify_all.py with the included mage_talent_targeting harness on the matching deployed source. No main merge is proposed. Investigation timing and pre-fix live measurements are unavailable.
