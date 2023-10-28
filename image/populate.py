import os, sh, re, sys

folders_imported = []

def populate_disk(disk: str):
    for root, dirs, files in os.walk("./image/root"):
        for name in files:
            folder = root.replace("./image/root", "")
            if (folder != "" and folder not in folders_imported):
                folders_imported.append(folder)
                sh.mmd("-i", disk, "::" + folder[1:])

            sh.mcopy("-i", disk, root + "/" + name, "::" + folder[1:].strip(" ") + "/" + name.strip(" "))

if __name__ == "__main__":
    populate_disk(sys.argv[1])
