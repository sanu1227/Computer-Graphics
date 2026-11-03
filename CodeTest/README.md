# CodeTest

**먼저 아래 두 파일만 열면 됩니다.**

- [기능모음집](Markdown/기능모음집.md): 기능 번호·검색어·복붙 코드.
- [사용 참고서](Markdown/OpenGL_기능별_통합참고서.md): 같은 번호의 붙일 위치·인자 순서와 의미·수정할 값·조합 설명.

[처음 사용](Markdown/기본코드.md) · [기능 조합](Markdown/기능_조합.md) · [base.cpp](base.cpp)

| 찾는 기능 | 검색 번호 |
|---|---|
| 상자·받침대·판 | D06 |
| 격자무늬 바닥 | D10 |
| 두 색이 번갈아 채워지는 체커 바닥 | D11 |
| 판과 손잡이 함께 회전 | T04 |
| 고른 객체만 X·Y·Z축 회전·시작·정지 | T08 |
| 자동 왕복 기울기 | A02 |
| 드래그와 스냅 | I04 + C03 |
| 점프·착지 | C04 / C05 |
| 공전·위성 | T06 / T07 |
| 카메라·투영·화면 분할 | P01 ~ P05 |

## 파일 구성

- Markdown: 기능모음집 1개와 사용 설명 문서 11개. 기능 번호는 실습 문제 번호와 다릅니다.
- base.cpp: 이 모음집의 기본 코드. basic.cpp는 도우미를 직접 추가하는 최소 바탕입니다.
- blocks/examples: 기존 기능 블록 22개와 완성 예제 26개.
- archives: 이전 상세 참고서와 압축 파일. 이전 설명이 필요할 때만 엽니다.
- OpenGL_기능별_MD_참고서.zip: 최신 Markdown, base.cpp, basic.cpp, 시작 안내를 포함한 휴대용 묶음.
- feature_catalog.py / make_markdown.py: 코드와 설명의 원본 및 생성기.
- validate_catalog.py: 기능 코드 연결·논리·숨김 OpenGL 검사 준비 도구.
- HTML 및 이전 GUIDE/INDEX/COMBINE/QUICK_REFERENCE 문서는 이전 자료로 보관합니다.

시험 PC에 C++17 이상/x64/GLFW/GLEW/GLM과 링크 라이브러리를 미리 준비합니다.
인터넷 없이 문서를 열고 코드를 복사할 수 있습니다. cpp는 main이 있는 파일 하나만 빌드합니다.
