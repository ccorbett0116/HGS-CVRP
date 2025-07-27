//
// Created by chkwon on 3/22/22.
//

#ifndef INSTANCECVRPLIB_H
#define INSTANCECVRPLIB_H
#include<string>
#include<vector>

class InstanceCVRPLIB
{
public:
	std::vector< std::vector<double> > dist_mtx;
	std::vector<std::vector<double>> time_mtx;
	std::vector<double> service_time;
	std::vector<double> demands;
	std::vector<double> pickups;
	std::vector<double> ready_time;
	std::vector<double> due_time;
	double durationLimit = 1.e30;							// Route duration limit
	double vehicleCapacity;									// Capacity limit
	bool isDurationConstraint = false;						// Indicates if the problem includes duration constraints
	int nbClients ;											// Number of clients (excluding the depot)
	std::string problemName;
	std::string problemType;
	int numVehicles = 0;
	double dispatchingCost = 0.0;
	double unitCost = 0.0;

	InstanceCVRPLIB(std::string pathToInstance, bool isRoundingInteger);
};


#endif //INSTANCECVRPLIB_H
