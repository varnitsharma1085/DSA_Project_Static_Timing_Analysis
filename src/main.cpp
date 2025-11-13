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


// ============================================================================
// TIMING ANALYZER IMPLEMENTATION
// ============================================================================

TimingAnalyzer::TimingAnalyzer(Circuit& circuit) 
    : circuit(circuit), worstSlack(0.0), totalDelay(0.0) {
}

void TimingAnalyzer::analyze() {
    cout << "Starting Static Timing Analysis..." << endl;
    
    // Reset previous analysis
    resetAnalysis();
    
    // Step 1: Calculate arrival times (forward propagation)
    cout << "Calculating arrival times..." << endl;
    calculateArrivalTimes();
    
    // Step 2: Calculate required times (backward propagation)
    cout << "Calculating required times..." << endl;
    calculateRequiredTimes();
    
    // Step 3: Calculate slack times
    cout << "Calculating slack times..." << endl;
    calculateSlackTimes();
    
    // Step 4: Find all timing paths
    cout << "Finding timing paths..." << endl;
    findAllPaths();
    
    // Step 5: Identify critical paths
    cout << "Identifying critical paths..." << endl;
    findCriticalPaths();
    
    // Step 6: Calculate total delay
    calculateTotalDelay();
    
    // Step 7: Additional analysis
    calculateSlewTimes();
    calculateCapacitance();
    calculateFanoutCounts();
    
    cout << "Timing analysis completed!" << endl;
}

void TimingAnalyzer::calculateArrivalTimes() {
    // Initialize arrival times for primary inputs
    for (const auto& input : circuit.getPrimaryInputs()) {
        auto node = circuit.getNode(input);
        if (node) {
            node->setArrivalTimeRise(0.0);
            node->setArrivalTimeFall(0.0);
            arrivalTimes[input] = 0.0;
        }
    }
    
    // Topological sort for forward propagation
    queue<string> processQueue;
    map<string, int> inDegree;
    
    // Calculate in-degrees
    for (const auto& gate : circuit.getGates()) {
        for (const auto& input : gate->getInputs()) {
            inDegree[input]++;
        }
    }
    
    // Add primary inputs to queue
    for (const auto& input : circuit.getPrimaryInputs()) {
        processQueue.push(input);
    }
    
    // Process nodes in topological order
    while (!processQueue.empty()) {
        string currentNode = processQueue.front();
        processQueue.pop();
        
        auto node = circuit.getNode(currentNode);
        if (!node) continue;
        
        // Process all fanout gates
        for (const auto& gate : node->getFanouts()) {
            // Calculate arrival time at gate output
            vector<double> inputArrivalTimes;
            for (const auto& input : gate->getInputs()) {
                auto inputNode = circuit.getNode(input);
                if (inputNode) {
                    inputArrivalTimes.push_back(inputNode->getMaxArrivalTime());
                }
            }
            
            // Set gate delay from circuit configuration
            double gateDelay = circuit.getGateDelay(gate->getType());
            gate->setDelay(gateDelay);
            double outputArrivalTime = gate->calculateDelay(inputArrivalTimes);
            
            // Update output node
            string outputNode = gate->getOutput();
            auto outputNodePtr = circuit.getNode(outputNode);
            if (outputNodePtr) {
                outputNodePtr->setArrivalTimeRise(outputArrivalTime);
                outputNodePtr->setArrivalTimeFall(outputArrivalTime);
                arrivalTimes[outputNode] = outputArrivalTime;
            }
            
            // Decrease in-degree for output node
            inDegree[outputNode]--;
            if (inDegree[outputNode] == 0) {
                processQueue.push(outputNode);
            }
        }
    }
}

void TimingAnalyzer::calculateRequiredTimes() {
    // Initialize required times for primary outputs
    double clockPeriod = circuit.getClockPeriod();
    for (const auto& output : circuit.getPrimaryOutputs()) {
        auto node = circuit.getNode(output);
        if (node) {
            node->setRequiredTimeRise(clockPeriod);
            node->setRequiredTimeFall(clockPeriod);
            requiredTimes[output] = clockPeriod;
        }
    }
    
    // Backward propagation using reverse topological order
    queue<string> processQueue;
    map<string, int> outDegree;
    
    // Calculate out-degrees
    for (const auto& gate : circuit.getGates()) {
        outDegree[gate->getOutput()]++;
    }
    
    // Add primary outputs to queue
    for (const auto& output : circuit.getPrimaryOutputs()) {
        processQueue.push(output);
    }
    
    // Process nodes in reverse topological order
    while (!processQueue.empty()) {
        string currentNode = processQueue.front();
        processQueue.pop();
        
        auto node = circuit.getNode(currentNode);
        if (!node) continue;
        
        // Process fanin gate
        auto faninGate = node->getFanin();
        if (faninGate) {
            double gateDelay = circuit.getGateDelay(faninGate->getType());
            double inputRequiredTime = node->getMinRequiredTime() - gateDelay;
            
            // Update input nodes
            for (const auto& input : faninGate->getInputs()) {
                auto inputNode = circuit.getNode(input);
                if (inputNode) {
                    inputNode->setRequiredTimeRise(inputRequiredTime);
                    inputNode->setRequiredTimeFall(inputRequiredTime);
                    requiredTimes[input] = inputRequiredTime;
                }
                
                // Decrease out-degree for input node
                outDegree[input]--;
                if (outDegree[input] == 0) {
                    processQueue.push(input);
                }
            }
        }
    }
}

