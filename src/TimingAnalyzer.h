#ifndef TIMING_ANALYZER_H
#define TIMING_ANALYZER_H

#include "Circuit.h"
#include <vector>
#include <string>
#include <map>
#include <queue>
using namespace std;

struct TimingPath {
    vector<string> nodes;
    double totalDelay;
    double slack;
    bool isCritical;
};

class TimingAnalyzer {
private:
    Circuit& circuit;
    vector<TimingPath> allPaths;
    vector<TimingPath> criticalPaths;
    double worstSlack;
    double totalDelay;
    
    // Analysis results
    map<string, double> arrivalTimes;
    map<string, double> requiredTimes;
    map<string, double> slackTimes;
    
public:
    TimingAnalyzer(Circuit& circuit);
    ~TimingAnalyzer() = default;
    
    // Main analysis functions
    void analyze();
    void calculateArrivalTimes();
    void calculateRequiredTimes();
    void calculateSlackTimes();
    void findCriticalPaths();
    void calculateTotalDelay();
    
    // Additional timing analysis
    void calculateSlewTimes();
    void calculateCapacitance();
    void calculateFanoutCounts();
    
    // Report generation
    void generateReport(const string& filename);
    void printSummary();
    void printDetailedReport();
    
    // Getters
    const vector<TimingPath>& getAllPaths() const { return allPaths; }
    const vector<TimingPath>& getCriticalPaths() const { return criticalPaths; }
    double getWorstSlack() const { return worstSlack; }
    double getTotalDelay() const { return totalDelay; }
    
    // Utility functions
    void resetAnalysis();
    bool isTimingViolation() const;
    void printTimingPath(const TimingPath& path) const;
    
private:
    // Helper functions
    void findAllPaths();
    void findPathsRecursive(const string& currentNode, 
                           vector<string>& currentPath,
                           map<string, bool>& visited);
    double calculatePathDelay(const vector<string>& path) const;
    void updateWorstSlack();
    void sortPathsBySlack();
};

#endif // TIMING_ANALYZER_H
