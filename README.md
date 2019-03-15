Set up the build environment:
```
$ docker build -t simobuild .
$ ./scripts/run_build_env.sh   # run from root folder
```

Building (in the container):
```
$ cd /simos
$ meson setup build/ --cross-file crossfile.txt
$ ninja -C build/
```
