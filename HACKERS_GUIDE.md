# Node.js Compiler Hackers' Guide

**Contents**

* [Upgrading libsquash](#upgrading-libsquash)
* [Upgrading libautoupdate](#upgrading-libautoupdate)
* [Upgrading the Node.js runtime](#upgrading-the-nodejs-runtime)

## Upgrading libsquash

    cd ..
    git clone git@github.com:pmq20/libsquash.git
    cd node-packer
    rm -rf node/deps/libsquash && cp -R ../libsquash node/deps/libsquash && rm -rf node/deps/libsquash/.git && rm node/deps/libsquash/.gitignore && rm node/deps/libsquash/sample/enclose_io_main.c && mv node/deps/libsquash/sample/enclose_io_libsquash.gyp node/deps/libsquash/enclose_io_libsquash.gyp

## Upgrading libautoupdate

    cd ..
    git clone git@github.com:pmq20/libautoupdate.git
    cd node-packer
    rm -rf node/deps/libautoupdate && cp -R ../libautoupdate node/deps/libautoupdate && rm -rf node/deps/libautoupdate/.git && rm node/deps/libautoupdate/.gitignore

## Upgrading the Node.js runtime

Suppose that you are upgrading from v8.3.0 to v8.4.0.

### Download the old source and inflate it

    wget https://nodejs.org/dist/v8.3.0/node-v8.3.0.tar.gz
    tar xzf node-v8.3.0.tar.gz
    
### Download the new source code and inflate it

    wget https://nodejs.org/dist/v8.4.0/node-v8.4.0.tar.gz
    tar xzf node-v8.4.0.tar.gz

### Overwrite node/ with the old code

    rm -rf node/
    mv node-v8.3.0 node/
    rm -rf node/deps/libsquash && cp -R ../libsquash node/deps/libsquash && rm -rf node/deps/libsquash/.git && rm node/deps/libsquash/.gitignore && rm node/deps/libsquash/sample/enclose_io_main.c && mv node/deps/libsquash/sample/enclose_io_libsquash.gyp node/deps/libsquash/enclose_io_libsquash.gyp
    rm -rf node/deps/libautoupdate && cp -R ../libautoupdate node/deps/libautoupdate && rm -rf node/deps/libautoupdate/.git && rm node/deps/libautoupdate/.gitignore
    find node -name .gitignore -exec rm {} \;
    rm -rf node/deps/npm
    git add node/
    git commit -m1

Then record the commit SHA1, say, 1f87d01b.

### Overwrite node/ with the new code

    rm -rf node/
    mv node-v8.4.0 node/
    rm -rf node/deps/libsquash && cp -R ../libsquash node/deps/libsquash && rm -rf node/deps/libsquash/.git && rm node/deps/libsquash/.gitignore && rm node/deps/libsquash/sample/enclose_io_main.c && mv node/deps/libsquash/sample/enclose_io_libsquash.gyp node/deps/libsquash/enclose_io_libsquash.gyp
    rm -rf node/deps/libautoupdate && cp -R ../libautoupdate node/deps/libautoupdate && rm -rf node/deps/libautoupdate/.git && rm node/deps/libautoupdate/.gitignore
    find node -name .gitignore -exec rm {} \;
    rm -rf node/deps/npm
    git add node/
    git commit -m2

### Revert the first commit

    git revert 1f87d01b

Then fix conflicts (if any).

### Squash changes

    git reset --soft origin/master
