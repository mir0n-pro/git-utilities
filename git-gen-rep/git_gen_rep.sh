#!/bin/bash
clear
tag=${1}  # require
till=${2}  # require
repo_name=$(basename "$PWD")
branch=$(git branch --show-current)
current_date=$(date +%Y_%m_%d)
#exit

mkdir -p doc/reports/
last_commit_hash=$(git log $tag...$till --no-merges --format="%h" | head -n 1)
output_file="doc/reports/report_${current_date}_${last_commit_hash}.md"

first_commit_hash=$(git log $tag~1...$tag --no-merges --format="%h" | head -n 1)

release_notes_txt_paths=($(git diff $tag $till --name-only | grep -E -i "release_notes.txt|ReleaseNotes.txt"))
commit_hashes=($(git log $first_commit_hash~1...$last_commit_hash --no-merges --format="%h"))
changes_txt_paths=($(git diff $first_commit_hash~1...$last_commit_hash --name-only | grep "changes.txt"))
release_notes_txt_paths=($(git diff $first_commit_hash~1...$last_commit_hash --name-only | grep -E -i "release_notes.txt|ReleaseNotes.txt"))

# Header
echo "# Release Report: $tag → $till" >  $output_file
echo "" >> $output_file
echo "**Repo:** \`${repo_name}/${branch}\`  " >> $output_file
echo "**Top commit:** \`${last_commit_hash}\`" >> $output_file
echo "" >> $output_file

# Release notes section
echo "---" >> $output_file
echo "" >> $output_file
echo "## Release Notes" >> $output_file
echo "" >> $output_file
for file in "${release_notes_txt_paths[@]}"; do
    echo "### $file" >> $output_file
    echo "" >> $output_file
    git diff $tag $till --ignore-space-change -- "$file" \
      | awk '!/^\+\+\+ / && /^[+]/ {print substr($0, 2)}' \
      | sed 's/<[^>]*>//g' \
      | grep -v '^[[:space:]-]*$' \
      | grep -v '^---' \
      | sed '/^\*\*/!s/^[[:space:]]*\([-:+~*]\)[[:space:]]*/\&nbsp;\1 /' \
      | sed 's/^\(v[0-9][^ ]*\)/\*\*\1\*\*/' \
      | sed 's/^\(\*\*v[0-9]\)/\n\1/' \
      | sed 's/^\([0-9][0-9]\/[0-9][0-9]\/[0-9][0-9][0-9][0-9]\)/\*\*\1\*\*/' \
      | sed 's/^\(\*\*[0-9][0-9]\/\)/\n\1/' \
      | sed -e ':a' -e 's/^ /\&nbsp;/' -e 'ta' \
      | sed '/[^[:space:]]/s/[[:space:]]*$/  /' \
      >> $output_file
    echo "" >> $output_file
done

# Code changes section
echo "---" >> $output_file
echo "" >> $output_file
echo "## Code Changes" >> $output_file
echo "" >> $output_file
for file in "${changes_txt_paths[@]}"; do
    echo "### $file" >> $output_file
    echo "" >> $output_file
    git diff $tag $till --ignore-space-change -- "$file" \
      | awk '!/^\+\+\+ / && /^[+]/ {print substr($0, 2)}' \
      | sed 's/<[^>]*>//g' \
      | grep -v '^[[:space:]-]*$' \
      | grep -v '^---' \
      | sed 's/^[[:space:]]*[-+~*][[:space:]]*\([a-zA-Z][^ ]*\\.*\)$/\*\*\1\*\*/' \
      | sed 's/^[[:space:]]*\([-+~*][[:space:]].*\\[^ ]*\)$/\&nbsp;\*\*\1\*\*/' \
      | sed 's/^\([a-zA-Z][^ ]*\\.*\)$/\*\*\1\*\*/' \
      | sed 's/^\([a-zA-Z][^ ]*\.[^ ]*\)$/\*\*\1\*\*/' \
      | sed '/^\*\*/!s/^[[:space:]]*\([-:+~*]\)[[:space:]]*/\&nbsp;\1 /' \
      | sed 's/^\([0-9][0-9]\/[0-9][0-9]\/[0-9][0-9][0-9][0-9]\)/\*\*\1\*\*/' \
      | sed 's/^\(\*\*[0-9][0-9]\/\)/\n\1/' \
      | sed -e ':a' -e 's/^ /\&nbsp;/' -e 'ta' \
      | sed '/[^[:space:]]/s/[[:space:]]*$/  /' \
      >> $output_file
    echo "" >> $output_file
done

# Commits section
echo "---" >> $output_file
echo "" >> $output_file
echo "## Commits" >> $output_file
echo "" >> $output_file
echo '```' >> $output_file
for i in "${!commit_hashes[@]}"; do
    let j=$i+1
    if [ $j -lt ${#commit_hashes[@]} ]; then
        git log ${commit_hashes[ $i + 1 ]}...${commit_hashes[$i]} --no-merges --pretty=format:"%n-- %cs | commit: %h | %al | %s --" --name-status >> $output_file
        git log ${commit_hashes[ $i + 1 ]}...${commit_hashes[$i]} --shortstat --pretty=format:"" >> $output_file
    fi
done
echo '```' >> $output_file
echo "" >> $output_file

# Files modified section
let j=2
if [ $j -lt ${#commit_hashes[@]} ]; then
    echo "---" >> $output_file
    echo "" >> $output_file
    echo "## Files Modified" >> $output_file
    echo "" >> $output_file
    echo '```' >> $output_file
    git diff --name-status $tag $till >> $output_file
    git diff --shortstat $tag $till >> $output_file
    echo '```' >> $output_file
    echo "" >> $output_file
    echo "---" >> $output_file
    echo "" >> $output_file
    echo "*From \`$tag\` till \`$till\`*" >> $output_file
fi

# Display the final report
cat "$output_file"
