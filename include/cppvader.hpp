#ifndef INCLUDE_CPPVADER_HPP_
#define INCLUDE_CPPVADER_HPP_

#define GOT_HERE std::cout << std::endl << "--> Got to " <<__func__ << ":" << __LINE__ << std::endl

#include <map>
#include <set>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <ostream>

// TODO LIST
// - Change most lower() usages to use pre-made lower container
// - Text emoticions (e.g. :D) are not calculated properly. Investigate.
// - Perhaps some free functions should be class members.

namespace vader
{
  // Anonymous namespace to ensure encapsulation
  namespace
  {
    // SentimentDict struct (output)
    struct SentimentDict
    {
      double compound = 0;
      double pos = 0;
      double neg = 0;
      double neu = 0;

      friend std::ostream &operator<<(std::ostream &os, const SentimentDict &sD);
    };

    // Overloaded << operator for SentimentDict to mimic original
    std::ostream& operator<<(std::ostream &os, const SentimentDict &sD)
    {
      os  << std::setprecision(4);
      os  << '{'
          << "neg: "      << sD.neg       << ", "
          << "neu: "      << sD.neu       << ", "
          << "pos: "      << sD.pos       << ", "
          << "compound: " << sD.compound  
          << "}\n";
      return os;
    }

    // ---------
    // Constants
    // ---------

    // Empirically derived mean sentiment intensity rating increase for exclamation points
    constexpr const size_t limitExclamationMarks = 4;
    constexpr const double intensityModifierExclamationPoint = 0.292;

    // Empirically derived mean sentiment intensity rating increase for question marks
    constexpr const size_t limitQuestionMarks = 3;
    constexpr const double intensityModifierQuestionMark = 0.18;
    constexpr const double upperLimitQuestionMarkModifier = 0.96;

    constexpr const double boostIncr = 0.293;
    constexpr const double boostDecr = -0.293;

    constexpr const double capsIncr = 0.733;
    constexpr const double nScalar = -0.74;

    const std::vector<std::string> negateWords =
    {
      "aint", "arent", "cannot", "cant", "couldnt", "darent", "didnt", "doesnt",
      "ain't", "aren't", "can't", "couldn't", "daren't", "didn't", "doesn't",
      "dont", "hadnt", "hasnt", "havent", "isnt", "mightnt", "mustnt", "neither",
      "don't", "hadn't", "hasn't", "haven't", "isn't", "mightn't", "mustn't",
      "neednt", "needn't", "never", "none", "nope", "nor", "not", "nothing", "nowhere",
      "oughtnt", "shant", "shouldnt", "uhuh", "wasnt", "werent",
      "oughtn't", "shan't", "shouldn't", "uh-uh", "wasn't", "weren't",
      "without", "wont", "wouldnt", "won't", "wouldn't", "rarely", "seldom", "despite"
    };

