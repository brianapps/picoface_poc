# Copyright (C) 2024 Brian Apps
#
# This file is part of picoFace.
#
# picoFace is free software: you can redistribute it and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# picoFace is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with picoFace. If 
# not, see <https://www.gnu.org/licenses/>.

from pathlib import Path
import convertZ80
import zipfile

# This script creates the snap.zip for use with snap_uploader.py and
# the android app.
# 
# It take downloaded toesec zips and pulls out .z80 snapshots,
# converts them to V1 format if possible and puts them into a
# new zip file.
#
# Putting everything in a single zip is much easier. Copying 10000 files
# to an SD card takes an age while a single 200MB zip copies very quickly.
#
# The tosec zips are not provided but a google search
# will probably locate them if required.

def convert_z80_files(source_zip : zipfile.ZipFile, dest_zip: zipfile.ZipFile):
    for zipinfo in source_zip.filelist:
        f = Path(zipinfo.filename)
        try:
            dest_zip.getinfo(f.name)
        except KeyError:
            if f.suffix == '.z80':
                snapshot = convertZ80.Z80Snapshot()
                try:
                    snapshot.process_bytes(source_zip.read(zipinfo))
                    dest_zip.writestr(f.name, snapshot.to_bytes())
                except:
                    print(f"Can't convert: {zipinfo.filename}")
            elif f.suffix.lower() == '.pok':
                dest_zip.writestr(f.name, source_zip.read(zipinfo))

download_dir = Path.home() / "Downloads" / "zx_spectrum_tosec_set_september_2023"
destzip = zipfile.ZipFile('snaps.zip', 'a', compression=zipfile.ZIP_DEFLATED)
convert_z80_files(zipfile.ZipFile(download_dir / "Applications.zip"), destzip)
convert_z80_files(zipfile.ZipFile(download_dir / "Games.zip"), destzip)



