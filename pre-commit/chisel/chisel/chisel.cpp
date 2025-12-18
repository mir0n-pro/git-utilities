//-----------------------------------------------
// $System: Tradu CFD-SB Trading System $
// $File: pre-commit/chisel/chisel.cpp $
// $Date: 2025/03/20 $
// $Revision: #1 $
// $Author: mir0n $
//-----------------------------------------------
// History:
//
// 03.20.2025  mir0n  Created
// 05.31.2025  mir0n  (plsql) "show errors" to remove from code
//                    (plsql) remove all trailing CRs not just last : works for oracle, good enough for now
//                    "it eats its own tail"" changes the p4 keywords in the code :)
//                     use other way to declare p4 keyword string constants
//                    process p4 tags in VIEWS (p4 keywords can be placed in the footer instead of the header)
//-----------------------------------------------

#define _CRT_SECURE_NO_WARNINGS
//#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

using namespace std;
const static string CHISEL_VERSION = "1.0.2";
const static string P4_ID = {'$','I','d'};
const static string P4_HEADER = {'$','H','e','a','d','e','r'};
const static string P4_REVISION = {'$','R','e','v','i','s','i','o','n'};
const static string P4_DATETIME = {'$','D','a','t','e','T','i','m','e'};
const static string P4_DATE = {'$','D','a','t','e'};
const static string P4_FILE = {'$','F','i','l','e'};
const static string P4_AUTHOR = {'$','A','u','t','h','o','r'};

bool xxx = false;

string normalize_line(const string& s, bool lastLine, bool eofEndLine, string& endLine, string& lastEndLine) {
    string result = "";
    if (!s.empty()) {
        char last[2] = { 0,0 };
        int i = 1;
        string cr = "";
        for (auto rit = s.rbegin(); rit != s.rend(); ++rit) {
            char ch = *rit;
            if (ch == '\r' or ch == '\n') {
                last[i] = ch;
            }
            if (--i < 0) {
                break;
            }
        }
        for (char ch : last) {
            if (ch != 0) {
                cr += ch;
            }
        }
        result = s.substr(0,s.length() - cr.length());

        //      result.erase(find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
        //          return !isspace(ch);
        //      }).base(), result.end());

        int toErase=0;
        for (auto rit = result.rbegin(); rit != result.rend(); ++rit) {
            if (isspace(*rit)) {
                toErase ++;
            } else {
                break;
            }

        }
        if (toErase > 0) {
            result.erase(result.length() - toErase, toErase);
        }
        //result.replace("\t","    ");
        if (lastLine) {
            if (result.empty()) {
                // do not add more eol : even it is forced
            } else {
                if (cr.empty()) {
                    // force eol
                    if (eofEndLine) {
                        result += endLine.empty() ? lastEndLine : endLine;
                    }
                } else {
                    // restore eol
                    result += endLine.empty() ? cr : endLine;
                }
            }
        } else {
            // restore eol
            result += endLine.empty() ? cr : endLine;
            lastEndLine = cr; //xxx need to keep it outide for eof
        }

    }
    return result;
}

string remove_tabs(const string& s) {
    if (s.empty()) {
        return s;
    }
    if (s.find('\t') == string::npos) {
        return s;
    }
    string ret = s;
    while (true) {
        size_t i = ret.find('\t');
        if (i != string::npos) {
            string t = ret.substr(0, i);
            size_t spaces = (i/4);
            spaces = spaces*4 + 4 - i;
            for (int j=0; j < spaces; j++) {
                t.append(" ");
            }
            t.append(ret.substr(i+1));
            ret = t;
        } else {
            break;
        }
    }
    return ret;
}