    const std::map<std::string, double> boosterDict =
    {
      {"absolutely", boostIncr}, {"amazingly", boostIncr}, {"awfully", boostIncr}, 
      {"completely", boostIncr}, {"considerable", boostIncr}, {"considerably", boostIncr}, 
      {"decidedly", boostIncr}, {"deeply", boostIncr}, {"effing", boostIncr}, 
      {"enormous", boostIncr}, {"enormously", boostIncr}, {"entirely", boostIncr}, 
      {"especially", boostIncr}, {"exceptional", boostIncr}, {"exceptionally", boostIncr}, 
      {"extreme", boostIncr}, {"extremely", boostIncr}, {"fabulously", boostIncr}, 
      {"flipping", boostIncr}, {"flippin", boostIncr}, {"frackin", boostIncr}, 
      {"fracking", boostIncr}, {"fricking", boostIncr}, {"frickin", boostIncr}, 
      {"frigging", boostIncr}, {"friggin", boostIncr}, {"fully", boostIncr}, 
      {"fuckin", boostIncr}, {"fucking", boostIncr}, {"fuggin", boostIncr}, 
      {"fugging", boostIncr}, {"greatly", boostIncr}, {"hella", boostIncr}, 
      {"highly", boostIncr}, {"hugely", boostIncr}, {"incredible", boostIncr}, 
      {"incredibly", boostIncr}, {"intensely", boostIncr}, {"major", boostIncr}, 
      {"majorly", boostIncr}, {"more", boostIncr}, {"most", boostIncr}, 
      {"particularly", boostIncr}, {"purely", boostIncr}, {"quite", boostIncr}, 
      {"really", boostIncr}, {"remarkably", boostIncr}, {"so", boostIncr}, 
      {"substantially", boostIncr}, {"thoroughly", boostIncr}, {"total", boostIncr}, 
      {"totally", boostIncr}, {"tremendous", boostIncr}, {"tremendously", boostIncr}, 
      {"uber", boostIncr}, {"unbelievably", boostIncr}, {"unusually", boostIncr}, 
      {"utter", boostIncr}, {"utterly", boostIncr}, {"very", boostIncr}, 
      {"almost", boostDecr}, {"barely", boostDecr}, {"hardly", boostDecr}, 
      {"just enough", boostDecr}, {"kind of", boostDecr}, {"kinda", boostDecr}, 
      {"kindof", boostDecr}, {"kind-of", boostDecr}, {"less", boostDecr}, 
      {"little", boostDecr}, {"marginal", boostDecr}, {"marginally", boostDecr}, 
      {"occasional", boostDecr}, {"occasionally", boostDecr}, {"partly", boostDecr}, 
      {"scarce", boostDecr}, {"scarcely", boostDecr}, {"slight", boostDecr}, 
      {"slightly", boostDecr}, {"somewhat", boostDecr}, {"sort of", boostDecr}, 
      {"sorta", boostDecr}, {"sortof", boostDecr}, {"sort-of", boostDecr}
    };

    const std::map<std::string, int> sentimentIdioms =
    {
      {"cut the mustard", 2}, {"hand to mouth", -2}, {"back handed", -2}, {"blow smoke", -2}, 
      {"blowing smoke", -2}, {"upper hand", 1}, {"break a leg", 2}, {"cooking with gas", 2}, 
      {"in the black", 2}, {"in the red", -2}, {"on the ball", 2}, {"under the weather", -2}
    };

    const std::map<std::string, double> specialCases =
    {
      {"the shit", 3}, {"the bomb", 3}, {"bad ass", 1.5}, {"badass", 1.5}, {"bus stop", 0.0}, 
      {"yeah right", -2}, {"kiss of death", -1.5}, {"to die for", 3}, {"beating heart", 3.1}, 
      {"broken heart", -2.9}
    };

  // ----------------
  // Helper functions
  // ----------------

    // Helper function to check if element is in container
    template <typename T, typename U, typename V>
    bool isIn(const T &container, const U &keyword)
    {
      if (std::find(container.begin(), container.end(), keyword) != container.end())
        return true;
      return false;
    }

    //isIn specialisation for maps
    template <typename T, typename U, typename V>
    bool isIn(const std::map<T, U> &container, const V &keyword)
    {
      return (container.find(keyword) != std::end(container));
    }

    // Helper function that returns lowercase version of string
    std::string lower(const std::string &input)
    {
      std::string lowerCaseWord;
      lowerCaseWord.resize(input.length());
      std::transform(input.begin(), input.end(), lowerCaseWord.begin(), ::tolower);
      return lowerCaseWord;
    }

    // Helper function to check capitalisation word
    // Required by both SentiText and SentimentIntensityAnalyser
    bool fullyUppercase(const std::string &word)
    {
      if (word.empty()) // Not sure if this is desired behaviour
        return false;
      for (auto &character : word)
      {
        if (::islower(character))
          return false;
      }
      return true;
    }
  } // End anonymous namespace

  // ------------------------
  // Original SentiText class
  // ------------------------

  // Identify sentiment-relevant string-level properties of input text.
  // Needs testing for desired output, especially with possible character encoding issues.
  class SentiText
  {
    public:
      // Variables
      const std::vector<std::string> listOfWordsAndEmoticons;
      const bool isCapitalisationDifferent;

