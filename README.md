## Build instruction
### Linux
- Open folder named csvreader in terminal
- Run cmake -B ./build
- cd build
- make

### Windows
Required MinGW toolchain with posix thread model!
- Open folder named csvreader in terinal
- Run cmake -G "MinGW Makefiles" -B ./build
- cd build
- Run mingw32-make

## Usage
- ./csvreader "table name".csv

