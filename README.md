# CVE-2007-4559 - Polemarch exploit 
Full working exploit for Polemarch server https://www.polemarch.org/, using CVE-2007-4559, allowing:

- Injection of a cryptominer for monero using TOR for better hiding.
- Injection of a rootkit targeting most common sysadmin tools.


## POC
Proof of concept using https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2007-4559 
and based on the work from https://www.trellix.com/en-us/about/newsroom/stories/research/tarfile-exploiting-the-world.html.

### Directories to attack
We want to be able to deploy the following files into a Polemarch server:

- /usr/local/lib
  - -> **rootkit.so**: library to do the preloading
- /etc
  - -> **ld.so.preload**: loader configuration file
- /opt/polemarch/bin
  - -> **polemarchctl**: infected docker entry point
- /usr/bin
  - -> **xmring**: miner
  - -> **tor**: tor
  - -> **shellcode**: obfuscated shellcode responsible for starting the miner and tor

Note: in the real exploit we would change some names to avoid detection.

### Notes
- Binary files MUST have executable permissions
- Tar's can not be deployed twice
- Malware will be placed in an auxiliar folder which will be symlinked to target folder

## Instructions
In order to deploy the malware we will have to set up the following:

- Compilation of necessary binaries
- Set up Polemarch server and malicious server hosting infected tar
- Upload infected tar into Polemarch server

## Compile binaries

We will need to compile the rootkit shared library and the shellcode:

```bash
$(rootkit): make
$(shellcode): gcc -z execstack -m32 -o shellcode shell.c
```

## Polemarch server and malicious server

We have created a docker-compose which will:

- Set up a vulnerable Polemarch server at http://localhost:8080
- Generate an infected tar with all the necessary files
- Set up an auxiliary python server hosting the malicous tar at http://energyplus.com/project.tar.gz (only accessible from the docker)

```bash
$: docker compose build 
$: docker compose up -d
```

## Upload infected tar
Log in to the polemarch server using default password (admin/admin) and
create a project with the compromised tar file:

![Peek 2023-01-10 20-55](https://user-images.githubusercontent.com/60936394/211649584-cd5ca7e1-f42c-4c96-8787-155b54519307.gif)

Now, next time the docker is `restarted`, the miner and tor will be executed into the polemarh server.

## Disclaimer
This project is for research purposes only and should not be used for any illegal or malicious activities. The creators of this project are not responsible for any harm or damage caused by the misuse of this software.
