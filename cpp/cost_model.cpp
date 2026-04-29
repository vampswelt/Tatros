#include "cost_model.h"

double computeCost(const Edge& e, double slopeWeight) {
    return e.distance + slopeWeight * std::abs(e.slope);
}