#include "SymbolTable.h"

void SymbolTable::run(string filename)
{
    ifstream infile(filename);
    string line;
    while (getline(infile, line))
    {
        if (regex_search(line, regex("^INSERT")))
        {
            if (!regex_match(line, regex("INSERT [a-z]\\w* (number|string) (true|false)")) && !regex_match(line, regex("INSERT [a-z]\\w* \\(((number|string)(,number|,string)*)?\\)->(number|string) true")))
                cout << "InValid1 " << line << "\n";
            else
                cout << "Valid\n";
        }
        else if (regex_search(line, regex("^ASSIGN")))
        {
        }
        else if (regex_search(line, regex("^BEGIN")))
        {
        }
        else if (regex_search(line, regex("^END")))
        {
        }
        else if (regex_search(line, regex("^PRINT")))
        {
        }
        else
            throw InvalidInstruction(line);
    }
    infile.close();
}