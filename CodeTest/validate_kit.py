"""생성물의 코드 일치, 로컬 링크, JavaScript 문법을 검사한다. 브라우저 UI 검사는 아니다."""
from pathlib import Path
from html.parser import HTMLParser
import json
import html
import re
import shutil
import subprocess

root=Path(__file__).resolve().parent
page=(root/'index.html').read_text(encoding='utf-8')
data=re.search(r'const records=(.*?);let selected=',page,re.S)
records=json.loads(data[1])
assert len(records)==26
for item in records:
    assert (root/'examples'/item['filename']).read_text(encoding='utf-8-sig')==item['full']
    assert item['block'] in item['full']
    assert item['hooks'] in item['full']
    assert item['settings'] and '[수정]' in item['settings']
    assert '[조합 카드]' in item['guide']
    assert len(re.findall(r'\bint main\(',item['full']))==1
    for hook in ('Reset','Key','Mouse','Cursor','Update','Draw','Check'):
        assert len(re.findall(r'\bvoid On'+hook+r'\(',item['full']))==1

class Links(HTMLParser):
    def handle_starttag(self,tag,attrs):
        for key,value in attrs:
            if key in ('href','src') and value and not value.startswith('#'):
                assert (root/value).is_file(), value
Links().feed(page)
static=(root/'offline.html').read_text(encoding='utf-8')
assert '<script' not in static.lower()
Links().feed(static)
for item in records:
    assert html.escape(item['full']) in static
    assert f'id="feature-{item["number"]}"' in static
for path in root.glob('*.md'):
    for link in re.findall(r'\]\(([^)]+)\)',path.read_text(encoding='utf-8-sig')):
        if not link.startswith(('http:','https:','#')):
            assert (root/link).is_file(), (path.name,link)
script=re.search(r'<script>(.*?)</script>',page,re.S)[1]
checkdir=root/'.validation';checkdir.mkdir(exist_ok=True)
js=checkdir/'index-script.js';js.write_text(script,encoding='utf-8')
node=shutil.which('node')
if not node:
    raise RuntimeError('JavaScript 문법 검사용 node가 없습니다.')
subprocess.run([node,'--check',str(js)],check=True)
search_function=re.search(r'function findMatches\(query\)\{[^\n]+',script)[0]
search_test=checkdir/'search-check.js'
search_test.write_text('const records='+json.dumps(records,ensure_ascii=False)+';\n'+search_function+'''
const assert = require('node:assert/strict');
assert.deepEqual(findMatches('공전').map(r=>r.number), ['13','15','23','25']);
assert.deepEqual(findMatches('  공전  ').map(r=>r.number), ['13','15','23','25']);
assert.equal(findMatches('없는기능xyz').length, 0);
assert.equal(findMatches('').length, 26);
assert.equal(findMatches('camera').length, 2);
assert.deepEqual(findMatches('PlanetSize').map(r=>r.number), ['13','25']);
assert.deepEqual(findMatches('OrbitRadius').map(r=>r.number), ['13','25']);
assert.deepEqual(findMatches('SnapDistance').map(r=>r.number), ['07','26']);
console.log('PASS: orbit search, whitespace, empty results, reset, English case; static offline codes present.');
''',encoding='utf-8')
subprocess.run([node,str(search_test)],check=True)
print('PASS: 26 annotated sources and hooks match files; setting search, local links and JavaScript syntax passed.')
