#!/bin/bash
clear
tag=${1}  # require
till=${2}  # require
repo_name=$(basename "$PWD")
branch=$(git branch --show-current)
current_date=$(date +%Y_%m_%d)
#exit
#echo "1: $tag $till"

mkdir -p doc/reports/
last_commit_hash=$(git log $tag...$till --no-merges --format="%h" | head -n 1)
output_file="doc/reports/report_${current_date}_${last_commit_hash}.txt"

first_commit_hash=$(git log $tag~1...$tag --no-merges --format="%h" | head -n 1)

#echo "2:"

# Store paths to all matching files in arrays
#changes_txt_paths=($(git diff $tag $till --name-only | grep "changes.txt"))
#commit_hashes=($(git log $tag~1...$till --format="%h"))
release_notes_txt_paths=($(git diff $tag $till --name-only | grep -E -i "release_notes.txt|ReleaseNotes.txt"))
commit_hashes=($(git log $first_commit_hash~1...$last_commit_hash --no-merges --format="%h"))
changes_txt_paths=($(git diff $first_commit_hash~1...$last_commit_hash --name-only | grep "changes.txt"))
release_notes_txt_paths=($(git diff $first_commit_hash~1...$last_commit_hash --name-only | grep -E -i "release_notes.txt|ReleaseNotes.txt"))
#echo "3: "

# Header for release notes
echo "--------------------------------------------------------------------------" >  $output_file
echo "Release notes: from $tag till $till" >> $output_file
echo "--------------------------------------------------------------------------" >> $output_file
echo "Top Commit: ${last_commit_hash}" >> $output_file 
# Iterate over all release_notes.txt files and append diffs to the report
for file in "${release_notes_txt_paths[@]}"; do
    echo "Notes from: $file" >> $output_file
    git diff $tag $till --ignore-space-change -- "$file" | awk '!/^\+\+\+ / && /^[+]/ {print substr($0, 2)}' >> $output_file
    echo "--------------------------------------------------------------------------" >> $output_file
done
#echo "4:"

# Add branch info
echo "Branch : ${repo_name}/${branch}" >> "$output_file"
echo "--------------------------------------------------------------------------" >> $output_file

# Header for code changes
echo "Code changes:" >> $output_file
echo "--------------------------------------------------------------------------" >> $output_file

# Iterate over all changes.txt files and append diffs to the report
for file in "${changes_txt_paths[@]}"; do
    echo "Changes from: $file" >> $output_file
    git diff $tag $till --ignore-space-change -- "$file" | awk  '!/^\+\+\+ / && /^[+]/ {print substr($0, 2)}' >> $output_file
    echo "--------------------------------------------------------------------------" >> $output_file
done
#echo "5:"

for  i in "${!commit_hashes[@]}"; do
    let j=$i+1
    if [ $j -lt ${#commit_hashes[@]} ]; then
#        echo "> $i of ${#commit_hashes[@]} : do it: ${commit_hashes[ $i + 1 ]} ${commit_hashes[$i]} last $last_commit_hash first $first_commit_hash" >> $output_file
        git log ${commit_hashes[ $i + 1 ]}...${commit_hashes[$i]} --no-merges --pretty=format:"%n-- %cs | commit: %h | %al | %s --" --name-status  >> $output_file
        git log ${commit_hashes[ $i + 1 ]}...${commit_hashes[$i]} --shortstat --pretty=format:"" >> $output_file
     fi
done


# Append last commit log
#git log $tag..$till --pretty=format:"-- %cs | commit: %h | %s --" --name-status  >> $output_file
#git log $tag..$till --shortstat --pretty=format:"" >> $output_file

let j=2
if [ $j -lt ${#commit_hashes[@]} ]; then
    echo "--------------------------------------------------------------------------" >> $output_file
    echo "List of files modified" >> $output_file
    echo "--------------------------------------------------------------------------" >> $output_file
    git diff --name-status $tag $till >> $output_file
    git diff --shortstat $tag $till >> $output_file
    echo "--------------------------------------------------------------------------" >> $output_file
    echo "The End: From $tag till $till" >> $output_file
    echo "--------------------------------------------------------------------------" >> $output_file
fi 
# Display the final report
cat "$output_file"
