#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>
#include <memory>
class Gate;
using namespace std;
class Node {
private:
    string name;
    bool isPrimaryInput;
    bool isPrimaryOutput;
    shared_ptr<Gate> fanin;  // Gate that drives this node
    vector<shared_ptr<Gate>> fanouts;  // Gates driven by this node
    
    // Timing information
    double arrivalTimeRise;
    double arrivalTimeFall;
    double requiredTimeRise;
    double requiredTimeFall;
    double slackRise;
    double slackFall;
    
    // Additional timing parameters
    double slewRise;
    double slewFall;
    double capacitance;
    int fanoutCount;

public:
    Node(const string& name, bool isInput = false, bool isOutput = false);
    ~Node() = default;

    // Getters
    const string& getName() const { return name; }
    bool isInput() const { return isPrimaryInput; }
    bool isOutput() const { return isPrimaryOutput; }
    
    // Timing getters
    double getArrivalTimeRise() const { return arrivalTimeRise; }
    double getArrivalTimeFall() const { return arrivalTimeFall; }
    double getRequiredTimeRise() const { return requiredTimeRise; }
    double getRequiredTimeFall() const { return requiredTimeFall; }
    double getSlackRise() const { return slackRise; }
    double getSlackFall() const { return slackFall; }
    
    // Additional parameter getters
    double getSlewRise() const { return slewRise; }
    double getSlewFall() const { return slewFall; }
    double getCapacitance() const { return capacitance; }
    int getFanoutCount() const { return fanoutCount; }
    
    // Timing setters
    void setArrivalTimeRise(double time) { arrivalTimeRise = time; }
    void setArrivalTimeFall(double time) { arrivalTimeFall = time; }
    void setRequiredTimeRise(double time) { requiredTimeRise = time; }
    void setRequiredTimeFall(double time) { requiredTimeFall = time; }
    void setSlackRise(double slack) { slackRise = slack; }
    void setSlackFall(double slack) { slackFall = slack; }
    
    // Additional parameter setters
    void setSlewRise(double slew) { slewRise = slew; }
    void setSlewFall(double slew) { slewFall = slew; }
    void setCapacitance(double cap) { capacitance = cap; }
    void setFanoutCount(int count) { fanoutCount = count; }
    
    // Connection management
    void setFanin(shared_ptr<Gate> gate) { fanin = gate; }
    void addFanout(shared_ptr<Gate> gate) { fanouts.push_back(gate); }
    shared_ptr<Gate> getFanin() const { return fanin; }
    const vector<shared_ptr<Gate>>& getFanouts() const { return fanouts; }
    
    // Utility functions
    double getMaxArrivalTime() const;
    double getMinRequiredTime() const;
    double getWorstSlack() const;
    void resetTiming();
    void printTiming() const;
};

#endif // NODE_H
