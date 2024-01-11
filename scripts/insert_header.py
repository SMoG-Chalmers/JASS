import os

HEADER_TEXT = """/*
Copyright XMN Software AB 2023

JASS is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version. The GNU Lesser General Public License is intended to
guarantee your freedom to share and change all versions of a program --
to make sure it remains free software for all its users.

JASS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with JASS. If not, see <http://www.gnu.org/licenses/>.
*/

"""

def insert_text(file_path, text_to_insert):
    with open(file_path, 'r+') as file:
        content = file.read()
        file.seek(0, 0)
        file.write(text_to_insert + content)

def process_files(root_folder, suffix, text_to_insert):
    for foldername, subfolders, filenames in os.walk(root_folder):
        for filename in filenames:
            if suffix and not filename.endswith(suffix):
                continue
            file_path = os.path.join(foldername, filename)
            print(file_path)
            insert_text(file_path, text_to_insert)

if __name__ == "__main__":
    process_files(".", None, HEADER_TEXT)
    print("Text insertion completed.")