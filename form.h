//
// Created by Riley King on 6/4/24.
//

#ifndef FORM_H
#define FORM_H

#include <vector>
#include <string>

#include "Question.h" // Assuming Question.h is available and correctly defined

class Form {
public:
    std::vector<Question> questions; // Vector of Question objects
    vector<pair<string, int>> textPage; // Text extracted from the form organized by page
    string formText; // All text extracted from the form as a string
    int numberOfPages; // Number of pages in the form
    int numberOfQuestions = 0; // Total number of questions in the form
    std::string formTitle; // Title of the form
    bool isGibberish{}; // Flag to indicate if the form text is gibberish

    Form(std::string  title); // Constructor
    void addQuestion(const Question& q); // Add a Question object to the form
    string getAllQuestions(); // Get a string representation of all questions in the form
    void convertToPNG() const; // Convert the PDF form to PNG images
    void extractTextFromPDF(); // Extract text from the PDF form
    void checkIfTextGibberish(); // Check if the extracted text is gibberish
    void ocrImagesTextExtract(); // Perform OCR on the PNG images, pull questions, and add details
    void cleanText();
    void formTextToQuestions(); // Convert the form text to individual questions
    void initialRun(); // Initial run to extract text and check for gibberish
    void printQuestions(); // Print all questions in the form
    void updateLocations(); // Update the locations of the questions based on the extracted text
    static double compareStrings(string& s1, string& s2); // Compare two strings and return a similarity score
    double getPercentQuestionsWithLocations(); // Get the percentage of questions with locations
    void compareToAnotherForm(Form& f); // Compare this form to another form
    string getComparisonString(double s); // Get a comparison string based on a similarity score
    void outputComparisonToFile(const string& filename, double s); // Output comparison to a file

};

#endif //FORM_H