      // Constructors
      SentiText() = delete;
      SentiText(const std::string &inputText)
               : listOfWordsAndEmoticons{wordsAndEmoticons(inputText)}
               , isCapitalisationDifferent{allCapDifferential(listOfWordsAndEmoticons)}
               , text(inputText)
      {}

    private:
      const std::string text;
      // Splits text into tokens. Leaves contractions and most emoticons.
      // (Unfortunately, not punctuation + letter ones like :D)
      std::vector<std::string> wordsAndEmoticons(const std::string &inputText);

      // Remove leading and trailing punctuation unless len <= 2, in which case it is likely an emoticon
      std::string stripPunctuationIfWord(const std::string &token);

      // Helper function to split string into words.
      // Note that setting keepEmpty to false also handles multiple consecutive delimiters.
      std::vector<std::string> splitIntoWords(const std::string &inputText, const char &delimiter = ' ', bool keepEmpty = false);

      // Helper function to strip punctuation from string
      // Returns new string
      std::string stripPunctuation(const std::string &inputWord);

      // Helper
      // Check whether some words of the input are fully capitalised
      template <class T>
      bool allCapDifferential(const T &input);
  };

  /* 
  SentiText member implementations
  */

  template <class T>
  bool SentiText::allCapDifferential(const T &input)
  {
    bool isDifferent = false;
    size_t allCapWords = 0;
    for (auto &word : input)
    {
      for (size_t idx = 0; idx != word.size(); ++idx)
      {
        if (fullyUppercase(word))
          ++allCapWords;
      }
    }
    int capDifferential = input.size() - allCapWords;
    if (0 < capDifferential < input.size())
      isDifferent = true;
    return isDifferent;
  }

  std::string SentiText::stripPunctuation(const std::string &inputWord)
  {
    std::string result;
    remove_copy_if(begin(inputWord), end(inputWord), back_inserter(result), [] (char c) { return ispunct(c); });
    return result;
  }

  std::vector<std::string> SentiText::splitIntoWords(const std::string &inputText, const char &delimiter, bool keepEmpty)
  {
    std::vector<std::string> wordList;
    std::stringstream ss(inputText);
    std::string currentWord;

    while ( std::getline(ss, currentWord, delimiter) )
    {
      if (!keepEmpty && !currentWord.empty())
        wordList.push_back(currentWord);
    }
    return wordList;
  }

  std::vector<std::string> SentiText::wordsAndEmoticons(const std::string &inputText)
  {
    std::vector<std::string> splitWords = splitIntoWords(inputText, ' ', false);
    std::vector<std::string> splitAndTrimmedWords;
    for (auto &el: splitWords)
      splitAndTrimmedWords.push_back(stripPunctuationIfWord(el));
    return splitAndTrimmedWords;
  }

  std::string SentiText::stripPunctuationIfWord(const std::string &token)
  {
    std::string strippedWord = stripPunctuation(token);
    if (strippedWord.length() <= 2)
      return token;
    return strippedWord;
  }

  /* 
  Original SentimentIntensityAnalyzer class
  */

  // Thoughts: perhaps I am mixing up lex file and words and emoticons. Should check.
  class SentimentIntensityAnalyser
  {
    public:
      // Constructors
      SentimentIntensityAnalyser(std::filesystem::path customLexicon = "./vader_lexicon.txt", std::filesystem::path customEmojis = "./emoji_utf8_lexicon.txt")
                                : lexDictionary{createLexDictionary(customLexicon)}
                                , emojiDictionary{createEmojiDictionary(customEmojis)}
      {}

      // Return a float for sentiment strength based on the input text.
      // Positive values are positive valence, negative value are negative
      // valence.
      SentimentDict polarityScores(const std::string &inputText);

    private:
      std::filesystem::path lexiconFile;
      // Create lexicon dictionary (word -> sentiment)
      std::map<std::string, double> createLexDictionary(const std::filesystem::path &lexiconFile);
      const std::map<std::string, double> lexDictionary;

      std::filesystem::path emojiFile;
      // Create emoji dictionary (emoji -> description)
      std::map<std::string, std::string> createEmojiDictionary(const std::filesystem::path &emojiFile);
      const std::map<std::string, std::string> emojiDictionary;

      // Needs comment
      SentimentDict scoreValence(const std::vector<double> &sentiments, const std::string &text);