unsigned p4_revision (string line) {
    unsigned ret = 0;
    size_t pos = line.find(P4_ID);
    if (pos == string::npos) {
        pos = line.find(P4_HEADER);
    }
    if (pos == string::npos) {
        pos = line.find(P4_REVISION);
    }
    if (pos != string::npos ) {
        pos = line.find("#",pos);
        if (pos != string::npos ) {
            size_t last = line.find(" ",pos);
            if (last != string::npos ) {
                string s = line.substr(pos+1,last-pos-1);
                ret = stoi(s);
            }
        }
    }
    return ret;
}
string p4_key (string line, string inputName, unsigned prev_revision, string author) {
   size_t pos = line.find(P4_ID);
    if (pos != string::npos) {
        size_t pos2 = line.find("$",pos+3);
        if (pos2 != string::npos) {
            string ret = line.substr(0,pos+3);
            ret.append(": ").append(inputName).append("#").append(to_string(prev_revision + 1)).append(" ").append(line.substr(pos2));
            return ret;
        }
    }

    pos = line.find(P4_HEADER);
    if (pos != string::npos) {
        size_t pos2 = line.find("$",pos+7);
        if (pos2 != string::npos) {
            string ret = line.substr(0,pos+7);
            ret.append(": ").append(inputName).append("#").append(to_string(prev_revision + 1)).append(" ").append(line.substr(pos2));
            return ret;
        }
    }

    pos = line.find(P4_DATETIME);
    if (pos != string::npos) {
        size_t pos2 = line.find("$",pos+9);
        if (pos2 != string::npos) {
            const time_t t = time(nullptr);
            struct tm * timeinfo = gmtime(&t); // localtime(&t);
            char buffer[80];
            strftime(buffer,sizeof(buffer),"%Y/%m/%d %H:%M:%S",timeinfo);
            string ret = line.substr(0,pos+9);
            ret.append(": ").append(buffer).append(" ").append(line.substr(pos2));
            return ret;
        }
    }

    pos = line.find(P4_DATE);
    if (pos != string::npos) {
        size_t pos2 = line.find("$",pos+5);
        if (pos2 != string::npos) {
            const time_t t = time(nullptr);
            struct tm * timeinfo = gmtime(&t); // localtime(&t);
            char buffer[80];
            strftime(buffer,sizeof(buffer),"%Y/%m/%d",timeinfo);
            string ret = line.substr(0,pos+5);
            ret.append(": ").append(buffer).append(" ").append(line.substr(pos2));
            return ret;
        }
    }

    pos = line.find(P4_FILE);
    if (pos != string::npos) {
        size_t pos2 = line.find("$",pos+5);
        if (pos2 != string::npos) {
            string ret = line.substr(0,pos+5);
            ret.append(": ").append(inputName).append(" ").append(line.substr(pos2));
            return ret;
        }
    }
    pos = line.find(P4_REVISION);
    if (pos != string::npos) {
        size_t pos2 = line.find("$",pos+9);
        if (pos2 != string::npos) {
            string ret = line.substr(0,pos+9);
            ret.append(": #").append(to_string(prev_revision + 1)).append(" ").append(line.substr(pos2));
            return ret;
        }
    }

    pos = line.find(P4_AUTHOR);
    if (pos != string::npos && !author.empty()) {
        size_t pos2 = line.find("$",pos+7);
        if (pos2 != string::npos) {
            string ret = line.substr(0,pos+7);
            ret.append(": ").append(author).append(" ").append(line.substr(pos2));
            return ret;
        }
    }

    return line;
}

vector<string> normalize_oracle(const vector<string>& lines, string& endLine) {
    // xxx: check if is plsql
    regex rReplaceCreate ("^( *create +or +replace +)",regex_constants::icase);
    // xxx: skip "show error;" and "/"
    regex rMatchSkip ("^((( *show +errors?;?)|( */))[\r\n]*$)",regex_constants::icase);
    regex rMatchView ("( view )",regex_constants::icase);
    smatch match;
    //  regex m ("(show errors"create +or +replace +)(.*)([\r\n]*)",regex_constants::icase);
    int startLine = -1;
    int finalLine = -1;
    bool view = false;
    for (int i = 0; i < lines.size(); i++) {
        if (lines[i].size() > endLine.size()) {
            if (regex_search(lines[i], match, rReplaceCreate)) {
                startLine = i;
            }
            break;
        }
    }
    if (startLine < 0) {
        if (xxx) {
            cerr << "plsql must should start with \"CREATE OR REPLACE \"\n";
        }
        return lines;
    }

    finalLine = startLine;
    for (int i = (int)lines.size() -1; i > startLine; i--) {
        if (lines[i].size() > endLine.size()) {
            if (!regex_search(lines[i], match, rMatchSkip)) {
                break;
            }
        }
        finalLine = i;
    }

    vector<string> ret;
    for (int i = startLine; i < finalLine; i++) {
        ret.push_back(lines[i]);
    }
    string s ("/");
    s.append(endLine);
    ret.push_back(s);

    ret[0] = regex_replace(ret[0], rReplaceCreate, "CREATE OR REPLACE ");

//  if (regex_search(ret[0], match, rMatchView)) {
//      ret[0] = regex_replace(ret[0], rReplaceCreate, " CREATE OR REPLACE ");
//      ret.insert(ret.begin(),endLine);
//  } else {
//      ret[0] = regex_replace(ret[0], rReplaceCreate, "CREATE OR REPLACE ");
//  }
    return ret;
 }

