Set up the build environment:
```
$ docker build -t simobuild .
$ docker run --rm --mount type=bind,source="$(pwd)",target=/simos -i simobuild:latest /bin/sh
```

Building (in the container):
```
$ cd /simos
$ meson setup build/ --cross-file crossfile.txt
$ ninja -C build/
```