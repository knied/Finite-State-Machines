//
//  Evaluator.h
//  FSM
//
//  Created by Kristof Niederholtmeyer on 05.04.14.
//  Copyright (c) 2014 Kristof Niederholtmeyer. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __Parser__Evaluator__
#define __Parser__Evaluator__

////////////////////////////////////////////////////////////////////////////////

#include <iostream>

////////////////////////////////////////////////////////////////////////////////

template <typename FSM>
class Evaluator {
    typedef typename FSM::EvaluationState State;
    typedef typename FSM::EvaluationAction Action;
    
    FSM const& _fsm;
    State _state;
    
public:
    Evaluator(FSM const& fsm)
    : _fsm(fsm), _state(fsm.initial()) {}
    
    // Perform an action on the automat. Returns false if the action is not
    // accepted. In this case the internal state stays unchanged.
    bool perform(Action const& action) {
        return _fsm.successor(_state, action, _state);
    }
    
    // Returns true if the automat is currently in an accepting state.
    bool accepted() const {
        return _fsm.accepted(_state);
    }
    
    // Reset the automat to its initial state.
    void reset() {
        _state = _fsm.initial();
    }
    
    State const& state() const {
        return _state;
    }
}; // Evaluator

////////////////////////////////////////////////////////////////////////////////

#endif /* defined(__Parser__Evaluator__) */

////////////////////////////////////////////////////////////////////////////////
