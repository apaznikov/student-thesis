# binboost

# Запуск

Из корня:

```sh
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -DBB_ARCH=amd64 -DBB_DISABLE_JIT=OFF ..
make -j
./test/bubble/bubble
```