      // Needs comment
      // Also, a lot of this function seems redundant. Why is it necessary?
      const std::vector<double> sentimentValence(double &valence, const SentiText &localSenti, const std::vector<std::string> &wordsAndEmoticonsLower, const std::string &item, size_t idx, std::vector<double> &sentiments);

      double specialIdiomsCheck(double &valence, const std::vector<std::string> &wordsAndEmoticonsLower, size_t idx);

      double negationCheck(double &valence, const std::vector<std::string> &wordsAndEmoticonsLower, size_t startIdx, size_t idx);

      // Helper function to transform vector of words and emojis to lowercase.
      // Note that this seems to keep emojis intact (luckily)
      std::vector<std::string> createLowercaseContainer (const std::vector<std::string> &wordsAndEmoticons);

      // Helper functions
      // Check for added emphasis resulting from exclamation points or question marks
      double amplify(const std::string &text, const char &symbol);

      // Seperates positive versus negative sentiment scores
      std::tuple<double, double, size_t> siftSentimentScores(const std::vector<double> &sentiments);

      // Adds emphasis from exclamation points or question marks
      double punctuationAmplifier(const std::string &text);

      // Check for modifications in sentiment due to constrastive conjunction 'but'
      // Modifies sentiments
      std::vector<double> butCheck(const std::vector<std::string> &wordsAndEmoticonsLower, std::vector<double> &sentiments);

      // Check for negation case using "least"
      // Modifies valence
      double leastCheck(double &valence, const std::vector<std::string> &wordsAndEmoticonsLower, size_t idx);
      
      // Helper function to replace all occurences of a substring
      // Note that this implementation may result in different outcomes from the original. Should be tested.
      void replaceStringInPlace(std::string &subject, const std::string &search, const std::string &replace, bool surroundWithSpaces = true);

      // Helper
      // Check if a certain word increase, decrease or negate valence
      double scalarChange(const std::string &word, double valence, bool capDifferent);

      // Normalise the score to be between -1 and 1 using an alpha that approximates the max expected value
      double normalise(double score, double alpha = 15);

      // Determine if the input contains negation words
      // Needs to use premade lowercase container to be efficient
      template <class T>
      bool negated(const T &input, bool includeNT = true);

      // Helper function to strip leading and trailing spaces
      std::string strip(const std::string &input);

  };

  // ----------------------------------------------------------
  // SentimentIntensityAnalyser member function implementations
  // ----------------------------------------------------------

  inline std::string SentimentIntensityAnalyser::strip(const std::string &input)
  {
    auto start_it = input.begin();
    auto end_it = input.rbegin();
    while (std::isspace(*start_it))
        ++start_it;
    while (std::isspace(*end_it))
        ++end_it;
    return std::string(start_it, end_it.base());
  }

  template <class T>
  inline bool SentimentIntensityAnalyser::negated(const T &input, bool includeNT)
  {
    std::vector<std::string> inputWords;
    for (auto &el : input) // WEIRD
    {
      inputWords.push_back(lower(input));
    }
    for (size_t idx = 0; idx != inputWords.size(); ++idx)
    {
      if (std::find(negateWords.begin(), negateWords.end(), inputWords.at(idx)) != negateWords.end())
        return true;
      if (includeNT && (inputWords.at(idx).find("n't") != std::string::npos))
        return true;
      // if (idx > 0 && inputWords.at(idx) == "least" && inputWords.at(idx - 1) != "at") // Commented out in the original
      //   return true;
    }
    return false;
  }

  inline double SentimentIntensityAnalyser::normalise(double score, double alpha)
  {
    double normalisedScore = score / std::sqrt((score * score) + alpha);

    if (normalisedScore < -1.0)
      return -1.0;
    else if (normalisedScore > 1.0)
      return 1.0;
    else
      return normalisedScore;
  }

