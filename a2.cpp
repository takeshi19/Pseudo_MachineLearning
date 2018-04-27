//////////////////////////////////////////////////////////////////////////////
// File Name:      a2.cpp
//
// Author:         Manuel Takeshi Gomez
// CS email:       gomez@cs.wisc.edu
//
// Description:    The source file for a2.
//
// IMPORTANT NOTE: THIS IS THE ONLY FILE THAT YOU SHOULD MODIFY FOR A2.
//                 You SHOULD NOT MODIFY any of the following:
//                   1. Name of the functions/methods.
//                   2. The number and type of parameters of the functions.
//                   3. Return type of the functions.
////////////////////////////////////////////////////////////////////////////////
#include "a2.hpp"
#include "trim.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/**
 * @brief Calls on the various functions below to clean the text
 *	  data from inFile, into raw sequences of pure words.
 *
 * @param infFile The file to clean up.
 * @param outFile Empty file to write cleansed data to.
 */
void cleanData(std::ifstream &inFile, std::ofstream &outFile,
               std::unordered_set<std::string> &stopwords) {
	
  std::string line; 		    //Each line of the file to clean. 
  std::vector<std::string>::iterator it;
	
  while (std::getline(inFile, line)) {
    replaceHyphensWithSpaces(line);
    std::vector<std::string> instrings;  //Resulting vector after splitLine() call
    splitLine(line, instrings);
	
    std::vector<std::string> outstrings; //Vector of strings to be cleansed.
    removePunctuation(instrings, outstrings);
    removeWhiteSpaces(outstrings);     
    removeEmptyWords(outstrings);
    removeSingleLetterWords(outstrings);
    removeStopWords(outstrings, stopwords);
    
    for (it = outstrings.begin(); it != outstrings.end(); it++) {				      
      outFile << *it << " "; 	     //Writing cleansed data to file.
    }
    outFile << "\n";		     //New line to separate next review.
  }
  outFile.close();  	             //Closing file for proper reaccess.	
}

/**
 * @brief Reads the reviews and ratings from the file and creates a map from
 *        each word to a pair consisting of the total rating for this word and
 *        the total number of times this word has occurred in the reviews.
 *
 * @param file The file with the review data.
 * @param dict The map/dictionary that we are creating
 *             for each word in the review.
 */		
void fillDictionary(std::ifstream &newInFile,
                    std::unordered_map<std::string, std::pair<long, long>> &dict) {
	
  std::string line; //Stores each line of fil.
  std::string word; //Individual word per line.

  int rating = 0;   //Movie rating per line.
  
  while (std::getline(newInFile, line)) {
    std::stringstream ss(line);
    while (ss >> word) {
      if (word.find_first_not_of("0123456789") == std::string::npos) {
	rating = atoi(word.c_str());  //Update the rating.
	continue;		      //Skip to next word after reading in rating #.
      }
      if (dict.find(word) == dict.end())
        dict[word] = std::make_pair(rating, 1);  //1, for 1 current occurrence.
      else {
	dict[word].first += rating;  //Conjoin the ratings.
	dict[word] = std::make_pair(dict[word].first, ++dict[word].second);
      }
    }
  }
}

/**
 * @brief Read the stop words from a file and store them in a set.
 *
 * @param inFile The file with the stopwords.
 * @param stopwords The set to store the stopwords.
 */
void fillStopWords(std::ifstream &inFile,
                   std::unordered_set<std::string> &stopwords) {
	
  std::string line; //Line of text file that contains each stopword.
  while (std::getline(inFile, line))   
    stopwords.insert(line); //Move stopwords from file into unordered set.
}

/**
 * @brief Read the reviews from a file and give a rating for each review.
 *
 * @param testFile The input file that contains the reviews to be rated.
 * @param dict The map/dictionary that we use to rate the reviews.
 * @param ratingsFile The output file into which we write our ratings
 *                    for each review.
 */
