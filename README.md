# bitcoin-pqc
post quantum으로 교체 작업 중<br>
**자세한 내용은 notion의 "소스코드 작성 일지" 참고:smile:** 
## init ##
- "bitcoin-pqc\Src\bitcoin-pqc\" 폴더 아래에 "data" 폴더 생성
- "bitcoin-pqc\Src\bitcoin-pqc\share\examples\bitcoin.conf"를 복사하여 "C:\Users\사용자 이름\AppData\Roaming\Bitcoin" 경로에 붙여넣기
- "C:\Users\사용자 이름\AppData\Roaming\Bitcoin\bitcoin.conf" 아래에, 아래 내용 추가
```
# Options only for pqcnet
[pqcnet]

datadir=C:\Users\..\bitcoin-pqc\Src\bitcoin-pqc\data # 본인이 생성한 data 폴더의 경로입니다.
```
## build ##
- Debug x64 <br>

**기본 설정법은 notion의 "윈도우에서 비트코인 빌드 방법" 참고:smile:** 
## usage ##
"bitcoin-pqc\Build\x64Debug" 경로 아래에서, <br>
- PQCnet 서버 실행
```
bitcoind -pqcnet
```
- (예시) 지갑 생성
```
bitcoin-cli -pqcnet createwallet "test_wallet"
```
## 커밋 시 유의사항 ##
- 각자 브랜치 생성
- Src 폴더만 커밋
- ..\bitcoin-pqc\Src\bitcoin-pqc\data 안에 파일들 지워주기<br>
- 혹시 bitcoin-pqc\Src\bitcoin-pqc\build_msvc 이 안에 빌드 산출물이 생긴다면.. 지워주기.. (이부분 더 찾아볼게요 :joy:)
