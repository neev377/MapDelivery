#include "provided.h"
#include <string>
#include <vector>
#include <functional>

#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "ExpandableHashMap.h"

using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
    
    bool find(const GeoCoord& gc);
    
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> data;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}


bool StreetMapImpl::find(const GeoCoord& gc) {
    
    vector<StreetSegment>* found = data.find(gc);
    
    if (found == nullptr)
        return false;
    return true;
}

bool StreetMapImpl::load(string mapFile)
{
    // if file is empty, return false
    if (mapFile == "")
        return false;
    
    // convert file to useable format
    ifstream file(mapFile);
    std::string line;

    // iterate through the entire file, each street and its segments
    int count = 0;
    while (std::getline(file, line))
    {
        // record street name
        string streetName = line;
        
        // record number of segments for this street
        getline(file, line);
        int nSegments = stoi(line);
        
        // loop through the street's segments
        for (int i = 0; i < nSegments; i++) {
            
            // use vector to save the coordinate values of the segment
            vector<string> coordValues;
            
            // get next segment's line
            getline(file, line);
            
            // loop through four values of the segment's coordinates
            int charCount = 0;
            string number;
            for (int j = 0; j < 4; j++) {
                // record each value needed for the coordinates
                while (line[charCount] != ' ' && charCount < line.size()) {
                    number += line[charCount];
                    charCount++;
                }
                coordValues.push_back(number);
                charCount++;
                number = "";
            }
            
            // create geocoordinates for the ends of the street segment
            GeoCoord start(coordValues[0], coordValues[1]);
            GeoCoord end(coordValues[2], coordValues[3]);

            // create forward and reverse format of the street segment
            StreetSegment forward(start, end, streetName);
            StreetSegment reverse(end, start, streetName);
            
            // add the forward segment to the data map
            vector<StreetSegment>* findStart = data.find(start);
            if (findStart != nullptr) {
                findStart->push_back(forward);
            }
            else {
                vector<StreetSegment> startValues;
                startValues.push_back(forward);
                data.associate(start, startValues);
            }
            
            // add the reverse segment to the data map
            vector<StreetSegment>* findEnd = data.find(end);
            if (findEnd != nullptr) {
                findEnd->push_back(reverse);
            }
            else {
                vector<StreetSegment> endValues;
                endValues.push_back(reverse);
                data.associate(end, endValues);
            }
        }
    }

    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    // search through data map for segments linked to given gc
    const vector<StreetSegment>* gcValues = data.find(gc);
    
    // if no segments were found, return false
    if (gcValues == nullptr)
        return false;
    
    // clear segs of any previous data
    segs.clear();
    
    // add found segments to segs
    for (int i = 0; i < gcValues->size(); i++) {
        StreetSegment* s = new StreetSegment((*gcValues)[i].start, (*gcValues)[i].end, (*gcValues)[i].name);
        segs.push_back(*s);
    }
    
    return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}

bool StreetMap::find(const GeoCoord& g) {
    return m_impl->find(g);
}
