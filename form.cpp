//
// Created by Riley King on 6/4/24.
//

#include "form.h"
#include <iostream>
#include <filesystem>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <regex>
#include <fstream>

using namespace poppler;
namespace fs = std::filesystem;

// Constructor
Form::Form(std::string  title)
    : formTitle(std::move(title)) {}

// Add a question
void Form::addQuestion(const Question& q) {
    questions.push_back(q);
    numberOfQuestions += 1;
}

string Form::getAllQuestions() {
    string result;
    for (const auto& question : questions) {
        result += question.toString();
    }
    return result;
}

// Convert PDF to PNG
void Form::convertToPNG() const {
    string directory = formTitle + "_images";
    fs::create_directory(directory);
    cout << "Created directory for " << formTitle << endl;

    // Convert each page of the PDF to a PNG image
    std::string baseName = directory + "/output"; // Append the directory name to the base name
    string command = "pdftoppm -png -r 300 -hide-annotations " + formTitle + " " + baseName;
    system(command.c_str());

    cout << "Converted " << formTitle << " to PNG." << endl;
}

// Extract text from the PDF form
void Form::extractTextFromPDF() {
    // Read the PDF form and extract text from each page
    // DOES NOT USE OCR
    if (!fs::exists(formTitle)) {
        cerr << "File does not exist!" << endl;
        return;
    }

    // Check if document is open
    unique_ptr<document> doc(document::load_from_file(formTitle));
    if (!doc) {
        cerr << "Error loading PDF file!" << endl;
        return;
    }

    string text;
    for (int i = 0; i < doc->pages(); ++i) {
        unique_ptr<page> p(doc->create_page(i));
        if (p) {
            // Convert ustring to UTF-8
            auto utf8_text = p->text().to_utf8();
            std::string page_text(utf8_text.begin(), utf8_text.end());
            this->textPage.push_back(make_pair(page_text, i + 1));
            text += page_text;
        }
    }
    formText = text;
}

/**
 * Check if the extracted text is gibberish (runs on the formText)
 * Sets the isGibberish flag accordingly
 */
void Form::checkIfTextGibberish() {
    if (formText.empty()) {
        cerr << "No text extracted from the form." << endl;
        return;
    }

    std::regex common_words(R"(\b(the|be|to|of|and|a|in|that|have|I)\b)", std::regex_constants::icase);
    std::regex unusual_patterns(R"(\b[^aeiou\s]*[bcdfghjklmnpqrstvwxyz]{5,}[^aeiou\s]*\b|\b[aeiou]{4,}\b)", std::regex_constants::icase);

    // Count common words
    auto words_begin = std::sregex_iterator(formText.begin(), formText.end(), common_words);
    auto words_end = std::sregex_iterator();
    long common_count = std::distance(words_begin, words_end);

    // Find unusual patterns
    auto patterns_begin = std::sregex_iterator(formText.begin(), formText.end(), unusual_patterns);
    auto patterns_end = std::sregex_iterator();
    long unusual_count = std::distance(patterns_begin, patterns_end);

    cout << "Common words: " << common_count << ", Unusual patterns: " << unusual_count << endl;
    isGibberish =  common_count < 25 && unusual_count > 4;

}
/**
 * Perform OCR on the PNG images, pull questions, and add details
 * details include the page number, x, y, width, and height of the question
 */

void Form::cleanText() {
    // regex for cleaning text
    vector<pair<string, int>> elements;
    regex rgx("\\s{2,}|\n|_+");

    for (const auto& [text, page] : textPage) {
        sregex_token_iterator iter(text.begin(), text.end(), rgx, -1);
        sregex_token_iterator end;

        for (; iter != end; ++iter) {
            if (!iter->str().empty()) {  // Check to ensure the string is not empty
                string segment = iter->str();
                size_t pos = segment.find('?');  // Find position of the question mark
                if (pos != string::npos) {
                    // If there is a question mark, take the substring up to the question mark
                    segment = segment.substr(0, pos);
                }
                // Convert segment to lowercase
                transform(segment.begin(), segment.end(), segment.begin(),
                           [](unsigned char c){ return tolower(c); });

                // If the string is no, yes, other, or page... followed by any text, skip it
                regex pattern("_+");
                if (segment == "no" || segment == "yes" || segment == "other" || segment.rfind("page", 0) == 0 || segment.rfind("---", 0) == 0 || segment.length() < 3 || regex_match(segment, pattern)) {
                    continue;
                }

                // remove any leading or trailing whitespace
                segment = regex_replace(segment, regex("^\\s+|\\s+$"), "");
                // Remove any newline characters
                segment = regex_replace(segment, regex("\n"), " ");
                elements.push_back(make_pair(segment, page));
            }
        }
    }
    textPage = elements;
}
/**
 * Convert the form text to individual questions
 * This pulls from the textPage vector and creates Question objects
 * Make sure to have cleaned the text before calling this function
 */
