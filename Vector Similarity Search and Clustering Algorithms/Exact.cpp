#include <iostream>
#include <algorithm>
#include <limits>
#include "Exact.h"
#include "Image.h"
#include "Utils.h"
#include "Technique.h"

using namespace std;

Exact::Exact(int arg_k, int arg_N, double arg_r, vector<Image*>& arg_data, vector<Image*>& arg_queries): Technique(arg_k,arg_N,arg_r,arg_data,arg_queries){
}

Exact::~Exact(){
    //Deallocate all the distancePairs that are allocated in the heap 
    for(int query_num=0; query_num<allDistances.size(); query_num++){
        for(int datum_num=0; datum_num<allDistances[query_num].size(); datum_num++){
            delete allDistances[query_num][datum_num];
        }
    }
}

void Exact::calculate_allDistances(Distance_Function distance_Function){
    allDistances.reserve(Queries.size());
    Times.resize(Queries.size());
    // For each Query
    for(int query_num=0; query_num<Queries.size(); query_num++){
        //cout << "Query: " << query_num << endl;
        auto start = chrono::system_clock::now();
        vector<distancePair*> distancesFromQuery; distancesFromQuery.reserve(Data.size()); // reserve the capacity in order to avoid extensive reallocations
        // For each Datum
        for(int datum_num=0; datum_num<Data.size(); datum_num++){
            distancePair* newDistancePair = new distancePair(distance_Function(Queries[query_num],Data[datum_num]),Data[datum_num]);
            distancesFromQuery.push_back(newDistancePair);
        }
        allDistances.push_back(distancesFromQuery);
        auto end = chrono::system_clock::now();
        auto duration = end-start;
        Times[query_num] += duration;
    }
}

vector<distancePair*> Exact::calculate_Nearest(Distance_Function distance_Function){
    if (allDistances.empty()) calculate_allDistances(distance_Function); //if we haven't already calculated all distances, do it
    
    vector<distancePair*> singleNearestNeighbors; //Holds the single nearest neighbor of each query
    for(int query_num=0; query_num<Queries.size(); query_num++){
        auto start = chrono::system_clock::now();
        distancePair* nearestNeighbor_of_Query = allDistances[query_num][0]; //initialize the nearest neighbor for each query to be the first datum
        for(int datum_num=0; datum_num<Data.size(); datum_num++){
            if (allDistances[query_num][datum_num]->first < nearestNeighbor_of_Query->first){
                nearestNeighbor_of_Query = allDistances[query_num][datum_num];
            }
        }
        singleNearestNeighbors.push_back(nearestNeighbor_of_Query);
        
        auto end = chrono::system_clock::now();
        auto duration = end-start;
        Times[query_num] += duration;
    }
    return singleNearestNeighbors;
}

vector<vector<distancePair*>> Exact::calculate_N_Nearest(Distance_Function distance_Function){
    if (allDistances.empty()) calculate_allDistances(distance_Function); //if we haven't already calculated all distances, do it

    vector<vector<distancePair*>> initialDistances = allDistances;
    vector<vector<distancePair*>> NNearestNeighbors;
    distancePair* dummyDistancePair = new distancePair(numeric_limits<double>::max(),NULL);
    for(int neighbor_num=0; neighbor_num<N; neighbor_num++){
        vector<distancePair*> singleNearestNeighbors = calculate_Nearest(distance_Function);
        NNearestNeighbors.push_back(singleNearestNeighbors);
        for(int query_num=0; query_num<Queries.size(); query_num++){
            allDistances[query_num][singleNearestNeighbors[query_num]->second->get_order()] = dummyDistancePair;
        }
    }
    allDistances = initialDistances;
    delete dummyDistancePair;
    return NNearestNeighbors;
}

vector<vector<distancePair*>> Exact::calculate_in_Range(Distance_Function distance_Function){
    if (allDistances.empty()) calculate_allDistances(distance_Function); //if we haven't already calculated all distances, do it

    vector<vector<distancePair*>> radiusNeighbors;
    for(int query_num=0; query_num<Queries.size(); query_num++){
        vector<distancePair*> radiusNeighbors_of_Query;
        for(int datum_num=0; datum_num<Data.size(); datum_num++){
            if (allDistances[query_num][datum_num]->first < r){
                radiusNeighbors_of_Query.push_back(allDistances[query_num][datum_num]);
            }
        }
        radiusNeighbors.push_back(radiusNeighbors_of_Query);
    }
    return radiusNeighbors;
}

void Exact::modify_radii(){
    r = r*2;
}

void Exact::new_Queries(vector<Image*>& newQueries){
    Queries = newQueries;
}

/* <------------ To find an appropriate value for w ------------> */

/*void Exact::calculate_allDataDistances(Distance_Function distance_Function){
    // For each Query
    for(int datum_num1=0; datum_num1<10000; datum_num1++){
        vector<distancePair*> distancesFromQuery;
        distancesFromQuery.reserve(10000); // reserve the capacity in order to avoid extensive reallocations
        // For each Datum
        for(int datum_num2=0; datum_num2<10000; datum_num2++){
            if (datum_num1 != datum_num2){
                distancePair* newDistancePair = new distancePair(distance_Function(Data[datum_num1],Data[datum_num2]),Data[datum_num2]);
                distancesFromQuery.push_back(newDistancePair);
            }
            else {
                distancePair* newDistancePair = new distancePair(numeric_limits<double>::max(),Data[datum_num2]);
                distancesFromQuery.push_back(newDistancePair);
            }
        }
        allDataDistances.push_back(distancesFromQuery);
    }
}

vector<distancePair*> Exact::calculate_Data_Nearest(Distance_Function distance_Function){
    if (allDistances.empty()) calculate_allDataDistances(distance_Function); //if we haven't already calculated all distances, do it
    
    vector<distancePair*> singleNearestNeighbors; //Holds the single nearest neighbor of each query
    for(int datum_num1=0; datum_num1<10000; datum_num1++){
        distancePair* nearestNeighbor_of_Query = allDataDistances[datum_num1][0]; //initialize the nearest neighbor for each query to be the first datum
        for(int datum_num2=0; datum_num2<10000; datum_num2++){
            if (allDataDistances[datum_num1][datum_num2]->first < nearestNeighbor_of_Query->first){
                nearestNeighbor_of_Query = allDataDistances[datum_num1][datum_num2];
            }
        }
        singleNearestNeighbors.push_back(nearestNeighbor_of_Query);
    }
    return singleNearestNeighbors;
}*/