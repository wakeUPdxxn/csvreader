#include "Reader.h"

int main(int argc,char *argv[])
{
    if (argc > 2 || argc < 2) {
        std::cerr << "Arguments received " << argc-1 << " Expected 1" << "\n";
        exit(EXIT_FAILURE);
    }
    std::string path = argv[1];
    if (path.find(".csv") == std::string::npos) {
        std::cerr << "Incorrect file type" << "\n";
        exit(EXIT_FAILURE);
    }
    Reader rd;
    try {
        rd.openFile(path);
    }
    catch (FileOpenException x) {
        std::cerr << "File not found or cant be oppend" << "\n";
        exit(EXIT_FAILURE);
    }
    rd.readFile();
    rd.closeFile();  
}
