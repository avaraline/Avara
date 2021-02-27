#!/usr/bin/env python3

import sys
import os
import json
import subprocess
from forker import get_forks
from tmpl import parse_tmpl
from pict2alf import parse_pict
from bspt import BSP

SETFILE = "set.json"
ALFDIR = "alf"
BSPDIR = "bsps"
ALFEXT = ".alf"
SCRIPTFILE = "default.avarascript"
OGGDIR = "ogg"

def is_exe(path):
    return os.path.isfile(path) and os.access(path, os.X_OK)

if not is_exe(os.path.join("build", "hsnd2wav")) and \
   not is_exe(os.path.join("build", "hsnd2wav.exe")):
    print("Please build hsnd2wav to use this utility: make hsnd2wav")
    exit(1)

sndfile_convert_found = False
for path in os.environ["PATH"].split(os.pathsep):
    bin_file = os.path.join(path, "sndfile-convert") 
    exe_file = bin_file + ".exe"
    if is_exe(exe_file) or is_exe(bin_file):
        sndfile_convert_found = True

if not sndfile_convert_found:
    print("Please install sndfile-convert to change WAV into OGG")
    exit(1)


def get_default_tmpl():
    with open("rsrc/Avara.r", "rb") as avarar:
        return get_forks(avarar.read())["TMPL"]

def get_tmpl(data, tmpl_tag):
    tmpl_data =  [x["data"] for x in data["TMPL"] if x["name"] == tmpl_tag][0]
    return {x["id"]:parse_tmpl(tmpl_data, x["data"]) for x in data[tmpl_tag]} 


def sane_filename(string):
    return "".join([c for c in string if c.isalpha() or c.isdigit()]).rstrip()

def convert_to_files(datafile, thedir):
    os.makedirs(thedir, exist_ok=True)
    data = open(datafile, 'rb').read()

    forks = get_forks(data)
    forks["TMPL"] = get_default_tmpl()

    #print(forks)
    if "LEDI" not in forks:
        print("No LEDI found")
        exit(1)

    rledi = get_tmpl(forks, "LEDI")
    rledi = rledi[list(rledi.keys())[0]]

    picts = {e["name"].lower():e["data"] for e in forks["PICT"]}
    result = {}

    alfdir = os.path.join(thedir, ALFDIR)
    os.makedirs(alfdir, exist_ok=True)

    result["LEDI"] = {}
    # for each level
    for le in rledi["*****"]:
        alfname = sane_filename(le["Path"]) + ALFEXT
        alfpath = os.path.join(alfdir, alfname)
        if len(le["Path"]) > 0:
            with open(alfpath, "w", encoding="utf-8") as alff:
                alff.write(parse_pict(picts[le["Path"].lower()]))

        result["LEDI"][le["Tag"]] = {
            "Alf": alfname,
            "Name": le["Name"],
            "Message": le["Message"]
        }

    if "HULL" in forks:
        result["HULL"] = get_tmpl(forks, "HULL")

    if "HSND" in forks:
        # todo: export ogg and point to new file
        hsnd = get_tmpl(forks, "HSND")
        result["HSND"] = get_tmpl(forks, "HSND")
        oggpath = os.path.join(thedir, OGGDIR)
        os.makedirs(oggpath, exist_ok=True)
        for k in result["HSND"].keys():
            oggfile = str(k) + ".ogg"
            result["HSND"][k]["Sound"] = 0
            result["HSND"][k]["Ogg"] = oggfile
            thepath = os.path.join(oggpath, oggfile)
            wavpath = os.path.join(oggpath, str(k) + ".wav")
            args = [f'build{os.path.sep}hsnd2wav', str(k), wavpath, str(datafile)]
            popen = subprocess.Popen(args, stdout=subprocess.PIPE)
            popen.wait()
            args = ['sndfile-convert', '-vorbis', wavpath, thepath]
            popen = subprocess.Popen(args, stdout=subprocess.PIPE)
            popen.wait()
            try:
                os.remove(wavpath)
            except FileNotFoundError:
                pass

    if "BSPT" in forks:
        result["BSPT"] = {}
        rbsps = get_tmpl(forks, "BSPT")
        bspspath = os.path.join(thedir, BSPDIR)
        os.makedirs(bspspath, exist_ok=True)
        for k in rbsps.keys():
            bspname = str(k) + ".json"
            bsppath = os.path.join(bspspath, bspname)
            with open(bsppath, 'w', encoding="utf-8") as bspf:
                bspd = [x["data"] for x in forks["BSPT"] if x["id"] == k][0]
                bspf.write(BSP(bspd).avara_format())

    setpath = os.path.join(thedir, SETFILE)
    with open(setpath, "w", encoding="utf-8") as setfile:
        setfile.write(json.dumps(result, indent=2, sort_keys=True))

    if "TEXT" in forks:
        textpath = os.path.join(thedir, SCRIPTFILE)
        with open(textpath, "w", encoding="utf-8") as textfile:
            textfile.write("".join([e["data"].decode('macroman') 
                                for e in forks["TEXT"]])
                             .replace("\r", "\n"))
    #print(forks);





if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("usage: rsrc2files.py <levelset.r> <result directory>")
        sys.exit(1);
    else:
        convert_to_files(sys.argv[1], sys.argv[2])