  inline double SentimentIntensityAnalyser::scalarChange(const std::string &word, double valence, bool capDifferent)
  {
    double scalar = 0.0;
    std::string wordLower = lower(word);
    // If word is in booster dictionary
    if (isIn(boosterDict, wordLower)) // Needs testing. Expected behaviour likely.
    {
      scalar = boosterDict.at(wordLower);
      if (valence < 0)
        scalar *= -1;
      if (fullyUppercase(word) && capDifferent)
      {
        if (valence > 0)
          scalar += capsIncr;
        else
          scalar -= capsIncr;
      }
    }
    return scalar;
  }

  inline std::vector<std::string> SentimentIntensityAnalyser::createLowercaseContainer (const std::vector<std::string> &wordsAndEmoticons)
  {
    std::vector<std::string> wordsAndEmoticonsLower;
    for (auto &entry : wordsAndEmoticons)
      wordsAndEmoticonsLower.push_back(lower(entry));
    return wordsAndEmoticonsLower;
  }

  inline double SentimentIntensityAnalyser::specialIdiomsCheck(double &valence, const std::vector<std::string> &wordsAndEmoticonsLower, size_t idx)
  {
    const std::string oneZero     = wordsAndEmoticonsLower.at(idx - 1) + ' ' + wordsAndEmoticonsLower.at(idx);
    const std::string twoOneZero  = wordsAndEmoticonsLower.at(idx - 2) + ' ' + wordsAndEmoticonsLower.at(idx - 1) + ' ' + wordsAndEmoticonsLower.at(idx);
    const std::string twoOne      = wordsAndEmoticonsLower.at(idx - 2) + ' ' + wordsAndEmoticonsLower.at(idx - 1);
    const std::string threeTwoOne = wordsAndEmoticonsLower.at(idx - 3) + ' ' + wordsAndEmoticonsLower.at(idx - 2) + ' ' + wordsAndEmoticonsLower.at(idx - 1);
    const std::string threeTwo    = wordsAndEmoticonsLower.at(idx - 3) + ' ' + wordsAndEmoticonsLower.at(idx - 2);

    // Would like to avoid copying strings here
    const std::vector<std::string> sequence = {oneZero, twoOneZero, twoOne, threeTwoOne, threeTwo};

    for (auto &element : sequence)
    {
      if (isIn(specialCases, element))
      {
        valence = specialCases.at(element);
        break;
      }
    }

    if ( (wordsAndEmoticonsLower.size() - 1) > idx)
    {
      const std::string zeroOne = wordsAndEmoticonsLower.at(idx) + ' ' + wordsAndEmoticonsLower.at(idx + 1);
      if (isIn(specialCases, zeroOne))
        valence = specialCases.at(zeroOne);
    }
    if ( (wordsAndEmoticonsLower.size() - 1) > (idx + 1) )
    {
      const std::string zeroOneTwo = wordsAndEmoticonsLower.at(idx) + ' ' + wordsAndEmoticonsLower.at(idx + 1)
                                                                    + ' ' + wordsAndEmoticonsLower.at(idx + 2);
      if (isIn(specialCases, zeroOneTwo))
        valence = specialCases.at(zeroOneTwo);
    }

    // Check for booster/damepener bi-grams such as 'sort of' or 'kind of'
    const std::vector<std::string> nGrams = {threeTwoOne, threeTwo, twoOne}; // Avoid copying?
    for (auto &nGram : nGrams) // Perhaps this is where the error is.
    {
      if (isIn(boosterDict, nGram))
        valence += boosterDict.at(nGram);
    }
    return valence;
  }