void TimingAnalyzer::calculateSlackTimes() {
    for (const auto& nodePair : circuit.getNodes()) {
        const string& nodeName = nodePair.first;
        auto node = nodePair.second;
        
        double arrivalTime = node->getMaxArrivalTime();
        double requiredTime = node->getMinRequiredTime();
        double slack = requiredTime - arrivalTime;
        
        node->setSlackRise(slack);
        node->setSlackFall(slack);
        slackTimes[nodeName] = slack;
    }
    
    updateWorstSlack();
}

void TimingAnalyzer::findAllPaths() {
    allPaths.clear();
    
    for (const auto& input : circuit.getPrimaryInputs()) {
        vector<string> currentPath;
        map<string, bool> visited;
        findPathsRecursive(input, currentPath, visited);
    }
}

void TimingAnalyzer::findPathsRecursive(const string& currentNode, 
                                       vector<string>& currentPath,
                                       map<string, bool>& visited) {
    if (visited[currentNode]) return; // Avoid cycles
    
    visited[currentNode] = true;
    currentPath.push_back(currentNode);
    
    auto node = circuit.getNode(currentNode);
    if (!node) return;
    
    // If this is a primary output, we found a complete path
    if (node->isOutput()) {
        TimingPath path;
        path.nodes = currentPath;
        path.totalDelay = calculatePathDelay(currentPath);
        path.slack = node->getWorstSlack();
        path.isCritical = (path.slack <= 0.0);
        allPaths.push_back(path);
    } else {
        // Continue with fanout gates
        for (const auto& gate : node->getFanouts()) {
            string nextNode = gate->getOutput();
            findPathsRecursive(nextNode, currentPath, visited);
        }
    }
    
    // Backtrack
    visited[currentNode] = false;
    currentPath.pop_back();
}

double TimingAnalyzer::calculatePathDelay(const vector<string>& path) const {
    double totalDelay = 0.0;
    
    for (size_t i = 0; i < path.size() - 1; ++i) {
        auto node = circuit.getNode(path[i]);
        if (!node) continue;
        
        for (const auto& gate : node->getFanouts()) {
            if (gate->getOutput() == path[i + 1]) {
                totalDelay += circuit.getGateDelay(gate->getType());
                break;
            }
        }
    }
    
    return totalDelay;
}

void TimingAnalyzer::findCriticalPaths() {
    criticalPaths.clear();
    
    for (const auto& path : allPaths) {
        if (path.isCritical) {
            criticalPaths.push_back(path);
        }
    }
    
    // Sort by slack (most critical first)
    sort(criticalPaths.begin(), criticalPaths.end(),
              [](const TimingPath& a, const TimingPath& b) {
                  return a.slack < b.slack;
              });
}

void TimingAnalyzer::calculateTotalDelay() {
    totalDelay = 0.0;
    
    for (const auto& path : allPaths) {
        totalDelay = max(totalDelay, path.totalDelay);
    }
}

void TimingAnalyzer::calculateSlewTimes() {
    // Simple slew calculation 
    for (const auto& nodePair : circuit.getNodes()) {
        auto node = nodePair.second;
        double arrivalTime = node->getMaxArrivalTime();
        node->setSlewRise(arrivalTime * 0.1); // 10% of arrival time
        node->setSlewFall(arrivalTime * 0.1);
    }
}

void TimingAnalyzer::calculateCapacitance() {
    for (const auto& nodePair : circuit.getNodes()) {
        auto node = nodePair.second;
        double capacitance = 1.0 + node->getFanouts().size() * 0.5;
        node->setCapacitance(capacitance);
    }
}

void TimingAnalyzer::calculateFanoutCounts() {
    for (const auto& nodePair : circuit.getNodes()) {
        auto node = nodePair.second;
        node->setFanoutCount(node->getFanouts().size());
    }
}



void TimingAnalyzer::updateWorstSlack() {
    worstSlack = 0.0;
    
    for (const auto& slackPair : slackTimes) {
        worstSlack = min(worstSlack, slackPair.second);
    }
}

void TimingAnalyzer::resetAnalysis() {
    allPaths.clear();
    criticalPaths.clear();
    arrivalTimes.clear();
    requiredTimes.clear();
    slackTimes.clear();
    worstSlack = 0.0;
    totalDelay = 0.0;
    
    // Reset node timing
    for (const auto& nodePair : circuit.getNodes()) {
        nodePair.second->resetTiming();
    }
}

