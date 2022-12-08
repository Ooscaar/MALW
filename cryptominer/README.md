
# Cryptominer and TOR

## xmrig
First, copy the default config.json:

```bash
cp config.json.default config.json
```

Then, you can start the cryptominer in two different ways:

Using the `config.json` file:
```
./xmrig
```

Without using the `config.json` file:
```
./xmrig \
    --url hashvaultsvg2rinvxz7kos77hdfm6zrd5yco3tx2yh2linsmusfwyad.onion \
    --proxy=localhost:9050 \
    --user 41yjBrEhDeThnyvxhbiC3rQwcJtV3X1Zy8s9uW4NKUxVKaH59C4XyuW72D1DeLqm7oSGunxpu4WgNawx1FqWc1deCAZTauo \
    --pass x \
    --donate-level 1 \
    --tls \
    --tls-fingerprint 420c7850e09b7c0bdcf748a7da9eb3647daf8515718f36d9ccfdd6b9ff834b14
```

## Tor
Note: the binaries are compiled for `x86_64` architecture.

```bash
./tor
```

## Build static Tor

```bash
# Versions
TOR_VERSION="0.4.7.12"
ZLIB="zlib-1.2.13"
OPENSSL="openssl-1.1.1k"
LIBEVENT="2.1.12-stable"

# Download and extract versions

## ZLIB
curl -fsSL "https://zlib.net/${ZLIB}.tar.gz" | tar zxvf -

## Compile and install ZLIB
cd ${ZLIB}
./configure --prefix=$PWD/install
make -j$(nproc)
make install
cd ..


## OpenSSL
curl -fsSL "https://www.openssl.org/source/${OPENSSL}.tar.gz" | tar zxvf -

## Compile and install OpenSSL
cd ${OPENSSL}
./config --prefix=$PWD/install no-shared no-dso
make -j$(nproc)
make install
cd ..

## Libevent
curl -fsSL "https://github.com/libevent/libevent/releases/download/release-${LIBEVENT}/libevent-${LIBEVENT}.tar.gz" | tar zxvf -

cd libevent-${LIBEVENT}
./configure --prefix=$PWD/install \
           --disable-shared \
           --enable-static \
           --with-pic
make -j$(nproc)
make install
cd ..

## Tor
curl -fsSL "https://dist.torproject.org/tor-${TOR_VERSION}.tar.gz" | tar zxvf -

cd tor-${TOR_VERSION}

## TODO: remove not needed libaries and files
./configure --prefix=$PWD/install \
            --enable-static-tor \
            --with-libevent-dir=$PWD/../libevent-${LIBEVENT}/install --enable-static-libevent \
            --with-openssl-dir=$PWD/../${OPENSSL}/install --enable-static-openssl \
            --with-zlib-dir=$PWD/../${ZLIB}/install --enable-static-zlib \
make -j$(nproc)
make install
```

Binaries are located in `tor-${TOR_VERSION}/install/bin/`

Based on:
- https://docs.j7k6.org/tor-static-build/
