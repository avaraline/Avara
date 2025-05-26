#!/usr/bin/env python3

import json
import os
import re
import struct
import subprocess
import sys
import unicodedata
from collections import OrderedDict

from bspt import BSP
from forker import get_forks
from pict2alf import parse_pict
from tmpl import parse_tmpl

SETFILE = "set.json"
ALFDIR = "alf"
BSPDIR = "bsps"
ALFEXT = ".alf"
SCRIPTFILE = "default.avarascript"
OGGDIR = "ogg"
WAVDIR = "wav"
AFTERSHOCKSETS = [
    "levels/aftershock.r",
    "levels/as-00-a.r",
    "levels/as-00-b.r"
]

EXPORT_SOUNDS = True
KEEP_WAV = False
OVERWRITE_ALF = True

def is_exe(path):
    return os.path.isfile(path) and os.access(path, os.X_OK)


if (
    not is_exe(os.path.join("build", "hsnd2wav"))
    and not is_exe(os.path.join("build", "hsnd2wav.exe"))
    and EXPORT_SOUNDS
):
    print("Please build hsnd2wav to use this utility: make hsnd2wav")
    exit(1)


def writebsp(bsppath, bspd):
    bsp = BSP(bspd).avara_format()
    with open(bsppath, "w", encoding="utf-8") as bspf:
        bspf.write(bsp)


def writealf(alfpath, alfd):
    alf = parse_pict(alfd)
    with open(alfpath, "w", encoding="utf-8") as alff:
        alff.write(alf)


ffmpeg_found = False
for path in os.environ["PATH"].split(os.pathsep):
    bin_file = os.path.join(path, "ffmpeg")
    exe_file = bin_file + ".exe"
    if is_exe(exe_file) or is_exe(bin_file):
        ffmpeg_found = True

if not ffmpeg_found and EXPORT_SOUNDS:
    print("Please install ffmpeg to change WAV into OGG")
    exit(1)


def get_default_tmpl():
    with open("rsrc/Avara.r", "rb") as avarar:
        return get_forks(avarar.read())["TMPL"]


def get_tmpl(data, tmpl_tag):
    tmpl_data = [x["data"] for x in data["TMPL"] if x["name"] == tmpl_tag][0]
    return {x["id"]: parse_tmpl(tmpl_data, x["data"]) for x in data[tmpl_tag]}


def remove_accents(input_str):
    nfkd_form = unicodedata.normalize("NFKD", input_str)
    return "".join([c for c in nfkd_form if not unicodedata.combining(c)])


def slugify(text):
    text = remove_accents(text)
    text = re.sub(r"[^a-zA-Z0-9\- ]", "", text)
    text = text.replace("-", " ")
    return "-".join(text.lower().split())


