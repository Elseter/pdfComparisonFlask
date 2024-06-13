#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <iostream>
#include <filesystem>
#include <regex>
#include "form.h"

using namespace std;
using namespace poppler;
namespace fs = std::filesystem;

// Function to extract file name from a given file path
std::string extractFileName(const std::string& filePath) {
    fs::path pathObj(filePath);
    return pathObj.filename().string();
}

int main(int argc, char* argv[]) {
    if (argc != 4) { // Check if there are exactly two arguments (argv[1] and argv[2])
        std::cerr << "Usage: " << argv[0] << " <input file 1> <input file 2> <simmilarity score>" << std::endl;
        return 1;
    }
    string filename1 = argv[1];
    string filename2 = argv[2];


    // Create new Form objects
    Form form1(filename1);
    Form form2(filename2);

    // Read the documents and extract text
    form1.initialRun();
    form2.initialRun();

    // Compare the two forms
    form1.compareToAnotherForm(form2);

    // check if output directory exists
    if (!fs::exists("output")) {
        fs::create_directory("output");
    }

    // sometimes form titles have file paths in them, so we need to remove them

    std::string fileName1 = extractFileName(form1.formTitle);
    std::string fileName2 = extractFileName(form2.formTitle);


    string outputFilename = "output/" + fileName1 + "_vs_" + fileName2 + "_" + argv[3] + ".csv";

    // Get comparison string
    form1.outputComparisonToFile(outputFilename, stod(argv[3]));

}
