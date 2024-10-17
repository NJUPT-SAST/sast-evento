#!/bin/bash
cd "${GITHUB_ACTION_PATH}/artifacts" || exit 1

version="unknown"

if [[ -f ./files-linux-x64/files.tar.gz ]]; then
    # 解压版本文件
    tar -xzf ./files-linux-x64/files.tar.gz ./sast-evento-version.txt
    if [[ -f ./sast-evento-version.txt ]]; then
        # 设置版本号
        version=$(cat ./sast-evento-version.txt)
        rm -f ./sast-evento-version.txt
    fi
fi

echo "SAST Evento Version: $version"

rm -rf ./files-*

# 处理文件
for dir in */; do
    echo "Processing $dir"
    new_name="sast-evento-$version-${dir%/}"
    mv "$dir" "$new_name"
    
    # Change to the new directory name
    items=("$new_name"/*)

    if [[ ${#items[@]} -eq 1 && -f ${items[0]} ]]; then
        single_file=${items[0]}
        if [[ $single_file =~ \.pkg\.tar\.zst$ ]]; then
            new_file_path="$new_name.pkg.tar.zst"
        else
            new_file_path="$new_name${single_file##*.}"
        fi
        mv "$single_file" "$new_file_path"
    else
        zip_path="$new_name.zip"
        zip -r "$zip_path" "$new_name"
    fi
    rm -rf "$new_name"
done

# Generate SHA256 hash and save it to a file
sha256sum * > sha256sums.txt

echo "Artifacts are ready"
ls -lh