void rateReviews(std::ifstream &testFile,
                 std::unordered_map<std::string, std::pair<long, long>> &dict,
                 std::ofstream &ratingsFile) {
  std::string line; //Line per testFile (cleanReviews.txt).
  std::string word; //Word per line of file.
	
  while (std::getline(testFile, line)) {
    std::stringstream ss(line);
    double sum = 0;
    double count = 0;
      
      while (ss >> word) {
	if (dict.find(word) == dict.end())  
	  sum += 2; //Value is always 2 if empty rating or new word.
	else 
	  sum += (dict[word].first*1.0 / dict[word].second);
	count++;	  //Increment count of words found per line.
      }
      //Write this data per line to an output file (round 2 deci-places).
     if (count == 0) {//When while loop doesnt exe and line empty.
	sum += 2;
     	++count;
     }

     ratingsFile << std::setprecision(2) << std::fixed << (sum/count) << "\n";
  }
  ratingsFile.close();
}

/**
 * @brief Removes empty words from vector of strings. E
 * 
 * @param tokens, The vector of strings to remove empty words from.
 */
void removeEmptyWords(std::vector<std::string> &tokens) {

 struct StringIsEmpty {
    bool operator()(const std::string& str) {
        return str.empty();
    }
  };

  tokens.erase(std::remove_if(tokens.begin(), tokens.end(), StringIsEmpty()), tokens.end());
}

/**
 * @brief Removes the punctuations from a vector of strings.
 * 
 * @param inTokens Vector of strings to filter punctuation from. 
 * @param outTokens Vector containing filtered strings.
 */
void removePunctuation(std::vector<std::string> &inTokens,
                       std::vector<std::string> &outTokens) {
 
  std::vector<std::string>::iterator it; //Iterator to the words of file line.
  for (it = inTokens.begin(); it != inTokens.end(); it++) {
    for (int i = 0, len = (*it).size(); i < len; i++) {
      if (ispunct((*it).at(i))) { 
	(*it).erase(i--, 1); //Remove any detect punct. from word.
	len = (*it).size();  //update smaller size of word.
      }
     }
     outTokens.push_back(*it);       //Store the unpunctuated strings to outTokens.	
    }
}

/**
 * @brief Removes single letter strings from the almost clean
 * 	  string vector passed in as a paramter. Ignores 
 *	  ints that are disguised as strings. 
 *
 * @param tokens The string vector to remove single letters from. 
 */
void removeSingleLetterWords(std::vector<std::string> &tokens) {

  for (int i = 0; i < tokens.size(); i++) { //If word is 1 char or empty, remove it.
    if (tokens.at(i).length() == 1) {
      //If end of string is reached & only digits found, then ignore.
      if (!(tokens.at(i).find_first_not_of("0123456789") == std::string::npos))
        tokens.erase(tokens.begin() + i);
    }
  }
}

/**
 * @brief Removes stopwords from a vector of strings.
 * 
 * @param tokens The vector to remove possible stopwords from.
 * @param stopwords The set of stopwords that'll be removed from tokens.
 */  
void removeStopWords(std::vector<std::string> &tokens,
                     std::unordered_set<std::string> &stopwords) {

  //Remove all elements of tokens that are held in stopwords:
  tokens.erase( remove_if (begin(tokens), end(tokens), 
      [&](std::string x) { return find(begin(stopwords), end(stopwords), x) 
          != end(stopwords);}), end(tokens) );
}

/**
 * @brief Removes any type of whitespace from the strings of the
 *  	  given vector.
 *
 * @param tokens String vector of words to remove whitespace from.
 */
void removeWhiteSpaces(std::vector<std::string> &tokens) {
  for (auto it = tokens.begin(); it != tokens.end(); it++)  
    trim(*it);
}

/**
 * @brief All hyphens are replaced with spaces for a given line of the 
 * 	  text file.
 *
 * @param line The line of the file to replace hyphens with.
 */
void replaceHyphensWithSpaces(std::string &line) {
  std::replace(line.begin(), line.end(), '-', ' ');
}

/**
 * @brief Splits a line of text into a vector of strings based
 *	  on the whitespace delimitation between words.
 *
 * @param line The string line per file that's being split.
 * @param words Vector of substrings after splitting.
 */
void splitLine(std::string &line, std::vector<std::string> &words) {
  std::string item; 	      //The substrings were are getting from splitting.
  std::stringstream ss(line);
  
  while (ss >> item) {
    words.push_back(item);
  }
  std::cout <<"Here are all of the strings stored in  splitline words vector: \n";
  for (auto it = words.begin(); it != words.end(); it++) {
	std::cout << *it << ", \n";	
  } 
  std::cout << "**END of splitLine call**\n";
}