vector<string> normalize_lines(const vector<string>& lines, bool oracle, bool tabs,  bool p4, string inputName, bool eofEol, string& endLine, string author) {
    vector<string> ret;
    string leol = "";
    string geol  = endLine;
    unsigned revision = 0;

    if (p4) {
        for (int i = 0; i < lines.size() ; i++) {
            revision = p4_revision(lines[i]);
            if (revision > 0) {
                break;
            }
        }
    }

    for (int i = 0; i < lines.size(); i++) {
        string s = normalize_line(lines[i], i == lines.size() -1, eofEol, geol, leol);
        if (tabs) {
            s = remove_tabs(s);
        }
        if (p4) {    //&& i < 10) {
            s = p4_key(s, inputName, revision, author);
        }
        ret.push_back(s);
    }

    if (oracle) {
        ret = normalize_oracle(ret,leol);
    }

    return ret;
}
string getParamValue(string paramNameValue) {
    string ret = "n/a";
    size_t pos = paramNameValue.find("=");
    if (pos > 1 && pos < paramNameValue.size() - 1) {
        ret = paramNameValue.substr(pos+1);
    }
    return ret;
}


void _printUsage_(bool oracle, bool p4 ) {
    cout << "chisel utility. v" <<  CHISEL_VERSION << "\n";
    cout << "\n";
    cout << "Syntax: chisel.exe [input] [output] [options]\n";
    cout << "   input         : optional, the file path to read from, use 'STD' for console input.\n";
    cout << "                   Default value is STD\n";
    cout << "   output        : optional, the file path to write to, use 'STD' for console output.\n";
    cout << "                   When ommited: output path is the same as input.\n";
    cout << "   options:\n";
    cout << "    -h[elp]      :help to use the chisel\n";
    cout << "    -r[trim]     :(default) remove trailing whitespace and tabs\n";
    cout << "    -o[racle]    :normalize Orcale plsql source file\n";
    cout << "    -d[os]       :dos line ending\n";
    cout << "    -u[nix]      :unix line ending\n";
    cout << "    -k[eep]      :(default) keep existing line ending\n";
    cout << "    -e[of]       :force end-of-line at end-of-file\n";
    cout << "    -p[erforce]  :perforce keywords processing\n";
    cout << "    -t[abs]      :tab symbol replaced with spaces to next tab position\n";
    cout << "    -b[ackup]    :backup the input file\n";
    cout << "    -a[uthor]<=name> :author name for p4 key\n";



    if (p4) {
        cout << "\n";
        cout << "Supported perforce keywords:\n";
        cout <<  "     - $Id: pre-commit/chisel/chisel.cpp#1 $ File name and revision number. $Id: path/file.txt#3 $\n";
        cout <<  "     - $Header$ Synonymous with $Id: pre-commit/chisel/chisel.cpp#1 $. $Header: path/file.txt#3 $\n";
        cout <<  "     - $Date: 2025/03/20 $ Date in format YYYY/MM/DD. $Date: 2010/08/18 $\n";
        cout <<  "     - $DateTime: 2025/03/20 21:46:49 $ Date and time in format YYYY/MM/DDhh:mm:ss. $DateTime: 2010/08/18 23:17:02 $\n";
        cout <<  "     - $File: pre-commit/chisel/chisel.cpp $ File name only (without revision number). $File: path/file.txt $\n";
        cout <<  "     - $Revision: #1 $ File revision number. $Revision: #3 $\n";
        cout <<  "     - $Author: mir0n $ User commiting the file. $Author: edk $\n";
    }

    if (oracle) {
        cout << "\n";
        cout << "Normalization of Oracle plsql file consists of ensuring:\n";
        cout <<  "     - No trailing whitespaces.\n";
        cout <<  "     - No tab symbols. Replaced with spaces to next tab position.\n";
        cout <<  "     - File ends by end-of-line symbol(s).\n";
        cout <<  "     - First line starts with \"CREATE OR REPLACE\" (in upper case).\n";
        cout <<  "     - Last line equals to \"/\".\n";
        cout <<  "     - No empty lines between code and last line.\n";
    }
}

string mode(bool oracle, bool tabs, string newEol, bool eof, bool backup, bool p4, string input, string output, string author) {
    string ret = "chisel.exe ";
    if (oracle) {
        ret.append("-oracle ");
    } else {
        ret.append("-rtrim ");
    }
    if (tabs) {
        ret.append("-tabs ");
    }
    if (newEol == "\r\n") {
        ret.append("-dos ");
    } else if (newEol == "\n") {
        ret.append("-unix ");
    } else {
        ret.append("-keep ");
    }
    if (eof) {
        ret.append("-eof ");
    }
    if (p4) {
        ret.append("-perforce ");
    }
    if (backup) {
        ret.append("-backup ");
    }
    if (!author.empty()) {
        ret.append("-author=").append(author).append(" ");
    }
    ret.append(input).append(" ");
    ret.append(output).append(" ");
    return ret;
}