  inline double SentimentIntensityAnalyser::negationCheck(double &valence, const std::vector<std::string> &wordsAndEmoticonsLower, size_t startIdx, size_t idx)
  {
    switch (startIdx)
    {
      case 0: 
      {
        if (negated(wordsAndEmoticonsLower.at(idx - (startIdx + 1)))) // 1 word preceding lexicon word (w/o stopwords)
          valence *= nScalar;
        break;
      }
      case 1:
      {
        if (wordsAndEmoticonsLower.at(idx - 2) == "never" && (wordsAndEmoticonsLower.at(idx - 1) == "so" || wordsAndEmoticonsLower.at(idx - 1) == "this"))
          valence *= 1.25;
        else if (wordsAndEmoticonsLower.at(idx - 2) == "without" && wordsAndEmoticonsLower.at(idx - 1) == "doubt")
          valence = valence; // Superfluous?
        else if (negated(wordsAndEmoticonsLower.at(idx - (startIdx + 1)))) // 2 words preceding the lexicon word position
          valence *= nScalar;
        break;
      }
      case 2: // Again, huge mess:
      {
        if (wordsAndEmoticonsLower.at(idx - 3) == "never" 
            && (wordsAndEmoticonsLower.at(idx - 2) == "so" || wordsAndEmoticonsLower.at(idx - 2) == "this") 
            || (wordsAndEmoticonsLower.at(idx - 1) == "so" || wordsAndEmoticonsLower.at(idx - 1) == "this"))
          valence *= 1.25;
        else if (wordsAndEmoticonsLower.at(idx - 3) == "without" && (wordsAndEmoticonsLower.at(idx - 2) == "doubt" || wordsAndEmoticonsLower.at(idx - 1) == "doubt"))
          valence = valence; // Superfluous?
        else if (negated(wordsAndEmoticonsLower.at(idx - (startIdx + 1)))) // Not sure what the bracket situation in the original is here
          valence *= nScalar;
        break;
      }
    }
    return valence;
  }

  inline const std::vector<double> SentimentIntensityAnalyser::sentimentValence(double &valence, const SentiText &localSenti, const std::vector<std::string> &wordsAndEmoticonsLower, const std::string &item, size_t idx, std::vector<double> &sentiments)
  {
    bool isCapDifferent = localSenti.isCapitalisationDifferent;
    const std::vector<std::string> wordsAndEmoticons = localSenti.listOfWordsAndEmoticons;
    std::string itemLower = lower(item);
    if (isIn(lexDictionary, itemLower))
    {
      valence = lexDictionary.at(itemLower); // Get sentiment valence
      // Check for "no" as negation for an adjacent lexicon item vs "no" as its own stand-alone lexicon item
      if (itemLower == "no" && idx != (wordsAndEmoticons.size() - 1) && isIn(lexDictionary, lower(wordsAndEmoticons.at(idx +1))))
      {
        // Don't use valence of "no" as a lexicon item. Instead set it's valence to 0.0 and negate the next item
        valence = 0;
      }
      // This is a mess \/
      if 
      (
        (idx > 0 && lower(wordsAndEmoticons.at(idx - 1)) == "no") 
        || (idx > 1 && lower(wordsAndEmoticons.at(idx - 2)) == "no")
        || (idx > 2 && lower(wordsAndEmoticons.at(idx - 3)) == "no" 
                    && (lower(wordsAndEmoticons.at(idx - 1)) == "or" || lower(wordsAndEmoticons.at(idx - 1)) == "nor"))
      )
        valence = lexDictionary.at(itemLower) * nScalar;

      // Check if sentiment laden word is in ALL CAPS (while others aren't)
      if (fullyUppercase(item) && isCapDifferent)
      {
        if (valence > 0)
          valence += capsIncr;
        else
          valence -= capsIncr;
      }

      // Dampen the scalar modifier of preceding words and emoticons
      // (excluding the ones that immediately preceed the item) based
      // on their distance from the current item.
      for (size_t startIdx = 0; startIdx != 3; ++startIdx)
      {
        if (idx > startIdx && !isIn(lexDictionary, lower(wordsAndEmoticons.at(idx - (startIdx + 1)))))
        {
          double s = scalarChange(wordsAndEmoticons.at(idx - (startIdx + 1)), valence, isCapDifferent);
          if (startIdx == 1 && s != 0)
            s *= 0.95;
          if (startIdx == 2 && s != 0)
            s *= 0.9;
          valence += s;
          valence = negationCheck(valence, wordsAndEmoticonsLower, startIdx, idx);
          if (startIdx == 2)
            valence = specialIdiomsCheck(valence, wordsAndEmoticonsLower, idx);
        }
      }
      valence = leastCheck(valence, wordsAndEmoticons, idx);
    }
    sentiments.push_back(valence);
    return sentiments;
  }