bool TimingAnalyzer::isTimingViolation() const {
    return worstSlack < 0.0;
}

void TimingAnalyzer::generateReport(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot create report file: " + filename);
    }
    
    file << fixed << setprecision(3);
    file << "===========================================" << endl;
    file << "        STATIC TIMING ANALYSIS REPORT" << endl;
    file << "===========================================" << endl;
    file << endl;
    
    // Summary
    file << "SUMMARY:" << endl;
    file << "--------" << endl;
    file << "Clock Period: " << circuit.getClockPeriod() << " ns" << endl;
    file << "Total Delay: " << totalDelay << " ns" << endl;
    file << "Worst Slack: " << worstSlack << " ns" << endl;
    file << "Timing Violation: " << (isTimingViolation() ? "YES" : "NO") << endl;
    file << "Number of Paths: " << allPaths.size() << endl;
    file << "Critical Paths: " << criticalPaths.size() << endl;
    file << endl;
    
    // Node timing information
    file << "NODE TIMING INFORMATION:" << endl;
    file << "-----------------------" << endl;
    for (const auto& nodePair : circuit.getNodes()) {
        const string& name = nodePair.first;
        auto node = nodePair.second;
        
        file << "Node: " << name << endl;
        file << "  Arrival Time: " << node->getMaxArrivalTime() << " ns" << endl;
        file << "  Required Time: " << node->getMinRequiredTime() << " ns" << endl;
        file << "  Slack: " << node->getWorstSlack() << " ns" << endl;
        file << "  Slew: " << node->getSlewRise() << " ns" << endl;
        file << "  Capacitance: " << node->getCapacitance() << " fF" << endl;
        file << "  Fanout: " << node->getFanoutCount() << endl;
        file << endl;
    }
    
    // Critical paths
    if (!criticalPaths.empty()) {
        file << "CRITICAL PATHS:" << endl;
        file << "---------------" << endl;
        for (size_t i = 0; i < criticalPaths.size(); ++i) {
            const auto& path = criticalPaths[i];
            file << "Path " << (i + 1) << " (Slack: " << path.slack << " ns):" << endl;
            for (size_t j = 0; j < path.nodes.size(); ++j) {
                file << "  " << path.nodes[j];
                if (j < path.nodes.size() - 1) file << " -> ";
            }
            file << endl;
            file << "  Total Delay: " << path.totalDelay << " ns" << endl;
            file << endl;
        }
    }
    
    file.close();
}

void TimingAnalyzer::printSummary() {
    cout << fixed << setprecision(3);
    cout << "\n=== TIMING ANALYSIS SUMMARY ===" << endl;
    cout << "Clock Period: " << circuit.getClockPeriod() << " ns" << endl;
    cout << "Total Delay: " << totalDelay << " ns" << endl;
    cout << "Worst Slack: " << worstSlack << " ns" << endl;
    cout << "Timing Violation: " << (isTimingViolation() ? "YES" : "NO") << endl;
    cout << "Number of Paths: " << allPaths.size() << endl;
    cout << "Critical Paths: " << criticalPaths.size() << endl;
    
    if (!criticalPaths.empty()) {
        cout << "\nMost Critical Path:" << endl;
        printTimingPath(criticalPaths[0]);
    }
}

void TimingAnalyzer::printDetailedReport() {
    printSummary();
    
    cout << "\n=== DETAILED NODE TIMING ===" << endl;
    for (const auto& nodePair : circuit.getNodes()) {
        nodePair.second->printTiming();
        cout << endl;
    }
}

void TimingAnalyzer::printTimingPath(const TimingPath& path) const {
    cout << "Path (Slack: " << path.slack << " ns): ";
    for (size_t i = 0; i < path.nodes.size(); ++i) {
        cout << path.nodes[i];
        if (i < path.nodes.size() - 1) cout << " -> ";
    }
    cout << " (Delay: " << path.totalDelay << " ns)" << endl;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    // Hardcoded file paths - we can change these to use different input files
    string circuitFile = "../examples/complex_circuit.txt";
    string delayFile = "../delays/gate_delays.txt";
    string outputFile = "../reports/timing_report.txt";
    // Create circuit and load configuration
   
    try {
        // Creating circuit and load configuration
        Circuit circuit;
        circuit.loadCircuit(circuitFile);
        circuit.loadDelays(delayFile);

        // Creating timing analyzer
        TimingAnalyzer analyzer(circuit);

        // Performing timing analysis
        cout << "Performing Static Timing Analysis..." << endl;
        analyzer.analyze();

        // Generating timing report
        analyzer.generateReport(outputFile);
        cout << "Timing analysis completed. Report saved to: " << outputFile << endl;

        // Printing summary to console
        analyzer.printSummary();

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