void Form::formTextToQuestions(){
// Split the text into individual questions
    for (const auto& [text, page] : textPage) {
        // Create a new Question object
        Question q(text, page, 0, 0, 0, 0);
        addQuestion(q);
    }
}
/**
 * Compare two strings and return a similarity score
 * @param s1 first string
 * @param s2 second string
 * @return similarity score
 */
double Form::compareStrings(string& s1, string& s2) {
    const int len1 = s1.length(), len2 = s2.length();
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

    for (int i = 0; i <= len1; ++i) dp[i][0] = i;
    for (int j = 0; j <= len2; ++j) dp[0][j] = j;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({ dp[i - 1][j] + 1,  // Deletion
                                  dp[i][j - 1] + 1,  // Insertion
                                  dp[i - 1][j - 1] + cost }); // Substitution
        }
    }
    int dist =  dp[len1][len2];
    int maxLen = std::max(len1, len2);
    return 1.0 - (static_cast<double>(dist) / maxLen);
}


/*
 * This function is called when the program is run for the first time
 * Pulls information from the form, checks if it is gibberish, and if so, uses OCR
 *
 */
void Form::initialRun() {
    this->extractTextFromPDF();
    this->checkIfTextGibberish();

    if (this->isGibberish) {
        // return error message if the text is gibberish
        cerr << this->formTitle << " | Error: Text is gibberish." << endl;
        return;

    }
    this->cleanText();
    this->formTextToQuestions();
}

void Form::printQuestions() {
    // print all questions to console
    for (const auto& question : questions) {
        cout << question.toString();
    }
}

double Form::getPercentQuestionsWithLocations() {
    int numLocations = 0;
    for (const auto& question : questions) {
        if (question.hasLocation) {
            numLocations++;
        }
    }
    return (numLocations / static_cast<double>(numberOfQuestions)) * 100;
}

/**
 * Compare this form to another form
 * @param f the other form to compare to
 */
void Form::compareToAnotherForm(Form& f) {
    for (auto& q1 : questions) {
        for (auto& q2 : f.questions) {
            std::string s1 = q1.text;
            std::string s2 = q2.text;

            // Get similarity
            double s = compareStrings(s1, s2);

            // Add to question with the second question
            q1.addComparison(q2, s);
        }
    }
}

std::string Form::getComparisonString(double s) {
    std::string result;
    for (auto& question : this->questions) {
        std::string temp = "\n\nPage: " + std::to_string(question.page) + "| Question: " + question.text +  "\n";
        temp += "---------------------------------------------------------\n";
        bool addedHeader = false;
        for (const auto& comp : question.topComparisons) {
            const Question& q2 = std::get<0>(comp);
            double score = std::get<1>(comp);
            if (score > s) {
                if (!addedHeader) {
                    result += temp;
                    addedHeader = true;
                }
                result += "Page: " + std::to_string(q2.page) + " | " + q2.text + " (" + std::to_string(score) + ")\n";
            }
        }
    }
    return result;
}
/**
 * Output the comparison to a file
 * @param filename the name of the file to output to
 * @param s the similarity score
 */
void Form::outputComparisonToFile(const std::string& filename, double s) {
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return;
    }

    string delimitaor = "^";

    for (const auto& question : this->questions) {
        for (const auto& comp : question.topComparisons) {
            const Question& q2 = std::get<0>(comp);
            double score = std::get<1>(comp);
            if (score > s) {
                outputFile <<  question.text << delimitaor
                           << question.page << delimitaor
                           << q2.text << delimitaor
                           << q2.page << delimitaor
                           << score << "\n";
            }
        }
    }

    outputFile.close();
}
