from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[3]
valid = """START TRANSACTION;
SELECT `id` FROM `example` WHERE `id` = 1 FOR UPDATE;
UPDATE `example` SET `name` = CONVERT(0x41 USING utf8mb4) WHERE HEX(CAST(JSON_OBJECT('id', `id`) AS CHAR CHARACTER SET utf8mb4)) = '01';
SELECT COUNT(*) FROM `example`;
COMMIT;
"""
with tempfile.TemporaryDirectory() as temporary:
    folder = Path(temporary) / 'data/sql/updates/pending_db_world'
    folder.mkdir(parents=True)
    path = folder / 'rev_20261023000000.sql'
    for text, succeeds in [(valid, True), (valid.replace('`example`', 'example'), False),
                           ("INSERT INTO `example` (`id`) VALUES (1);\n", False)]:
        path.write_text(text)
        result = subprocess.run([sys.executable, str(root / 'apps/codestyle/codestyle-sql.py'),
                                 '--files', str(path)], capture_output=True, text=True, cwd=root)
        assert (result.returncode == 0) == succeeds, result.stdout + result.stderr
print('SQL guarded syntax passes; unquoted identifiers and unsafe inserts remain rejected')
