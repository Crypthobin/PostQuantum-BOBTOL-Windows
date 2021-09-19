# bitcoin-pqc
post quantum으로 교체 작업 중<br>
**자세한 내용은 notion의 "소스코드 작성 일지" 참고:smile:** 
## init ##
- "bitcoin-pqc" 폴더 아래에 "data" 폴더 생성
- "\bitcoin-pqc\share\examples\bitcoin.conf"를 복사하여 "C:\Users\사용자 이름\AppData\Roaming\Bitcoin" 경로에 붙여넣기
- "C:\Users\사용자 이름\AppData\Roaming\Bitcoin\bitcoin.conf" 아래에, 아래 내용 추가
```
# Options only for pqcnet
[pqcnet]

datadir=C:\Users\..\bitcoin-pqc\data #ex.
```
## build ##
- Debug x64
- 빌드 시, "bitcoin-pqc\Build\x64Debug" 경로에 실행파일 생성 <br>
**기본 설정법은 notion의 "윈도우에서 비트코인 빌드 방법" 참고:smile:** 
## usage ##
- PQCnet 서버 실행
```
bitcoind -pqcnet
```
