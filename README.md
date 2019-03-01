```
$ docker build -t simobuild .
$ docker run --rm --mount type=bind,source="$(pwd)",target=/simos -i simobuild:latest /bin/sh
```
