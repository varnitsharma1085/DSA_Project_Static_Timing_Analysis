#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <memory>

#include "Node.h"
#include "Gate.h"
#include "Circuit.h"
#include "TimingAnalyzer.h"

using namespace std;

// ============================================================================
// NODE IMPLEMENTATION
// ============================================================================

Node::Node(const string& name, bool isInput, bool isOutput) 
    : name(name), isPrimaryInput(isInput), isPrimaryOutput(isOutput),
      arrivalTimeRise(0.0), arrivalTimeFall(0.0),
      requiredTimeRise(0.0), requiredTimeFall(0.0),
      slackRise(0.0), slackFall(0.0),
      slewRise(0.0), slewFall(0.0),
      capacitance(0.0), fanoutCount(0) {
}

double Node::getMaxArrivalTime() const {
    return max(arrivalTimeRise, arrivalTimeFall);
}

double Node::getMinRequiredTime() const {
    return min(requiredTimeRise, requiredTimeFall);
}

double Node::getWorstSlack() const {
    return min(slackRise, slackFall);
}

void Node::resetTiming() {
    arrivalTimeRise = 0.0;
    arrivalTimeFall = 0.0;
    requiredTimeRise = 0.0;
    requiredTimeFall = 0.0;
    slackRise = 0.0;
    slackFall = 0.0;
    slewRise = 0.0;
    slewFall = 0.0;
}

void Node::printTiming() const {
    cout << fixed << setprecision(3);
    cout << "Node: " << name << endl;
    cout << "  Arrival Time (Rise/Fall): " << arrivalTimeRise << " / " << arrivalTimeFall << " ns" << endl;
    cout << "  Required Time (Rise/Fall): " << requiredTimeRise << " / " << requiredTimeFall << " ns" << endl;
    cout << "  Slack (Rise/Fall): " << slackRise << " / " << slackFall << " ns" << endl;
    cout << "  Slew (Rise/Fall): " << slewRise << " / " << slewFall << " ns" << endl;
    cout << "  Capacitance: " << capacitance << " fF" << endl;
    cout << "  Fanout Count: " << fanoutCount << endl;
    cout << "  Worst Slack: " << getWorstSlack() << " ns" << endl;
}

// ============================================================================
// CIRCUIT IMPLEMENTATION
// ============================================================================

Circuit::Circuit() : clockPeriod(1.0) {
    
}

void Circuit::loadCircuit(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open circuit file: " + filename);
    }

    string line;
    while (getline(file, line)) {
        
        if (line.empty() || line[0] == '#') continue;
        
        istringstream iss(line);
        string command;
        iss >> command;
        
        if (command == "CLOCK_PERIOD") {
            iss >> clockPeriod;
        }
        else if (command == "INPUT") {
            string inputName;
            while (iss >> inputName) {
                addNode(inputName, true, false);
                primaryInputs.push_back(inputName);
            }
        }
        else if (command == "OUTPUT") {
            string outputName;
            while (iss >> outputName) {
                addNode(outputName, false, true);
                primaryOutputs.push_back(outputName);
            }
        }
        else if (command == "GATE") {
            string gateType, gateName, outputName;
            vector<string> inputs;
            
            iss >> gateType >> gateName >> outputName;
            
            string input;
            while (iss >> input) {
                inputs.push_back(input);
            }
            
            addGate(gateType, gateName, inputs, outputName);
        }
    }
    
    file.close();
    
    if (!validateCircuit()) {
        throw runtime_error("Invalid circuit configuration");
    }
}

void Circuit::loadDelays(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open delay file: " + filename);
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        istringstream iss(line);
        string gateType;
        double delay;
        
        if (iss >> gateType >> delay) {
            gateDelays[gateType] = delay;
        }
    }
    
    file.close();
}

void Circuit::addNode(const string& name, bool isInput, bool isOutput) {
    if (nodes.find(name) == nodes.end()) {
        nodes[name] = make_shared<Node>(name, isInput, isOutput);
    }
}

void Circuit::addGate(const string& type, const string& name, 
                     const vector<string>& inputs, const string& output) {
    // Ensure all nodes exist
    for (const auto& input : inputs) {
        addNode(input);
    }
    addNode(output);
    
    // Create gate
    auto gate = GateFactory::createGate(type, name, inputs, output);
    if (gate) {
        gates.push_back(gate);
        
        // Connect nodes
        for (const auto& input : inputs) {
            nodes[input]->addFanout(gate);
        }
        nodes[output]->setFanin(gate);
    }
}

shared_ptr<Node> Circuit::getNode(const string& name) {
    auto it = nodes.find(name);
    return (it != nodes.end()) ? it->second : nullptr;
}

double Circuit::getGateDelay(const string& gateType) const {
    auto it = gateDelays.find(gateType);
    return (it != gateDelays.end()) ? it->second : 0.0;
}

void Circuit::printCircuit() const {
    cout << "\n=== Circuit Information ===" << endl;
    cout << "Clock Period: " << clockPeriod << " ns" << endl;
    
    cout << "\nPrimary Inputs: ";
    for (const auto& input : primaryInputs) {
        cout << input << " ";
    }
    cout << endl;
    
    cout << "Primary Outputs: ";
    for (const auto& output : primaryOutputs) {
        cout << output << " ";
    }
    cout << endl;
    
    cout << "\nGates:" << endl;
    for (const auto& gate : gates) {
        cout << "  " << gate->getType() << " " << gate->getName() 
                  << " -> " << gate->getOutput() << endl;
    }
    
    cout << "\nGate Delays:" << endl;
    for (const auto& delay : gateDelays) {
        cout << "  " << delay.first << ": " << delay.second << " ns" << endl;
    }
}

bool Circuit::validateCircuit() const {
    // Checking if all gates have valid inputs and output
    for (const auto& gate : gates) {
        for (const auto& input : gate->getInputs()) {
            if (nodes.find(input) == nodes.end()) {
                cerr << "Error: Gate " << gate->getName() 
                          << " references undefined input " << input << endl;
                return false;
            }
        }
        
        if (nodes.find(gate->getOutput()) == nodes.end()) {
            cerr << "Error: Gate " << gate->getName() 
                      << " references undefined output " << gate->getOutput() << endl;
            return false;
        }
    }
    
    return true;
}
