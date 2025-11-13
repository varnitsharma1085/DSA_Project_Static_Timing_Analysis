#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Gate.h"
#include "Node.h"

using namespace std;
class Circuit {
private:
    map<string, shared_ptr<Node>> nodes;
    vector<shared_ptr<Gate>> gates;
    map<string, double> gateDelays;
    vector<string> primaryInputs;
    vector<string> primaryOutputs;
    double clockPeriod;

public:
    Circuit();
    ~Circuit() = default;

    // File I/O
    void loadCircuit(const string& filename);
    void loadDelays(const string& filename);
    
    // Circuit construction
    void addNode(const string& name, bool isInput = false, bool isOutput = false);
    void addGate(const string& type, const string& name, 
                const vector<string>& inputs, const string& output);
    
    // Getters
    map<string, shared_ptr<Node>>& getNodes() { return nodes; }
    vector<shared_ptr<Gate>>& getGates() { return gates; }
    vector<string>& getPrimaryInputs() { return primaryInputs; }
    vector<string>& getPrimaryOutputs() { return primaryOutputs; }
    double getClockPeriod() const { return clockPeriod; }
    void setClockPeriod(double period) { clockPeriod = period; }
    
    // Utility functions
    shared_ptr<Node> getNode(const string& name);
    double getGateDelay(const string& gateType) const;
    void printCircuit() const;
    
    // Validation
    bool validateCircuit() const;
};

#endif // CIRCUIT_H
