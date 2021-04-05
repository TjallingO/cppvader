#include <vector>
#include <string>
#include "../include/cppvader.hpp"
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"

const std::vector<std::string> sentences = 
{
  "VADER is smart, handsome, and funny.",  // positive sentence example
  "VADER is smart, handsome, and funny!",                 // punctuation emphasis handled correctly (sentiment intensity adjusted)
  "VADER is very smart, handsome, and funny.",                 // booster words handled correctly (sentiment intensity adjusted)
  "VADER is VERY SMART, handsome, and FUNNY.",  // emphasis for ALLCAPS handled
  "VADER is VERY SMART, handsome, and FUNNY!!!",                 // combination of signals - VADER appropriately adjusts intensity
  "VADER is VERY SMART, uber handsome, and FRIGGIN FUNNY!!!",                 // booster words & punctuation make this close to ceiling for score
  "VADER is not smart, handsome, nor funny.",  // negation sentence example
  "The book was good.",  // positive sentence
  "At least it isn't a horrible book.",  // negated negative sentence with contraction
  "The book was only kind of good.",                 // qualified positive sentence is handled correctly (intensity adjusted)
  "The plot was good, but the characters are uncompelling and the dialog is not great.",                 // mixed negation sentence
  "Today SUX!",  // negative slang with capitalization emphasis
  "Today only kinda sux! But I'll get by, lol",                  // mixed sentiment example with slang and constrastive conjunction "but"
  "Make sure you :) or :D today!",  // emoticons handled
  "Catch utf-8 emoji such as 💘 and 💋 and 😁",  // emojis handled
  "Not bad at all",  // Capitalized negation
  // Tricky sentences
  "Sentiment analysis has never been good.",
  "Sentiment analysis has never been this good!",
  "Most automated sentiment analysis tools are shit.",
  "With VADER, sentiment analysis is the shit!",
  "Other sentiment analysis tools can be quite bad.",
  "On the other hand, VADER is quite bad ass",
  "VADER is such a badass!",
  "Without a doubt, excellent idea.",
  "Roger Dodger is one of the most compelling variations on this theme.",
  "Roger Dodger is at least compelling as a variation on the theme.",
  "Roger Dodger is one of the least compelling variations on this theme.",
  "Not such a badass after all.",
  "Without a doubt, an excellent idea."
};

const std::vector<vader::SentimentDict> expectedOutcomes =
{
  {.compound = 0.8316, .pos = 0.746,.neg = 0.0, .neu = 0.254},
  {.compound = 0.8439, .pos = 0.752,.neg = 0.0, .neu = 0.248},
  {.compound = 0.8545, .pos = 0.701,.neg = 0.0, .neu = 0.299},
  {.compound = 0.9227, .pos = 0.754,.neg = 0.0, .neu = 0.246},
  {.compound = 0.9342, .pos = 0.767,.neg = 0.0, .neu = 0.233},
  {.compound = 0.9469, .pos = 0.706,.neg = 0.0, .neu = 0.294},
  {.compound = -0.7424, .pos = 0.0,.neg = 0.646, .neu = 0.354},
  {.compound = 0.4404, .pos = 0.492,.neg = 0.0, .neu = 0.508},
  {.compound = 0.431, .pos = 0.322,.neg = 0.0, .neu = 0.678},
  {.compound = 0.3832, .pos = 0.303,.neg = 0.0, .neu = 0.697},
  {.compound = -0.7042, .pos = 0.094,.neg = 0.327, .neu = 0.579},
  {.compound = -0.5461, .pos = 0.0,.neg = 0.779, .neu = 0.221},
  {.compound = 0.5249, .pos = 0.317,.neg = 0.127, .neu = 0.556},
  {.compound = 0.8633, .pos = 0.706,.neg = 0.0, .neu = 0.294},
  {.compound = 0.875, .pos = 0.417,.neg = 0.0, .neu = 0.583},
  {.compound = 0.431, .pos = 0.487,.neg = 0.0, .neu = 0.513},
  // Tricky sentences
  {.compound = -0.3412, .pos = 0.0, .neg = 0.325, .neu = 0.675},
  {.compound = 0.5672, .pos = 0.379, .neg = 0.0, .neu = 0.621},
  {.compound = -0.5574, .pos = 0.0, .neg = 0.375, .neu = 0.625},
  {.compound = 0.6476, .pos = 0.417, .neg = 0.0, .neu = 0.583},
  {.compound = -0.5849, .pos = 0.0, .neg = 0.351, .neu = 0.649},
  {.compound = 0.802, .pos = 0.577, .neg = 0.0, .neu = 0.423},
  {.compound = 0.4003, .pos = 0.402, .neg = 0.0, .neu = 0.598},
  {.compound = 0.7013, .pos = 0.659, .neg = 0.0, .neu = 0.341},
  {.compound = 0.2944, .pos = 0.166, .neg = 0.0, .neu = 0.834},
  {.compound = 0.2263, .pos = 0.147, .neg = 0.0, .neu = 0.853},
  {.compound = -0.1695, .pos = 0.0, .neg = 0.132, .neu = 0.868},
  {.compound = -0.2584, .pos = 0.0, .neg = 0.289, .neu = 0.711},
  {.compound = 0.7013, .pos = 0.592, .neg = 0.0, .neu = 0.408}
};



TEST_CASE( "Results are compared against the original outcomes.")
{
  vader::SentimentIntensityAnalyser analyser;
  size_t idx = 0;
  
  for (auto sentence : sentences)
  {
    vader::SentimentDict currentResult = analyser.polarityScores(sentence);
    INFO("Sentence: " + sentence);
    CHECK(currentResult.compound == Approx(expectedOutcomes.at(idx).compound).margin(0.00005));
    CHECK(currentResult.pos == Approx(expectedOutcomes.at(idx).pos).margin(0.0005));
    CHECK(currentResult.neu == Approx(expectedOutcomes.at(idx).neu).margin(0.0005));
    CHECK(currentResult.neg == Approx(expectedOutcomes.at(idx).neg).margin(0.0005));
    ++idx;
  }
  BENCHMARK("Analysing all sentences.")
  {
    for (auto &sentence : sentences)
      analyser.polarityScores(sentence);
  };
}