#ifndef GATE_H
#define GATE_H

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;
class Gate {
private:
    string type;
    string name;
    vector<string> inputs;
    string output;
    double delay;

    bool checkMinimumInputs(int count) const;

public:
    Gate(const string& type,
         const string& name,
         const vector<string>& inputs,
         const string& output,
         double delay = 0.0);

    // Getters
    const string& getType() const { return type; }
    const string& getName() const { return name; }
    const vector<string>& getInputs() const { return inputs; }
    const string& getOutput() const { return output; }
    double getDelay() const { return delay; }

    // Setters
    void setDelay(double newDelay) { delay = newDelay; }

    // Behavior helpers
    bool evaluate(const vector<bool>& inputValues) const;
    string getTruthTable() const;
    int getExpectedInputCount() const;
    bool isValidInputCount(int count) const;

    // Timing helpers
    double calculateDelay(const vector<double>& inputArrivalTimes) const;
    double calculateSlew(const vector<double>& inputSlews) const;
    double calculateCapacitance() const;

    // Utility
    void printGate() const;
};

class GateFactory {
public:
    static shared_ptr<Gate> createGate(const string& type,
                                            const string& name,
                                            const vector<string>& inputs,
                                            const string& output);
};

// ===== Implementation =======================================================

Gate::Gate(const string& typeValue,
           const string& nameValue,
           const vector<string>& inputList,
           const string& outputValue,
           double delayValue)
    : type(typeValue),
      name(nameValue),
      inputs(inputList),
      output(outputValue),
      delay(delayValue) {
}

bool Gate::checkMinimumInputs(int count) const {
    return count >= 2;
}

bool Gate::evaluate(const vector<bool>& inputValues) const {
    if (type == "AND") {
        if (!checkMinimumInputs(static_cast<int>(inputValues.size()))) return false;
        bool result = true;
        for (auto value : inputValues) {
            result = result && value;
        }
        return result;
    }

    if (type == "OR") {
        if (!checkMinimumInputs(static_cast<int>(inputValues.size()))) return false;
        bool result = false;
        for (auto value : inputValues) {
            result = result || value;
        }
        return result;
    }

    if (type == "XOR") {
        if (inputValues.size() != 2) return false;
        return inputValues[0] != inputValues[1];
    }

    if (type == "NAND") {
        if (!checkMinimumInputs(static_cast<int>(inputValues.size()))) return false;
        bool result = true;
        for (auto value : inputValues) {
            result = result && value;
        }
        return !result;
    }

    if (type == "NOR") {
        if (!checkMinimumInputs(static_cast<int>(inputValues.size()))) return false;
        bool result = false;
        for (auto value : inputValues) {
            result = result || value;
        }
        return !result;
    }

    if (type == "XNOR") {
        if (inputValues.size() != 2) return false;
        return inputValues[0] == inputValues[1];
    }

    if (type == "NOT") {
        if (inputValues.size() != 1) return false;
        return !inputValues[0];
    }

    if (type == "HALF_ADDER") {
        if (inputValues.size() != 2) return false;
        return inputValues[0] != inputValues[1];
    }

    if (type == "FULL_ADDER") {
        if (inputValues.size() != 3) return false;
        bool result = inputValues[0];
        for (size_t i = 1; i < inputValues.size(); ++i) {
            result = result != inputValues[i];
        }
        return result;
    }

    if (type == "MUX2TO1") {
        if (inputValues.size() != 3) return false;
        return inputValues[2] ? inputValues[1] : inputValues[0];
    }

    if (type == "MUX_SWITCH") {
        if (inputValues.size() != 2) return false;
        return inputValues[1] ? inputValues[0] : false;
    }

    return false;
}

