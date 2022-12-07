
# Start xmrig
First, copy the default config.json:

```bash
cp config.json.default config.json
```

The, you can start the cryptominer in two different ways:

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
# Build static Tor

https://docs.j7k6.org/tor-static-build/
