
#pre-commit  (bash command to handle GIT pre-commit hook)
#chisel.exe (x64 conslole utility to normalize a source file)

#How to install
Copy chisel.ex` to the root of local git repositories.
Copy pre-commit to .git/hooks of repository where you like to enable code normalization.

##How to use:
At commit your committed file will be normalized with options within the pre-commit bash script.

Initially,for SQL files chisel option in use:
         `(../chisel.exe -o -e -t -p -a="$author" "$file")`

for TXT, ME, MD, JAVA, CPP:
         `(../chisel.exe -t -e "$file")`

Rest of file extensions are skipped.

You can always modify the `pre-commit` script to suit your needs.

##chisel utility CLI
`
 chisel utility. (c) v1.0.0

 Syntax: chisel.exe [input] [output] [options]
    input         : optional, the file path to read from, use 'STD' for console input.
                    Default value is STD
    output        : optional, the file path to write to, use 'STD' for console output.
                    When ommited: output path is the same as input.
    options:
     -h[elp]      :help to use the chisel
     -r[trim]     :(default) remove trailing whitespace and tabs
     -o[racle]    :normalize Orcale plsql source file
     -d[os]       :dos line ending
     -u[nix]      :unix line ending
     -k[eep]      :(default) keep existing line ending
     -e[of]       :force end-of-line at end-of-file
     -p[erforce]  :perforce keywords processing
     -t[abs]      :tab symbol replaced with spaces to next tab position
     -b[ackup]    :backup the input file
     -a[uthor]<=name> :author name for p4 key
`
##  Supported perforce keywords:
     - $Id$ File name and revision number. $Id: path/file.txt#3 $
     - $Header$ Synonymous with $Id$. $Header: path/file.txt#3 $
     - $Date$ Date in format YYYY/MM/DD. $Date: 2010/08/18 $
     - $DateTime$ Date and time in format YYYY/MM/DDhh:mm:ss. $DateTime: 2010/08/18 23:17:02 $
     - $File$ File name only (without revision number). $File: path/file.txt $
     - $Revision$ File revision number. $Revision: #3 $
     - $Author$ User commiting the file. $Author: vpupkin $

##  Normalization of Oracle plsql file consists of ensuring:
     - No trailing whitespaces.
     - No tab symbols. Replaced with spaces to next tab position.
     - File ends by end-of-line symbol(s).
     - First line starts with \"CREATE OR REPLACE\" (in upper case).
     - Last line equals to "\".
     - No empty lines between code and last line.


## How to rebuild 'chisel`
The `chisel` utility is written in C++ using only the standard library, so it is cross-platform.
This repository only contains the binary file for the 64-bit Windows platform.
To build the utility for Unix or Mac, you simply need to execute two commands from the chisel project directory:
`cmake -S chisel -B out`
`cmake --build ./out --config Release`


