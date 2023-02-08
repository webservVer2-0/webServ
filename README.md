# webserv
42 seoul webserv server project

## Out-line 
- 2023.01.01 ~ 2023.02.08 study Server, HTTP, Kevent ...

## Goal 
- HTTP/ 1.1 을 준수하는 웹서버 구현
- GET, HEAD, DELETE, POST 메서드 통신이 가능하도록 구현
- Nginix 프로그램을 기반으로 유사한 동작 방식을 구현하기 
- 멀티 포트, 가상 호스팅 기능을 지원하라 
- 완벽한 정적 웹 사이트를 호출할 수 있는 기능을 설정하라. 
- 소켓 프로그레밍, 비동기, 비봉쇄에 대한 온전한 이해를 진행하라.

## 기능 구현요소 정리 

### Mandatory
- 멀티포트, 멀티 서버(가상 호스트)구현하기 
- IO Multiplexing 을 구현하시오
- slect 와 이에 준하는 함수를 사용하여 작성하며, 이벤트 기반으로 IO가 일어나는 로직이 되어 있어야 한다. 
- 따라서 모든 소켓 상의 read, recv, write, send를 검색하고 에러가 반환되면 클라이언트가 제거가 되었는지를 검사하십시오. 
- errno를 검사하는 방식으로 구현되어선 안되며, 그러나 에러 케이스에 대한 적절한 처리가 되어야 한다. 

### Configuration 
- 멀티 포트, 멀피 호스트 지원이 되어야 한다. 
- 기본 에러 페이지를 설정한다.
- 다른 호스트 네임으로 멀티 서버를 설치해보아라
- client 바디를 제한하라 
- 다른 디렉토리들로 서버 상에 루트를 설정할 수 있어야 한다. 
- 기본파일 설정 
- 특정 루트에서 수용 가능한 메서드를 추가 지정이 가능하다. (권한 형태로 만들어라)

### Basic feature 
- HTTP/1.1 기반으로 메소드를 적절하게 기능하도록 만들어라 
	- GET
	- POST
	- DELETE
	- UNKNOWN
- 모든 테스트 도안 상태 코드들은 적절하게 나와야 한다. 
- 어떤 파일을 서버로 업로드 해야 하며, 이를 다시 다운로드 받을 수 있어야 한다. 

### Browser feature 
- 지정한 브라우저를 기반으로 작동할 수 있도록 만들어야 한다. 
- 요청 헤더와 대응헤더가 적절한지를 판단한다. 
- 완벽한 '정적'웹 사이트에 호환되어야 하낟. 
- 디렉토리 리스트 기능이 구현되어 있어야 한다. 
- 리다이렉션 기능이 구현되어야 한다. 

### Stress Test standard
- Siege 를 사용해 스트레스 테스트를 실행하고 가용성 99.5% 를 넘겨야 한다. 
- 메모리 누수가 없어야 하며, 메모리 사용량이 점점 증가하는 것을 최소화 해야 한다. 
- 커넥션이 정상적으로 유지되며 종료되는 것이 없는지 확인하라 
- 서버 재시작이 되는 일들이 발생하지 않고, siege -b 를 알아봐야 한다. 

### Bonus feature 
- 작업 세션, 그리고 쿠키 시스템이 구현되어 있어야 한다. 
- CGI 기능이 구현되어서, 한 개 이상 존재 해야 한다. 

## 컨벤션 
Co-Work Flow
프로젝트에서 이슈 생성
이슈 develop에서 분기하여 브런치 생성
코드 작업
develop로 PR
code review
Merge
위 작업 반복.

Code Convention
google c++ style guide를 따른다.

code style
- reference와 포인터 모두 자료형에 붙혀서 작성.
⭕️ : int& foo;
❌ : int & bar; or int &bar;
- return 괄호 없이
- void 반환 함수 일 때, return 사용 필수!
- tab 간격은 4칸.
- doxygen (vscode extension) 을 사용하여 작성한다.
- 주석은 함수 내부 말고 함수 선언 위에 작성할 것.
- 클래스 생성 시 public, protected, private 순서로 작성
- define 말고 enum 사용하기
- 일반변수는 snake case 
- 멤버변수는 snake case. 하지만 끝에 언더바 붙힘. (Ex. int this_is_example_)
- 모든 함수는 Pascal case. (Ex. void Example_Fun)
int num; // 일반변수 snake case

class example {

  int test_; // 멤버변수 snake case + _

  void Example_Function(); // 함수는 Pascal case

}
Commit Conventions
commit의 기준
commit은 아래 커밋 타입에 맞게 commit들을 분리한다.
commit의 타입
FEAT: 기능을 추가 또는 수정
ENV: 개발 환경을 추가 또는 수정 (eslint 변경, dockerfile 변경 등)
FIX: 버그를 해결
DOCS: 문서를 수정 (README.md 변경, swagger)
STYLE: 코드 스타일 변경 (prettier, npm run lint 등)
REFACT: 코드를 리팩토링, 기능은 같고 코드가 변경
TEST: 테스트 코드를 추가 또는 수정
MERGE: 풀 리퀘스트 머지할 경우
CHORE: 단순오타
WIP: working in process 아직 작업중인 내용
commit 예시
ex)
FIX: 모델 validation 오류 수정
- Book title 제목 default 값 추가
- User intra 최소 길이 0으로 변경

ex)
FEAT: 로그인 기능 추가
- auth/ api 추가

ex)
TEST: bookController 테스트 코드 추가 
- 책 제목에 대한 유효성 테스트 추가
Branch Convention
master(main): 배포된 가장 최신 브랜치입니다.
develop: master에 issue브랜치가 머지되기 전에 거쳐가는 브랜치로, issue 브랜치에서 작업한 결과물이 develop 브랜치로 머지됩니다.
{issue 번호}-{issue title} : 작업이 이뤄지는 이슈의 브런치 (깃허브에서 자동생성해주는 이름 사용)