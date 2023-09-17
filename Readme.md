# Trading Platform
A simple trading platform that simulates clients and exchages. The intent is to provide client connectivity, message handling (binary/fix), simple matching / crossing engines, level order books, market data handlers etc. Project dependencies are resolved by vcpkg. 

## Build Instructions :
### External build dependencies for building from source
| Plugin | README |
| ------ | ------ |
| CMake | https://cmake.org/ |
| Ninja | [https://ninja-build.org/]|

### Building for source
```sh
git clone --recursive https://github.com/pashnidgunde/TradingPlatform.git
cmake --list-presets
cmake --preset=<Preset of your choice>
cd build/<target>
ninja
```

### Docker
```sh
docker build --pull --rm -f "Dockerfile" -t tradingplatform:latest "." 
```

### For debugging docker builds
```sh
docker build --pull -f "Dockerfile" -t tradingplatform:latest "." 
docker run -it tradingplatform
```