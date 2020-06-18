#include "provided.h"
#include <vector>


using namespace std;



class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    const StreetMap* m_StreetMap;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
:   m_StreetMap(sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    // calculate the old crow distance, going between each delivery point
    oldCrowDistance = distanceEarthMiles(depot, deliveries[0].location);
    for (int i = 1; i < deliveries.size(); i++)
        oldCrowDistance += distanceEarthMiles(deliveries[i - 1].location, deliveries[i].location);
    oldCrowDistance += distanceEarthMiles(deliveries[deliveries.size()-1].location, depot);
    
    // create vector of delivery locations, will be ordered correctly
    vector<DeliveryRequest> newOrder;
    
    // order the delivery locations based on proximity to one another, keep adding the location that is closest to the current one
    // finally, return to the depot from the final location
    GeoCoord current = depot;
    double shortestFromDepot = distanceEarthMiles(depot, deliveries[0].location);
    int shortestFromDepotLocationNumber = 0;
    for (int i = 1; i < deliveries.size(); i++) {
        double currentDistance = distanceEarthMiles(depot, deliveries[i].location);
        if (currentDistance < shortestFromDepot) {
            shortestFromDepot = currentDistance;
            shortestFromDepotLocationNumber = i;
        }
    }
    newOrder.push_back(deliveries[shortestFromDepotLocationNumber]);
    current = deliveries[shortestFromDepotLocationNumber].location;
    deliveries.erase(deliveries.begin() + shortestFromDepotLocationNumber);
    
    // add the other locations necessary
    while (deliveries.size() > 1) {
        double shortestDistance = distanceEarthMiles(current, deliveries[0].location);
        int shortestDistancePosition = 0;
        for (int i = 1; i < deliveries.size(); i++) {
            double currentDistance = distanceEarthMiles(current, deliveries[i].location);
            if (currentDistance < shortestDistance) {
                shortestDistance = currentDistance;
                shortestDistancePosition = i;
            }
        }
        newOrder.push_back(deliveries[shortestDistancePosition]);
        current = deliveries[shortestDistancePosition].location;
        deliveries.erase(deliveries.begin() + shortestDistancePosition);
    }
    newOrder.push_back(deliveries[0]);
    deliveries = newOrder;
    
    // calculate the new crow distance, going between each delivery point
    newCrowDistance = distanceEarthMiles(depot, deliveries[0].location);
    for (int i = 1; i < deliveries.size(); i++)
        newCrowDistance += distanceEarthMiles(deliveries[i - 1].location, deliveries[i].location);
    newCrowDistance += distanceEarthMiles(deliveries[deliveries.size()-1].location, depot);
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