  inline double SentimentIntensityAnalyser::amplify(const std::string &text, const char &symbol)
  {
    size_t nSymbols = std::count(text.begin(), text.end(), symbol);
    switch (symbol)
    {
      case '!':
        return (std::min(nSymbols, limitExclamationMarks) * intensityModifierExclamationPoint);
      case '?':
        if (nSymbols <= 1) // Could be handled by default
          return 0;
        else if (nSymbols <= 3)
          return nSymbols * intensityModifierQuestionMark;
        else 
          return upperLimitQuestionMarkModifier;
      default:
        return 0; // Not sure if desired behaviour
    }
  }

  inline double SentimentIntensityAnalyser::punctuationAmplifier(const std::string &text)
  {
    return amplify(text, '!') + amplify(text,'?');
  }

  inline void SentimentIntensityAnalyser::replaceStringInPlace(std::string &subject, const std::string &search, const std::string &replace, bool surroundWithSpaces) 
  {
    std::string alteredReplace = (surroundWithSpaces ? std::string(" " + replace + " ") : replace);
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) 
    {
        subject.replace(pos, search.length(), alteredReplace);
        pos += alteredReplace.length();
    }
  }

  inline std::vector<double> SentimentIntensityAnalyser::butCheck(const std::vector<std::string> &wordsAndEmoticonsLower, std::vector<double> &sentiments)
  {
    const auto butLocation = std::find(wordsAndEmoticonsLower.begin(), wordsAndEmoticonsLower.end(), "but");
    if (butLocation != wordsAndEmoticonsLower.end())
    {
      size_t butIndex = std::distance(wordsAndEmoticonsLower.begin(), butLocation);
      for (size_t idx = 0; idx != butIndex; ++idx)
        sentiments.at(idx) *= 0.5;
      for (size_t idx = butIndex + 1; idx != sentiments.size(); ++idx)
        sentiments.at(idx) *= 1.5;
    }
    return sentiments;
  }

  // This function feels like it has a lot of redundancies in it, but is true to the original
  inline double SentimentIntensityAnalyser::leastCheck(double &valence, const std::vector<std::string> &wordsAndEmoticonsLower, size_t idx)
  {
    if (idx > 1 && !isIn(this->lexDictionary, wordsAndEmoticonsLower.at(idx - 1)) && wordsAndEmoticonsLower.at(idx - 1) == "least")
    {
      if (wordsAndEmoticonsLower.at(idx - 2) != "at" && wordsAndEmoticonsLower.at(idx - 2) != "very")
        valence *= nScalar;
    }
    else if (idx > 0 && !isIn(this->lexDictionary, wordsAndEmoticonsLower.at(idx - 1)) && wordsAndEmoticonsLower.at(idx - 1) == "least")
      valence *= nScalar;
    return valence;
  }

  inline std::map<std::string, double> SentimentIntensityAnalyser::createLexDictionary(const std::filesystem::path &lexiconFile)
  {
    std::map<std::string, double> lexDictionary;
    std::ifstream lexFile(lexiconFile);
    if (lexFile.fail())
    {
      throw std::runtime_error ("Unable to access lexFile.");
      return lexDictionary;
    }
    std::string line;
    while (std::getline(lexFile, line))
    {
      std::string entry;
      std::string sMeasure;
      std::stringstream lineStream(line);
      std::getline(lineStream, entry, '\t');
      std::getline(lineStream, sMeasure, '\t');
      lexDictionary.insert( {entry, stod(sMeasure)} );
    }
    if (lexDictionary.empty())
    {
      throw std::runtime_error ("Empty dictionary. Something went wrong reading the lexFile.");
      return lexDictionary;
    }
    return lexDictionary;
  }

  inline std::map<std::string, std::string> SentimentIntensityAnalyser::createEmojiDictionary(const std::filesystem::path &emojiFile)
  {
    std::map<std::string, std::string> emojiDictionary;
    std::ifstream emojiLex(emojiFile);
    if (emojiLex.fail())
    {
      throw std::runtime_error ("Unable to access emojiFile.");
      return emojiDictionary;
    }    
    std::string line;
    while (std::getline(emojiLex, line))
    {
      std::string emoji;
      std::string description;
      std::stringstream lineStream(line);
      std::getline(lineStream, emoji, '\t');
      std::getline(lineStream, description, '\t');
      emojiDictionary.insert( {emoji, description} );
    }
    if (lexDictionary.empty())
    {
      throw std::runtime_error ("Empty dictionary. Something went wrong reading the emojiFile.");
      return emojiDictionary;
    }
    return emojiDictionary;
  }

  std::tuple<double, double, size_t> SentimentIntensityAnalyser::siftSentimentScores(const std::vector<double> &sentiments)
  {
    double posSum = 0, negSum = 0;
    size_t neuCount = 0;
    for (auto &sentiment : sentiments)
    {
      if (sentiment > 0)
        posSum += (sentiment + 1);
      if (sentiment < 0)
        negSum += (sentiment - 1);
      if (sentiment == 0)
        neuCount += 1;
    }
    return std::make_tuple(posSum, negSum, neuCount);
  }

  inline SentimentDict SentimentIntensityAnalyser::scoreValence(const std::vector<double> &sentiments, const std::string &text)
  {
    double pos = 0, neg = 0, neu = 0, compound = 0;
    if (!sentiments.empty())
    {
      double sumSentiments = 0;
      for (auto &sentiment : sentiments)
        sumSentiments += sentiment;

      // Compute and add emphasis from punctuation in text
      double punctuationEmphasisAmplifier = punctuationAmplifier(text);
      if (sumSentiments > 0)
        sumSentiments += punctuationEmphasisAmplifier;
      else if (sumSentiments < 0)
        sumSentiments -= punctuationEmphasisAmplifier;

      compound = normalise(sumSentiments);
      
      // Discriminate between positive, negative and neutral sentiment scores
      double posSum, negSum;
      size_t neuCount;
      std::tie(posSum, negSum, neuCount) = siftSentimentScores(sentiments);

      if (posSum > fabs(negSum))
        posSum += punctuationEmphasisAmplifier;
      else if (posSum < fabs(negSum)) // DRY
        negSum -= punctuationEmphasisAmplifier;
      
      double total = posSum + fabs(negSum) + neuCount;
      pos = fabs(posSum / total);
      neg = fabs(negSum / total);
      neu = fabs(neuCount / total);
    }
    else
    {
      compound = 0, pos = 0, neg = 0, neu = 0; // DRY
    }
    return SentimentDict {compound, pos, neg, neu}; // Not rounded at this point. Not sure why it should be.
  }

  inline SentimentDict SentimentIntensityAnalyser::polarityScores(const std::string &inputText)
  {
    // Convert emojis to textual descriptions
    // Note that emojis are not single chars, so an alternative to the original is needed.
    // Not sure if this is the best option.
    std::string textWithoutEmojis = inputText;
    bool prevSpace = true;
    for (auto &emojiEntry : emojiDictionary) // Not sure if this is efficient
      replaceStringInPlace(textWithoutEmojis, emojiEntry.first, emojiEntry.second, true);
    std::string text = strip(textWithoutEmojis);

    SentiText interSentiText(text);

    const std::vector<std::string> wordsAndEmoticons = interSentiText.listOfWordsAndEmoticons;
    const std::vector<std::string> wordsAndEmoticonsLower = createLowercaseContainer(wordsAndEmoticons);

    std::vector<double> sentiments;

    for (size_t idx = 0; idx != wordsAndEmoticons.size(); ++idx)
    {
      double valence = 0;
      const std::string &elLower = wordsAndEmoticonsLower.at(idx);
      if (isIn(boosterDict, elLower))
      {
        sentiments.push_back(valence);
        continue;
      }
      if (idx < (wordsAndEmoticons.size() - 1) && elLower == "kind" && (wordsAndEmoticonsLower.at(idx + 1)) == "of")
      {
        sentiments.push_back(valence);
        continue;
      }
      sentiments = sentimentValence(valence, interSentiText, wordsAndEmoticonsLower, wordsAndEmoticons.at(idx), idx, sentiments);
    }
    sentiments = butCheck(wordsAndEmoticonsLower, sentiments);

    SentimentDict valenceDictionary = scoreValence(sentiments, text);

    return valenceDictionary;
  }
}

#endif // INCLUDE_CPPVADER_HPP_