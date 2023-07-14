FROM wiiuenv/devkitppc:20230621

COPY --from=wiiuenv/libkernel:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20230108 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20230225 /artifacts $DEVKITPRO

WORKDIR project