def convert_to_files(datafile, thedir):
    os.makedirs(thedir, exist_ok=True)
    data = open(datafile, "rb").read()

    forks = get_forks(data)
    forks["TMPL"] = get_default_tmpl()

    # print(forks)
    if "LEDI" not in forks:
        print("No LEDI found")
        exit(1)

    rledi = get_tmpl(forks, "LEDI")
    rledi = rledi[list(rledi.keys())[0]]

    picts = {e["name"].lower(): e["data"] for e in forks["PICT"]}
    result = {}

    alfdir = os.path.join(thedir, ALFDIR)
    os.makedirs(alfdir, exist_ok=True)

    result["LEDI"] = []
    # for each level
    for le in rledi["*****"]:
        alfname = slugify(le["Name"]) + ALFEXT
        if alfname == ".alf":
            continue
        alfpath = os.path.join(alfdir, alfname)
        pictk = le["Path"].lower()
        if len(pictk) > 0:
            if pictk not in picts:
                print(f"Skipping {alfpath} - Couldn't find pict '{pictk}'")
                continue
            if os.path.exists(alfpath) and not OVERWRITE_ALF:
                print(f"Skipping {alfpath} - exists")
            else:
                print(alfpath)
                writealf(alfpath, picts[pictk])

        lvlentry = {
            "Alf": alfname,
            "Name": le["Name"],
            "Message": le["Message"].strip(),
        }
        if datafile in AFTERSHOCKSETS:
            lvlentry["Aftershock"] = True
        result["LEDI"].append(lvlentry)

    if "HULL" in forks:
        result["HULL"] = get_tmpl(forks, "HULL")

    if "HSND" in forks and EXPORT_SOUNDS:
        # todo: export ogg and point to new file
        # hsnd = get_tmpl(forks, "HSND")
        result["HSND"] = get_tmpl(forks, "HSND")
        if datafile == "levels/single-player.r":
            oggdir = os.path.join("rsrc", OGGDIR)
        elif datafile == "levels/aftershock.r":
            oggdir = os.path.join("rsrc", "aftershock", OGGDIR)
        else:
            oggdir = os.path.join(thedir, OGGDIR)
        os.makedirs(oggdir, exist_ok=True)
        wavdir = os.path.join(thedir, WAVDIR)
        os.makedirs(wavdir, exist_ok=True)
        for k in result["HSND"].keys():
            oggfile = str(k) + ".ogg"
            wavfile = str(k) + ".wav"
            result["HSND"][k]["Sound"] = 0
            result["HSND"][k]["Ogg"] = oggfile
            result["HSND"][k]["Wav"] = wavfile

            wavpath = os.path.join(wavdir, wavfile)
            oggpath = os.path.join(oggdir, oggfile)
            if os.path.exists(oggpath):
                print(f"Skipping {oggpath} - exists")
                continue

            if not os.path.exists(wavpath):
                args = [f"build{os.path.sep}hsnd2wav", str(k), wavpath, str(datafile)]
                popen = subprocess.Popen(args, stdout=subprocess.PIPE)
                popen.wait()

            args = ["ffmpeg", "-y", "-i", wavpath, "-acodec", "libvorbis", oggpath]
            popen = subprocess.Popen(args, stdout=subprocess.PIPE)
            popen.wait()

            if not KEEP_WAV:
                os.remove(wavpath)

    if "BSPT" in forks:
        result["BSPT"] = {}
        rbsps = get_tmpl(forks, "BSPT")
        if datafile == "levels/single-player.r":
            bspspath = os.path.join("rsrc", BSPDIR)
        elif datafile == "levels/aftershock.r":
            bspspath = os.path.join("rsrc", "aftershock", BSPDIR)
        else:
            bspspath = os.path.join(thedir, BSPDIR)
        os.makedirs(bspspath, exist_ok=True)
        for k in rbsps.keys():
            bspname = str(k) + ".json"
            bsppath = os.path.join(bspspath, bspname)
            result["BSPT"][k] = bsppath
            if os.path.exists(bsppath):
                print(f"Skipping {bsppath} - exists")
                continue
            print(bsppath)

            bspd = [x["data"] for x in forks["BSPT"] if x["id"] == k][0]

            datapath = os.path.join(bspspath, str(k))
            with open(datapath, "wb") as datafile:
                datafile.write(bspd)

            args = ["bin/bspt.py", datapath]
            popen = subprocess.Popen(args, stdout=subprocess.PIPE)
            bsp, err = popen.communicate()

            with open(bsppath, "w") as bspfile:
                bspfile.write(bsp.decode("utf-8"))

            try:
                os.remove(datapath)
            except FileNotFoundError:
                pass

    if "BSPS" in forks:
        try:
            result["BSPS"] = get_tmpl(forks, "BSPS")
        except struct.error:
            print("Bad BSPS data")

    dumpdirs = [os.path.join(thedir, SETFILE)]
    if datafile == "levels/single-player.r":
        dumpdirs.append(os.path.join("rsrc", SETFILE))
    elif datafile == "levels/aftershock.r":
        dumpdirs.append(os.path.join("rsrc", "aftershock", SETFILE))
    for d in dumpdirs:
        with open(d, "w", encoding="utf-8") as setfile:
            setfile.write(json.dumps(result, indent=2, ensure_ascii=False))
            setfile.write("\n")


    if "TEXT" in forks:
        text = "".join([e["data"].decode("macroman") for e in forks["TEXT"]])
        text = text.replace("\r", "\n")
        text = re.sub(r"ambient(\s*)=", "ambient.i\1=", text)
        textpath = os.path.join(thedir, SCRIPTFILE)
        if datafile == "levels/single-player.r" or datafile == "levels/aftershock.r":
            textpath += '.ignore'
        with open(textpath, "w", encoding="utf-8") as textfile:
            textfile.write(text)
    # print(forks);


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("usage: rsrc2files.py <levelset.r> <result directory>")
        sys.exit(1)
    else:
        convert_to_files(sys.argv[1], sys.argv[2])
