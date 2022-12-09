We had discovered a Python tarfile vulnerability called CVE-2007-4559: Extracting a crafted TAR archive with the tarfile.extract or tarfile.extractall functions could lead to a directory traversal vulnerability, resulting in overwrite of arbitrary files.

We wanted to use this vulnerabilty to earn money via cryptojacking so we started to search vulnerable applications. We found that some versions of Polemarch, a web application for managing servers, are vulnerable to this attack.

Knowing all this, we started looking for companies that had a Polemarch server to attack. Using Linkedin, we found some people with experience using the application. We found a user who was using Polemarch at his current job and was also sharing his company email. Doing some research about the company, we also found the name of his boss.

Since to execute the attack we need access to the server, we decided to do a phishing attack to this employee. We created a fake mail using his boss's name to trick the employee into generating a project that points to a server containing the files needed to execute the attack.

Once the project has been created, using the vulnerability, we write in the system the required files to execute the attack. To perform the cryptojacking we are using XMRig, a Monero miner. The server runs in a Docker container so we have overwritten the container entrypoint to execute the required commands every time the cointainer starts.

However, there is an important problem: many firewalls and antivirus know IPs of the most popular mining pools. To avoid this problem, we are using Tor to hidde the IPs.

Finally, to hide all the processes and connections retaled with the cryptojacking, we are using a rootkit. In our case, since we are mining Monero using the CPU, the rootkit also hides the CPU usage.
