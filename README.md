# itsalamp
It is what the name says: a lamp. The idea is to be able to control the icon through the stdin, so that a program can display permanent notifications that do not popup.

## Compiling

You will need `gtk+2`. Then run
    mkdir release
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
    sudo make install

This will install the binaries `itsalamp`, `itsacounter` and `itsalarm` to `/usr/local/bin` and the icons to `/usr/local/share/icons/itsalamp`.
