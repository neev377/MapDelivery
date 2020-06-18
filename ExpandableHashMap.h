

// ExpandableHashMap.h

//#ifndef ExpandableHashMap
//#define ExpandableHashMap

#include <vector>
#include <list>
#include "provided.h"

using namespace std;

const int INITIAL_NUMBER_OF_BUCKETS = 8;

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);
    unsigned int getBucketNumber(const KeyType& key) const;

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;
      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;
    
    
    
    void print () const;
    

private:
    struct KeyAndValue {
        KeyAndValue(KeyType key, ValueType value): m_KeyType(key), m_ValueType(value) {}
        KeyType m_KeyType;
        ValueType m_ValueType;
    };

    vector<list<KeyAndValue>*>* m_buckets;
    double m_nAssociations;
    double m_maximumLoadFactor;

    // Helper Functions
    void rehash();
    double currentLoadFactor() const;
    

};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
:   m_maximumLoadFactor(maximumLoadFactor)
{
    // create new buckets
    m_buckets = new vector<list<KeyAndValue>*>;
    
    // reset the buckets
    reset();
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    // loop through buckets and delete any present lists
    for (int i = m_buckets->size() - 1; i >= 0; i--) {
        (*m_buckets)[i]->clear();
        if (!(*m_buckets)[i]->empty())
            delete (*m_buckets)[i];
    }
    
    // delete pointer to vector of buckets
    delete m_buckets;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    // loop through buckets and delete any present lines
    for (int i = m_buckets->size() - 1; i >= 0; i--) {
        (*m_buckets)[i]->clear();
        if (!(*m_buckets)[i]->empty())
            delete (*m_buckets)[i];
    }

    // add an initial amount of buckets
    for (int i = 0; i < INITIAL_NUMBER_OF_BUCKETS; i++)
        m_buckets->push_back(new list<KeyAndValue>);

    // reset number of associations to zero
    m_nAssociations = 0;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    // return number of associations
    return m_nAssociations;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    // determine if the key is already in the map, if so replace the value it is mapped to
    ValueType* findAttempt = find(key);
    if (findAttempt != nullptr) {
        *findAttempt = value;
        return;
    }

    // if key is not already in map, obtain bucket number it will be place in and add it to the bucket's list
    unsigned int i = getBucketNumber(key);
    (*m_buckets)[i]->push_back(KeyAndValue(key, value));
    // increase number of associations
    m_nAssociations++;

    // check if max load factor has been exceeded
    if (currentLoadFactor() > m_maximumLoadFactor)
        rehash();

    return;
}

template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getBucketNumber(const KeyType& key) const {
    // find number of well distribution using hash function
    unsigned int hasher(const KeyType& k);
    unsigned int h = hasher(key);
    // convert the found number into a particular bucket, given the total buckets present
    return h % m_buckets->size();
    return 0;
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    // get the bucket number of given key
    unsigned int i = getBucketNumber(key);

    // loop through the bucket's list
    for (typename list<KeyAndValue>::iterator p = (*m_buckets)[i]->begin(); p != (*m_buckets)[i]->end(); p++) {
        // if key is found, return the value it is mapped to
        if ((*p).m_KeyType == key)
            return &((*p).m_ValueType);
    }

    // if key was not found, return null
    return nullptr;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::rehash() {
    
    // set old buckets to m_buckets
    vector<list<KeyAndValue>*>* oldBuckets = m_buckets;

    // determine the new size of m_buckets
    int newSize = m_buckets->size() * 2;
    
    // set m_buckets to a new vector of buckets
    m_buckets = new vector<list<KeyAndValue>*>;
    // add the necessary amount of new buckets
    for (int i = 0; i < newSize; i++) {
        m_buckets->push_back(new list<KeyAndValue>);
    }
    
    // loop through old buckets
    for (int i = 0; i < oldBuckets->size(); i++) {
        // loop through each bucket's list
        for (typename list<KeyAndValue>::iterator p = (*oldBuckets)[i]->begin(); p != (*oldBuckets)[i]->end(); p++) {
            // obtain key and value at each element of the list
            KeyType k = (*p).m_KeyType;
            ValueType v = (*p).m_ValueType;
            // associate the key and value for m_buckets
            associate(k, v);
        }
    }
    
    // loop through old buckets
    for (int i = 0; i < oldBuckets->size(); i++) {
        // delete every present list
        if ((*oldBuckets)[i] != nullptr)
            delete (*oldBuckets)[i];
    }
    // delete vector of old buckets
    delete oldBuckets;
}

template<typename KeyType, typename ValueType>
double ExpandableHashMap<KeyType, ValueType>::currentLoadFactor() const {
    // calculate and return current load factor
    return m_nAssociations / m_buckets->size();
}


// function that prints out current ExpandableHashMap.h (meant for analysis)
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::print() const {
    
    for (int i = 0; i < m_buckets->size(); i++) {
        cout << "bucket " << i << endl;
        for (typename list<KeyAndValue>::iterator p = (*m_buckets)[i]->begin(); p != (*m_buckets)[i]->end(); p++) {
            
            KeyType key = (*p).m_KeyType;
            ValueType values = (*p).m_ValueType;
            cout << '\t';
            cout << key;
            cout << "  -->  ";
            cout << values;
        }
        cout << endl;
        
    } 
    
}


//#endif
