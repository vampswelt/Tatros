#pragma once
#include "graph.h"
using namespace std;
class CostModel {
public:
    virtual ~CostModel() = default;

    
    virtual double compute(const Edge& e) const = 0;

    virtual string name() const = 0;

    
    void applyToGraph(Graph& g) const;
};


class DistanceCost : public CostModel {
public:
    double      compute(const Edge& e) const override;
    string name()    const override { return "distance"; }
};


class TimeCost : public CostModel {
public:
    double      compute(const Edge& e) const override;
    string name()    const override { return "time"; }
};


class HybridCost : public CostModel {
public:
    explicit HybridCost(double alpha = 0.5) : alpha_(alpha) {}
    double      compute(const Edge& e) const override;
    string name()    const override { return "hybrid"; }
private:
    double alpha_;
};

CostModel* makeCostModel(const std::string& mode, double alpha = 0.5);