int main(int argc, char* argv[]) {

    string newEol = "";
    string inputName  = "";
    string outputName = "";

    bool rtrim = true;
    bool tabs = false;
    bool eofEol = false;
    bool backup = false;
    bool help = false;
    bool p4 = false;
    bool oracle = false;

    bool conIn = false;
    bool conOut = false;
    string author ="";

    for (int i = 1; i < argc; i++) {
        string s = argv[i];
        if (s.find("?") == 0) {
            help = true;
        } else if (s.find("-") == 0) {
            if (s.find("-r") == 0) {
                rtrim = true;
            } else if (s.find("-t") == 0) {
                tabs = true;
            } else if (s.find("-o") == 0) {
                oracle = true;
            } else if (s.find("-d") == 0) {
                newEol = "\r\n";
            } else if (s.find("-u") == 0) {
                newEol = "\n";
            } else if (s.find("-k") == 0) {
                newEol = "";
            } else if (s.find("-e") == 0) {
                eofEol = true;
            } else if (s.find("-b") == 0) {
                backup = true;
            } else if (s.find("-x") == 0) {
                xxx = true;
            } else if (s.find("-p") == 0) {
                p4 = true;
            } else if (s.find("-h") == 0) {
                help = true;
            } else if (s.find("-a") == 0) {
                author = getParamValue(s);
            }
// TODO: Author
        } else if (inputName.empty()) {
            inputName = s;
        } else if (outputName.empty()){
            outputName = s;
        }
    }

    if (outputName.empty()) {
        outputName = inputName;
    }
    if (inputName == "STD") {
        conIn = true;
        backup = false;
    }
    if (outputName == "STD") {
        conOut = true;
        eofEol = true;
        if (newEol.empty()) {
            newEol = "\r\n";
        }
    }
    if (oracle) {
        rtrim = true;
        //tabs = true;
        eofEol = true;
    }
    if (help) {
        _printUsage_(oracle, p4);
        return 0;
    }
    if (xxx) {
        cerr << mode(oracle, tabs, newEol, eofEol, backup, p4, inputName, outputName, author) << "\n";
    }
    vector<string> inLines;

    if (conIn) {
        size_t size = 4000;
        char * memblock = new char[size+2];
        ::memset(memblock, 0, size + 2);
        int iLines = 0;
        bool bFailed = false;
        bool bEof = false;
        size_t iRead = 0;
        while (true) {
            memset(memblock,0, size + 2);
            cin.getline(memblock, size);
            if (cin.eof()) {
                bEof = true;
                break;
            }
            if (cin.fail()) {
                bFailed = true;;
                break;
            }
            if (conOut) {
                size_t i = strcspn(memblock, "\r\n");
                memblock[i] = 0;
            }  else {
                memblock[strlen(memblock)] = '\n';
            }
            inLines.push_back(memblock);
            iLines ++;
        }
        delete[] memblock;

    } else {

        fstream fin;

        fin.open(inputName, ios::in| ios::binary | ios::ate);

        if (!fin.is_open()) {
            cerr << "No file \"" << inputName << "\" found\n";
            return -1;
        }

        // xxx fin  is in last position
        fin.seekg(0, ios::end);
        size_t size = fin.tellg();
        fin.seekg(0, ios::beg);
        char * memblock = new char[size+1];
        memset(memblock, 0, size + 1);
        fin.read(memblock, size);
        size_t pos = 0;
        string s;
        while (true) {
            size_t i = strcspn(memblock + pos, "\n");
            if (conOut) {
                size_t j = strcspn(memblock + pos, "\r\n");
                s = string(memblock + pos, 0, j);
            } else {
                s = string(memblock + pos, 0, i+1);
            }
            inLines.push_back(s);
            pos += i + 1;
            if (pos >= size) {
                break;
            }
        }
        if (backup) {
            string bfile = inputName + ".bak";
            fstream fbackup;
            fbackup.open(bfile, ios::out | ios::binary | ios::trunc);
            if (fbackup.is_open()) {
                ::memset(memblock, 0, size + 1);
                fin.seekg(0, ios::beg);
                fin.read(memblock, size);
                fbackup.write(memblock, size);
                fbackup.close();
            } else {
                cerr << "No backup file " << bfile  << " can be created or used.\n";
            }
        }
        fin.close();
        delete[] memblock;
    }


    ostream * out = NULL;
    if (conOut) {
        newEol = "";  //xxx: eol will be added at output
        out = &cout;
    } else {
        fstream * fout = new fstream();
        fout->open(outputName, ios::out | ios::binary | ios::trunc);
        if (!fout->is_open()) {
            cerr << "No file \"" << outputName << "\" can be created or used.\n";
            delete(fout);
            return -1;
        }
        out = fout;
    }

    vector<string> outLines = normalize_lines(inLines, oracle, tabs, p4, inputName, eofEol, newEol, author);
    for (int i = 0; i < outLines.size(); i++) {
        *(out) << outLines[i];
        if (conOut) {
            *(out) << "\n";
        }
    }

    if (!conOut) {
        ((fstream *)out)->close();
        delete((fstream *)out);
    }
    return 0;
}
