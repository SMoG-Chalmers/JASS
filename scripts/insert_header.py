import argparse
import os

LICENSE_MARK = "you can redistribute it and/or modify it"

HEADER_TEXT = """/*
Copyright Ioanna Stavroulaki 2023

This file is part of JASS.

JASS is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version.

JASS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along 
with JASS. If not, see <https://www.gnu.org/licenses/>.
*/"""

EXTENSIONS = [".cpp", ".h", ".hpp"]

def process_file(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
    with open(file_path, 'w') as file:
        if content.find(LICENSE_MARK) == -1:
            file.write(HEADER_TEXT + "\n\n" + content)
            print("INSERTED")
        else:
            end_of_license_at = content.find("*/")
            assert(end_of_license_at != -1)
            end_of_license_at += 2
            file.write(HEADER_TEXT + content[end_of_license_at:])
            print("PATCHED")

def process_files(root_folder):
    for foldername, subfolders, filenames in os.walk(root_folder):
        for filename in filenames:
            extension_with_dot = os.path.splitext(filename)[1]
            if extension_with_dot not in EXTENSIONS:
                continue
            file_path = os.path.join(foldername, filename)
            print(file_path)
            process_file(file_path)

if __name__ == "__main__":
    
    # Parse arguments
    parser = argparse.ArgumentParser(description="Tool for inserting header in source files.")
    parser.add_argument('rootpath')
    options = parser.parse_args()

    rootpath = options.rootpath or "."

    process_files(rootpath)

    print("\nDone.")