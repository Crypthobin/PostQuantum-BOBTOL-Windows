# Post-Quantum BOB-TOL
BOB-TOL is Based On Bitcoin, Toward Outlasting any Legacy.<br>
BOB-TOL is the safest and fastest cryptocurrency that can respond to quantum computers.
## Reference ##
- Website: (...)
- Whitepaper: (...)
## Guidelines ##
### Cloning a repository ###
```
git clone https://github.com/Crypthobin/bitcoin-pqc
```
### Set Data directory ###
- Create a "data" folder under the "bitcoin-pqc" folder.
- Copy and paste "bitcoin-pqc\Src\bitcoin-pqc\share\examples\bitcoin.conf" into path "C:\Users\[user name]\AppData\Roaming\Bitcoin".
- Add the contents below to "C:\Users\[user name]\AppData\Roaming\Bitcoin\bitcoin.conf".
```
# Options only for pqcnet
[pqcnet]
datadir=C:\...\bitcoin-pqc\data # This is the path of the data folder you created.
```
### Build (Windows) ###
- Visual Studio 2019  and higher.
- Linux here: https://github.com/Crypthobin/PostQuantum-BOBTOL-Windows.git
#### 1) Run msvc-autogen.py ####
- Copy the configuration file and create additional project files.
```
cd bitcoin-pqc\Src\bitcoin-pqc\build_msvc
py -3 msvc-autogen.py
```
#### 2) Installing an external library through vcpkg. ####
```
cd ~
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
```
- Set the environment variable for vcpkg.exe path.
- Restart the terminal and link it to use vcpkg in Visual Studio.
```
vcpkg integrate install
```
#### 3) Build ####
- Set "bobtold" as Startup Project.
- Build  in x64Debug mode in Visual Studio.
### Start PQCnet ###
```
cd bitcoin-pqc\Build\x64Debug
bobtold
```
