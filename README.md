# Aroma plugin that fixes the RCE bugs on WiiU games

- Mario Kart 8 (All regions, v64)
	- [ENLBufferPwn](https://github.com/PabloMK7/ENLBufferPwn) fix
	- Identification token parsing RCE fix (exploit found by Kinnay)
	- ENL nullptr deref fix + OOB read

- Splatoon (All regions, v272)
	- [ENLBufferPwn](https://github.com/PabloMK7/ENLBufferPwn) fix
	- ENL nullptr deref fix + OOB read

## Buildflags

### Logging
Building via `make` only logs errors (via OSReport). To enable logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule) set `DEBUG` to `1` or `VERBOSE`.

`make` Logs errors only (via OSReport).  
`make DEBUG=1` Enables information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).  
`make DEBUG=VERBOSE` Enables verbose information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).

If the [LoggingModule](https://github.com/wiiu-env/LoggingModule) is not present, it'll fallback to UDP (Port 4405) and [CafeOS](https://github.com/wiiu-env/USBSerialLoggingModule) logging.

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t rcepatches-builder

# make 
docker run -it --rm -v ${PWD}:/project rcepatches-builder make

# make clean
docker run -it --rm -v ${PWD}:/project rcepatches-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src wiiuenv/clang-format:13.0.0-2 -r ./source -i`