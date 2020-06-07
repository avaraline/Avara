#!/usr/bin/env python3
from pict2svg import parse_pict, PictParseError
from rsrc_tools import resource
import rsrc_tools.tmpl.reader as tmplReader
import rsrc_tools.bspt.reader as bsptReader
import os
import sys
import json
import subprocess
import time
from pathlib import Path

BSPS_DIR = "bsps"
HSND_DIR = "snd"
SVG_DIR = "svg"

DEBUG_EXPORT = True

EXPORT_SOUND = True


def is_exe(path):
    return os.path.isfile(path) and os.access(path, os.X_OK)

if not is_exe(os.path.join("build", "hsnd2wav")) and \
   not is_exe(os.path.join("build", "hsnd2wav.exe")):
    EXPORT_SOUND = False

WAV2OGG = False
for path in os.environ["PATH"].split(os.pathsep):
    bin_file = os.path.join(path, "sndfile-convert") 
    exe_file = bin_file + ".exe"
    if is_exe(exe_file) or is_exe(bin_file):
        WAV2OGG = True

def parse_level_rsrc(rpath, outpath, tmpl=None):
    manifest_data = {}
    data = open(str(rpath), 'rb').read()

    reader = resource.Reader()
    rsrc = reader.parse(data)
    if tmpl is not None:
        rsrc['TMPL'] = tmpl
    # print(rsrc.keys())
    tmplData = tmplReader.parse(rsrc)

    # avara reads the LEDI #128
    set_ledi = tmplData['LEDI'][128]
    set_tag = set_ledi["Set Tag"]

    # the key for the list of levels is
    # five asterisks
    ledi_meta = {}
    for single_ledi in set_ledi["*****"]:
        # store these in a dictionary by pict name
        ledi_meta[single_ledi["Path"]] = single_ledi

    #rsrc = get_forks(data)
    print(rsrc.keys())
    if 'LEDI' not in rsrc:
        print("No LEDI found for set %s" % rpath)
        return

    ledi = rsrc['LEDI']

    dirname = rpath.name.split('.')[0]# + "_svg"
    dirpath = os.path.join(outpath, dirname)
    os.makedirs(dirpath, exist_ok=True)

    svgdir = os.path.join(dirpath, SVG_DIR)
    os.makedirs(svgdir, exist_ok=True)

    if 'PICT' in rsrc:
        print(rsrc['PICT'].keys())
        picts = rsrc['PICT']
        for pict in picts:
            name = picts[pict]["name"]
            # make sure we have an LEDI for this
            if name not in ledi_meta:
                print("%s is not in LEDI, skipping" % name)
                continue

            meta = ledi_meta[name]
            data = picts[pict]["data"]
            # print(data)
            
            filename = ("%s_%s_%s.svg" % (str(pict), meta["Tag"], name)).replace(" ", "_")

            if DEBUG_EXPORT:
                print(filename)
            #print(meta["Name"].encode('macintosh'))
            #print(meta["Message"].encode('macintosh'))
            ledi_meta[name]["Svg"] = filename
            fn = os.path.join(svgdir, filename)
            # if os.path.isfile(fn):
                # print("%s was found, skipping" % fn)
                # continue
            try:
                xml_text = parse_pict(fn, data)
            except PictParseError:
                print(F"Could not parse {fn} - {meta['Name']} because of unknown opcode")
                continue
            with open(fn, "w", encoding="utf-8") as xml_file:
                xml_file.write(xml_text.decode("utf-8"))

        manifest_data["LEDI"] = {v["Tag"]:v for (_, v) in ledi_meta.items()}

    if 'BSPT' in rsrc:
        bspt_meta = {}
        bspdir = os.path.join(dirpath, BSPS_DIR)
        os.makedirs(bspdir, exist_ok=True)
        bsps = bsptReader.parse(rsrc['BSPT'])
        for bsp in bsps:
            #filename = "%d_%s.avarabsp.json" % (bsp.res_id, bsp.name)
            filename = F"{bsp.res_id}.json"
            fn = os.path.join(bspdir, filename)
            if DEBUG_EXPORT:
                print("Writing BSPT %s" % fn)
            with open(fn, "w") as bsp_file:
                bsp_file.write(bsp.avara_format())
            bspt_meta[bsp.res_id] = {
                "name": bsp.name,
                "file": fn
            }
        manifest_data["BSPT"] = bspt_meta

    if 'HSND' in rsrc and EXPORT_SOUND:
        hsnd_meta = {}
        snddir = os.path.join(dirpath, HSND_DIR)
        os.makedirs(snddir, exist_ok=True)
        for hsnd_id in rsrc['HSND'].keys():
            hsnd_name = rsrc['HSND'][hsnd_id]["name"]

            fn = str(hsnd_id) + "_" + "".join(c for c in hsnd_name if c.isalnum() or c in ('.', '_')).rstrip()
            wavpath = os.path.join(snddir, fn + ".wav")
            if DEBUG_EXPORT:
                print(f"Found HSND {hsnd_id} {hsnd_name} {fn}")

            args = [f'build{os.path.sep}hsnd2wav', str(hsnd_id), wavpath, rpath]
            popen = subprocess.Popen(args, stdout=subprocess.PIPE)
            popen.wait()

            if WAV2OGG:
                oggpath = os.path.join(snddir, fn + ".ogg")
                args = ['sndfile-convert', '-vorbis', wavpath, oggpath]
                popen = subprocess.Popen(args, stdout=subprocess.PIPE)
                popen.wait()
                os.remove(wavpath)


            hsnd_meta[hsnd_id] = {
                "name": hsnd_name,
                "file": wavpath if not WAV2OGG else oggpath
            }

    if 'TEXT' in rsrc:
        manifest_data["TEXT"] = {k:v["data"].decode("macroman") for (k,v) in rsrc["TEXT"].items()}
  
    if 'HULL' in rsrc:
        manifest_data["HULL"] = tmplData["HULL"]

    return set_tag, manifest_data




if __name__ == '__main__':

    if not EXPORT_SOUND:
        print("hsnd2wav is not built! I need this to export sound.")
        print("build it with `make hsnd2wav`")
        exit(1)

    if not WAV2OGG:
        print("I need libsndfile available to convert WAV to OGG.")
        print("Install libsndfile and make sure sndfile-convert is on your PATH")
        exit(1)

    ldir = "levels"

    avara_r = os.path.join("levels", "single-player.r")
    data = open(avara_r, 'rb').read()

    reader = resource.Reader()
    avara_rsrc = reader.parse(data)
    avara_tmpl = avara_rsrc['TMPL']


    print(sys.argv)
    if len(sys.argv) > 1:
        parse_level_rsrc(Path(sys.argv[1]), ldir, avara_tmpl)
    else:
        # run against everything in levels
        # and store them alongside
        manifest = {}
        for rsrc_file in os.listdir(ldir):
            rpath = os.path.join(ldir, rsrc_file)
            print(rpath)

            if os.path.isdir(rpath):
                continue
            if ".r" not in rpath:
                continue
            tag, manifest_data = parse_level_rsrc(Path(rpath), ldir, avara_tmpl)
            manifest[tag] = manifest_data
        manifest_path = os.path.join(ldir, "manifest.json")
        with open(manifest_path, 'w') as manifest_file:
            json.dump(manifest, manifest_file, indent=1)