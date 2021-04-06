# cppvader
This is a single-header C++ port of the [Valence Aware Dictionary and sEntiment Reasoner (VADER)](https://github.com/cjhutto/vaderSentiment) sentiment analysis tool, written in Python. At this stage, it should be considered a work-in-progress. More specifically:

1. It compiles.
2. It analyses sentences, however:
  - The results do not always match those of the original tool.
  - The syntax, naming and construction match those of the original tool very closely.
    - This implies that the format of this code differs from established C++ best practices.

## Progress
### 2020-04-07

| # Tests | Passed | Failed |
| :------ | :----: | :----: |
|   116   |   109  |    7   |

Failed cases:

| Sentence | Outcome | Expected |
| :------- | :-----: | :------: |
| Today only kinda sux! But I'll get by, lol | **0.2966**, **0.3625**, **0.3409**, **0.4086** | **0.127**, **0.556**, **0.317**, **0.5249** |
| Make sure you :) or :D today!              | 0.0, **0.2786**, **0.7214**, **0.8849**        | 0.0, **0.294**, **0.706**, **0.8633**       |

    
## Minimal working example
```c++
#include "include/cppvader.hpp"

int main()
{
  vader::SentimentIntensityAnalyser analyser;
  const std::vector<std::string> sentences = 
  {
    "VADER is smart, handsome, and funny.", // positive sentence example
    "VADER is smart, handsome, and funny!", // punctuation emphasis handled correctly (sentiment intensity adjusted)
    "VADER is very smart, handsome, and funny.",  // booster words handled correctly (sentiment intensity adjusted)
    "VADER is VERY SMART, handsome, and FUNNY.",   // emphasis for ALLCAPS handled
    "VADER is VERY SMART, handsome, and FUNNY!!!",  // combination of signals - VADER appropriately adjusts intensity
    "VADER is VERY SMART, uber handsome, and FRIGGIN FUNNY!!!",  // booster words & punctuation make this close to ceiling for score
    "VADER is not smart, handsome, nor funny.",  // negation sentence example
    "The book was good.",  // positive sentence
    "At least it isn't a horrible book.",  // negated negative sentence with contraction
    "The book was only kind of good.",  // qualified positive sentence is handled correctly (intensity adjusted)
    "The plot was good, but the characters are uncompelling and the dialog is not great.",  // mixed negation sentence
    "Today SUX!",  // negative slang with capitalization emphasis
    "Today only kinda sux! But I'll get by, lol",  // mixed sentiment example with slang and constrastive conjunction "but"
    "Make sure you :) or :D today!",  // emoticons handled
    "Catch utf-8 emoji such as 💘 and 💋 and 😁",  // emojis handled
    "Not bad at all"  // Capitalized negation
  };

  for (auto &sentence : sentences)
  {
    std::cout.width(80);
    std::cout << sentence << "\t\t" << std::right << analyser.polarityScores(sentence);
  }
}
```

Output:
```
                              VADER is smart, handsome, and funny.    {neg: 0, neu: 0.2542, pos: 0.7458, compound: 0.8316}
                              VADER is smart, handsome, and funny!    {neg: 0, neu: 0.2481, pos: 0.7519, compound: 0.8439}
                          VADER is very smart, handsome, and funny.   {neg: 0, neu: 0.2991, pos: 0.7009, compound: 0.8545}
                          VADER is VERY SMART, handsome, and FUNNY.   {neg: 0, neu: 0.2459, pos: 0.7541, compound: 0.9227}
                        VADER is VERY SMART, handsome, and FUNNY!!!   {neg: 0, neu: 0.2333, pos: 0.7667, compound: 0.9342}
          VADER is VERY SMART, uber handsome, and FRIGGIN FUNNY!!!    {neg: 0, neu: 0.294, pos: 0.706, compound: 0.9469}
                          VADER is not smart, handsome, nor funny.    {neg: 0.6458, neu: 0.3542, pos: 0, compound: -0.7424}
                                                The book was good.    {neg: 0, neu: 0.5085, pos: 0.4915, compound: 0.4404}
                                At least it isn't a horrible book.    {neg: 0, neu: 0.678, pos: 0.322, compound: 0.431}
                                    The book was only kind of good.   {neg: 0, neu: 0.6971, pos: 0.3029, compound: 0.3832}
(...) the characters are uncompelling and the dialog is not great.    {neg: 0.3274, neu: 0.5786, pos: 0.09402, compound: -0.7042}
                                                        Today SUX!    {neg: 0.779, neu: 0.221, pos: 0, compound: -0.5461}
                        Today only kinda sux! But I'll get by, lol    {neg: 0.2966, neu: 0.3625, pos: 0.3409, compound: 0.4086}
                                      Make sure you :) or :D today!   {neg: 0, neu: 0.2786, pos: 0.7214, compound: 0.8849}
                        Catch utf-8 emoji such as 💘 and 💋 and 😁   {neg: 0, neu: 0.5833, pos: 0.4167, compound: 0.875}
                                                    Not bad at all    {neg: 0, neu: 0.5128, pos: 0.4872, compound: 0.431}
```

