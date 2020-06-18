#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_StreetMap;
    
    // Helper Function
    string turn(double angle) const;
    string direction(double angle) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
:   m_StreetMap(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    // clear any commands that may have been given
    commands.clear();
    
    // if there are no deliveries, we are done
    if (deliveries.empty())
        return DELIVERY_SUCCESS;

    
    double oldCrowDistance;
    double newCrowDistance;
    
    // Reorder the delivery requests for an optimal route (use the DeliveryOptimizer class)
    vector<DeliveryRequest> targetDeliveries = deliveries;
    DeliveryOptimizer optimizer(m_StreetMap);
    optimizer.optimizeDeliveryOrder(depot, targetDeliveries, oldCrowDistance, newCrowDistance);
    
    
    // Generate point to point routes between depot and through each delivery location and back to depot (use the PointToPointRouter class)
    PointToPointRouter router(m_StreetMap);
    
    vector<list<StreetSegment>> routes;
    list<StreetSegment> currentRoute;
    double currentDistanceTravelled;
    
    // check to see if the path from the depot to first delivery location is valid
    DeliveryResult result = router.generatePointToPointRoute(depot, deliveries[0].location, currentRoute, currentDistanceTravelled);
    if (result == BAD_COORD)
        return BAD_COORD;
    if (result == NO_ROUTE)
        return NO_ROUTE;
    routes.push_back(currentRoute);
    
    // check to see if paths between each delivery location are valid
    for (int i = 0; i < targetDeliveries.size() - 1; i++) {
        result = router.generatePointToPointRoute(targetDeliveries[i].location, targetDeliveries[i+1].location, currentRoute, currentDistanceTravelled);
        if (result == BAD_COORD)
            return BAD_COORD;
        if (result == NO_ROUTE)
            return NO_ROUTE;
        routes.push_back(currentRoute);
    }
    
    // check to see if path from final delivery location to depot is valid
    result = router.generatePointToPointRoute(targetDeliveries[targetDeliveries.size() - 1].location, depot, currentRoute, currentDistanceTravelled);
    if (result == BAD_COORD)
        return BAD_COORD;
    if (result == NO_ROUTE)
        return NO_ROUTE;
    routes.push_back(currentRoute);
    
    // before starting the delivery process, total distance is reset to zero
    totalDistanceTravelled = 0;
    
    // loop through each route we will be taking
    for (int i = 0; i < routes.size(); i ++) {
        list<StreetSegment> currentRoute = routes[i];
        
        // loop through the current route
        StreetSegment* prev = nullptr;
        string prevStreet = "";
        double currentDistance = 0;
        string currentDirection;
        for (list<StreetSegment>::iterator p = currentRoute.begin(); p != currentRoute.end(); p++) {

            // determine some features of the current route
            currentDirection = direction(angleOfLine(*p));
            string currentStreet = (*p).name;
            double segmentDistance = distanceEarthMiles((*p).start, (*p).end);
            
            if (prev != nullptr) {

                if (prevStreet == currentStreet) {
                    currentDistance += distanceEarthMiles(prev->end, (*p).end);
                }
                else {
                    string currentTurn = turn(angleBetween2Lines(*p, *prev));
                    if (currentTurn != "error") {
                        DeliveryCommand turnCommand;
                        turnCommand.initAsTurnCommand(currentTurn, currentStreet);
                        commands.push_back(turnCommand);
                    }
                    DeliveryCommand proceedCommand;
                    proceedCommand.initAsProceedCommand(currentDirection, prevStreet, currentDistance);
                    commands.push_back(proceedCommand);
                    currentDistance = 0;
                }
            }
            else {
                currentDistance = distanceEarthMiles((*p).start, (*p).end);
            }

            totalDistanceTravelled += segmentDistance;
            prev = &(*p);
            prevStreet = currentStreet;

            
        }
        
        if (currentDistance != 0) {
            DeliveryCommand proceedCommand;
            proceedCommand.initAsProceedCommand(currentDirection, prevStreet, currentDistance);
            commands.push_back(proceedCommand);
        }
        
        // determine next item to be delivered
        if (i < routes.size() - 1) {
            string item = targetDeliveries[i].item;
            DeliveryCommand deliverCommand;
            deliverCommand.initAsDeliverCommand(item);
            commands.push_back(deliverCommand);
        }
    }
    
    
    return DELIVERY_SUCCESS;
}

// return whether or not the given angle is a left turn, right turn, or not turn at all
string DeliveryPlannerImpl::turn(double angle) const {
    if (angle >= 1 && angle < 180)
        return "left";
    else if (angle >= 180 && angle <= 359)
        return "right";
    else
        return "error";
}

// return the directional label for a given angle of movement
string DeliveryPlannerImpl::direction(double angle) const {
    if (angle >= 0 && angle < 22.5)
        return "east";
    else if (angle >= 22.5 && angle < 67.5)
        return "northeast";
    else if (angle >= 67.5 && angle < 112.5)
        return "north";
    else if (angle >= 112.5 && angle < 157.5)
        return "northwest";
    else if (angle >= 157.5 && angle < 202.5)
        return "west";
    else if (angle >= 202.5 && angle < 247.5)
        return "southwest";
    else if (angle >= 247.5 && angle < 292.5)
        return "south";
    else if (angle >= 292.5 && angle < 337.5)
        return "southeast";
    else
        return "east";
}






//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