string Gate::getTruthTable() const {
    if (type == "AND") return "AND: 0&0=0, 0&1=0, 1&0=0, 1&1=1";
    if (type == "OR") return "OR: 0|0=0, 0|1=1, 1|0=1, 1|1=1";
    if (type == "XOR") return "XOR: 0^0=0, 0^1=1, 1^0=1, 1^1=0";
    if (type == "NAND") return "NAND: !(0&0)=1, !(0&1)=1, !(1&0)=1, !(1&1)=0";
    if (type == "NOR") return "NOR: !(0|0)=1, !(0|1)=0, !(1|0)=0, !(1|1)=0";
    if (type == "XNOR") return "XNOR: 0==0=1, 0==1=0, 1==0=0, 1==1=1";
    if (type == "NOT") return "NOT: !0=1, !1=0";
    if (type == "HALF_ADDER") return "HALF_ADDER: A,B -> Sum,Carry";
    if (type == "FULL_ADDER") return "FULL_ADDER: A,B,Cin -> Sum,Carry";
    if (type == "MUX2TO1") return "MUX2TO1: S=0->D0, S=1->D1";
    if (type == "MUX_SWITCH") return "MUX_SWITCH: S=0->0, S=1->D";
    return "UNKNOWN";
}

int Gate::getExpectedInputCount() const {
    if (type == "NOT") return 1;
    if (type == "HALF_ADDER") return 2;
    if (type == "FULL_ADDER") return 3;
    if (type == "MUX2TO1") return 3;
    if (type == "MUX_SWITCH") return 2;
    if (type == "XOR") return 2;
    if (type == "XNOR") return 2;
    return 2;
}

bool Gate::isValidInputCount(int count) const {
    if (type == "AND") return count >= 2;
    if (type == "OR") return count >= 2;
    if (type == "NAND") return count >= 2;
    if (type == "NOR") return count >= 2;
    if (type == "XOR" || type == "XNOR") return count == 2;
    if (type == "NOT") return count == 1;
    if (type == "HALF_ADDER") return count == 2;
    if (type == "FULL_ADDER") return count == 3;
    if (type == "MUX2TO1") return count == 3;
    if (type == "MUX_SWITCH") return count == 2;
    return true;
}

double Gate::calculateDelay(const vector<double>& inputArrivalTimes) const {
    if (inputArrivalTimes.empty()) return delay;
    auto maxValue = *max_element(inputArrivalTimes.begin(), inputArrivalTimes.end());
    return maxValue + delay;
}

double Gate::calculateSlew(const vector<double>& inputSlews) const {
    if (inputSlews.empty()) return 0.0;
    auto maxValue = *max_element(inputSlews.begin(), inputSlews.end());
    return maxValue + delay * 0.1;
}

double Gate::calculateCapacitance() const {
    return 1.0 + static_cast<double>(inputs.size()) * 0.5;
}

void Gate::printGate() const {
    cout << type << " " << name << " (";
    for (size_t i = 0; i < inputs.size(); ++i) {
        cout << inputs[i];
        if (i < inputs.size() - 1) {
            cout << ", ";
        }
    }
    cout << ") -> " << output << " [delay: " << delay << " ns]" << endl;
}

shared_ptr<Gate> GateFactory::createGate(const string& type,
                                              const string& name,
                                              const vector<string>& inputs,
                                              const string& output) {
    static const vector<string> knownTypes = {
        "AND", "OR", "XOR", "NAND", "NOR", "XNOR",
        "NOT", "HALF_ADDER", "FULL_ADDER", "MUX2TO1", "MUX_SWITCH"
    };

    if (find(knownTypes.begin(), knownTypes.end(), type) == knownTypes.end()) {
        cerr << "Unknown gate type: " << type << endl;
        return nullptr;
    }

    auto gate = make_shared<Gate>(type, name, inputs, output);
    if (!gate->isValidInputCount(static_cast<int>(inputs.size()))) {
        cerr << "Invalid input count for gate " << name << " of type " << type << endl;
        return nullptr;
    }
    return gate;
}

#endif // GATE_H
