import json
import os
import re

suffixs = {".h", ".hpp", ".c", "cpp", ".cxx", ".cc"}
flag = "EASYTR"

dir_path = input("Enter the directory path: ")
files = []

for root, dirs, file_list in os.walk(dir_path):
    for file in file_list:
        if file.endswith(tuple(suffixs)):
            files.append(os.path.join(root, file))

print("Successfully found files. Count: ", len(files))

text_ids = {}

i = 0
for file in files:
    print(f"Processing file: {file}, {i+1}/{len(files)}")
    with open(file, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            result = re.search(f"{flag}(\"(.*)\")", line)
            if result:
                textId = result.group(1)
                text_ids.update({textId: ""})
    i += 1

text_ids = dict(sorted(text_ids.items()))
out_file = input("\nEnter the output file path: ")
with open(out_file, "w", encoding="utf-8") as f:
    json.dump(text_ids, f, ensure_ascii=False, indent=4)

print("Done.")
