#include "provided.h"
#include <list>
using namespace std;

#include "ExpandableHashMap.h"
#include <queue>


class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_StreetMap;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
:   m_StreetMap(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    // clear the given variables of any past values
    route.clear();
    totalDistanceTravelled = 0;
    
    // if start equals end, the delivery is done (no route needed)
    if (start == end) {
        return DELIVERY_SUCCESS;
    }

    vector<StreetSegment> segsToCheckBadCoord;
    
    // check to see if start and end coordinates are valid
    if(!m_StreetMap->getSegmentsThatStartWith(end, segsToCheckBadCoord))
        return BAD_COORD;
    if(!m_StreetMap->getSegmentsThatStartWith(start, segsToCheckBadCoord))
        return BAD_COORD;
    
    GeoCoord current = start;
    
    // create a queue for our search of valid routes
    queue<GeoCoord> coords;
    coords.push(current);
    vector<GeoCoord> visited;
    
    // create map to record the route we choose to travel and later set
    ExpandableHashMap<GeoCoord, GeoCoord> locationOfPreviousWayPoint;
    
    // loop through the available paths in a circular manner (dictated by the use of a queue)
    bool pathFound = false;
    while (!coords.empty()) {
        // access the current coordinate off the queue
        current = coords.front();
        coords.pop();
        // if the current coordinate being checked is the end, the route is finished
        if (current == end) {
            pathFound = true;
            break;
        }
        vector<StreetSegment> segs;
        m_StreetMap->getSegmentsThatStartWith(current, segs);
        visited.push_back(current);
        // loop through potential paths off of the current coordinate
        for (int i = 0; i < segs.size(); i++) {
            // make sure the potential paths does not lead to a coordinate already visited
            bool notVisited = true;
            for (int j = 0; j < visited.size() && notVisited == true; j++) {
                if (segs[i].end == visited[j]) {
                    notVisited = false;
                }
            }
            if (notVisited) {
                coords.push(segs[i].end);
                locationOfPreviousWayPoint.associate(segs[i].end, current);
            }
        }
    }
    
    // if a valid path was found, construct the route
    if (pathFound) {
        list<StreetSegment> optimalRoute;
        
        // obtain end point and previous point
        GeoCoord endUse = end;
        current = endUse;
        GeoCoord prev = endUse;
        // loop until we reach the starting location
        while (prev != start) {
            GeoCoord* prevPointer = locationOfPreviousWayPoint.find(endUse);
            prev = *prevPointer;
            
            // construct street segment the two points make up and add it to the route
            string streetName;
            vector<StreetSegment> segs;
            m_StreetMap->getSegmentsThatStartWith(prev, segs);
            for (int i = 0; i < segs.size(); i++) {
                if (segs[i].end == endUse) {
                    streetName = segs[i].name;
                    break;
                }
            }
            // add found segment to the route
            optimalRoute.push_back(StreetSegment(prev, endUse, streetName));
            
            endUse = prev;
        }
        
        // flip route, since it was built back to front
        optimalRoute.reverse();
        
        // if route is empty, no route was found
        if (optimalRoute.empty())
            return NO_ROUTE;
        
        // set route to the route we found
        route = optimalRoute;
        
        // delivery was successful
        return DELIVERY_SUCCESS;
    }
    
    return NO_ROUTE;

}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
