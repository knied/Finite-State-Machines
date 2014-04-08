//
//  example.cpp
//  FSM
//
//  Created by Kristof Niederholtmeyer on 05.04.14.
//  Copyright (c) 2014 Kristof Niederholtmeyer. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "NFA.h"
#include "DFA.h"
#include "Evaluator.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char * argv[])
{
    // A range of characters
    typedef ActionRange<char> CharRange;
    // A group of character ranges
    typedef ActionFilter<char> CharFilter;
    
    // A nondeterministic finite automat working on characters
    typedef NFA<char> CharNFA;
    // A deterministic finite automat working on characters
    typedef DFA<char> CharDFA;
    
    // Evaluators
    typedef Evaluator<CharNFA> CharNFAEvaluator;
    typedef Evaluator<CharDFA> CharDFAEvaluator;
    
    // Defining some ranges of characters
    CharRange character(0x21, 0x7E);
    CharRange upper('A', 'Z');
    CharRange lower('a', 'z');
    CharRange digit('0', '9');
    
    // Combine ranges
    CharFilter alpha = upper + lower;
    CharFilter alphanumeric = alpha + digit;
    CharFilter special = character - alphanumeric;
    CharFilter whitespace = CharFilter(0x09) + CharFilter(0x0A) + CharFilter(0x0D) + CharFilter(0x20);
    
    std::cout << "alpha: " << alpha << std::endl;
    std::cout << "alphanumeric: " << alphanumeric << std::endl;
    std::cout << "special: " << special << std::endl;
    
    // A number representing nondeterministic finite automat.
    CharNFA number_nfa;
    number_nfa.add_transition(0, CharFilter('-'), 1);
    number_nfa.add_transition(0, 1); // epsilon transition
    number_nfa.add_transition(1, digit, 1);
    number_nfa.add_transition(1, 2); // epsilon transition
    number_nfa.add_transition(1, CharFilter('.'), 2);
    number_nfa.add_transition(2, digit, 3);
    number_nfa.add_transition(3, digit, 3);
    number_nfa.add_transition(3, CharFilter('e') + CharFilter('E'), 4);
    number_nfa.add_transition(4, CharFilter('-'), 5);
    number_nfa.add_transition(4, 5); // epsilon transition
    number_nfa.add_transition(5, digit, 6);
    number_nfa.add_transition(6, digit, 6);
    number_nfa.set_accepting_states({3, 6});
    
    // Automata can be visualized by graphviz
    std::cout << std::endl;
    std::cout << number_nfa.graphviz("number_nfa") << std::endl;
    
    // Create a deterministic finite automat from the nondeterministic automat
    CharDFA number_dfa(number_nfa);
    
    std::cout << std::endl;
    std::cout << number_dfa.graphviz("number_dfa") << std::endl;
    
    // Evaluation (works for both, NFAs and DFAs)
    CharNFAEvaluator evaluator_nfa(number_nfa);
    std::string string;
    char c = 0;
    bool cont = false;
    do {
        std::cout << "========================================" << std::endl;
        std::cout << "String: " << string << std::endl;
        std::cout << "Accepted (nfa): " << (evaluator_nfa.accepted() ? "true" : "false") << std::endl;
        std::cout << "Next character: ";
        std::cin >> c;
        std::cout << std::endl;
        
        cont = evaluator_nfa.perform(c);
        if (cont) {
            string += c;
        }
    } while (cont);
    std::cout << "Unexpected character: " << c << std::endl;
    std::cout << "Final String: " << string << std::endl;
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
