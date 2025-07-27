
// Created by chkwon on 3/22/22.
// Rewritten to properly match VRPSPDTW dataset format.

#include <fstream>
#include <cmath>
#include "InstanceCVRPLIB.h"

#include <iostream>
#include <sstream>  // for std::getline and std::stringstream

InstanceCVRPLIB::InstanceCVRPLIB(std::string pathToInstance, bool isRoundingInteger)
{
	std::ifstream inputFile(pathToInstance);
	if (!inputFile.is_open())
		throw std::string("Cannot open instance file: " + pathToInstance);

	std::string line;
	while (std::getline(inputFile, line)) {
		if (line.empty()) continue;

		if (line.rfind("NAME", 0) == 0) {
			problemName = line.substr(line.find(":") + 1);
			problemName.erase(0, problemName.find_first_not_of(" \t"));
		}
		else if (line.rfind("TYPE", 0) == 0) {
			problemType = line.substr(line.find(":") + 1);
			problemType.erase(0, problemType.find_first_not_of(" \t"));
		}
		else if (line.rfind("DIMENSION", 0) == 0) {
			nbClients = std::stoi(line.substr(line.find(":") + 1)) - 1;
		}
		else if (line.rfind("VEHICLES", 0) == 0) {
			numVehicles = std::stoi(line.substr(line.find(":") + 1));
		}
		else if (line.rfind("DISPATCHINGCOST", 0) == 0) {
			dispatchingCost = std::stod(line.substr(line.find(":") + 1));
		}
		else if (line.rfind("UNITCOST", 0) == 0) {
			unitCost = std::stod(line.substr(line.find(":") + 1));
		}
		else if (line.rfind("CAPACITY", 0) == 0) {
			vehicleCapacity = std::stod(line.substr(line.find(":") + 1));
		}
		else if (line.rfind("EDGE_WEIGHT_TYPE", 0) == 0) {
			std::string type = line.substr(line.find(":") + 1);
			type.erase(0, type.find_first_not_of(" \t"));
			if (type != "EXPLICIT") {
				throw std::runtime_error("Expected EDGE_WEIGHT_TYPE: EXPLICIT");
			}
		}
		else if (line == "NODE_SECTION") {
			break;
		}
	}

	// Allocate storage (x/y omitted — not part of format)
	demands.resize(nbClients + 1);
	service_time.resize(nbClients + 1);
	pickups.resize(nbClients + 1);
	ready_time.resize(nbClients + 1);
	due_time.resize(nbClients + 1);

	// Parse NODE_SECTION lines: i,delivery,pickup,start,end,service
	for (int i = 0; i <= nbClients; ++i) {
		std::getline(inputFile, line);
		std::stringstream ss(line);
		std::string field;
		int id;
		double delivery, pickup, ready, due, service;

		std::getline(ss, field, ','); id = std::stoi(field);
		std::getline(ss, field, ','); delivery = std::stod(field);
		std::getline(ss, field, ','); pickup   = std::stod(field);
		std::getline(ss, field, ','); ready    = std::stod(field);
		std::getline(ss, field, ','); due      = std::stod(field);
		std::getline(ss, field, ','); service  = std::stod(field);

		if (id != i) throw std::string("Node index mismatch at line: " + line);

		demands[i] = delivery;
		pickups[i] = pickup;
		ready_time[i] = ready;
		due_time[i] = due;
		service_time[i] = service;
	}

	// Allocate distance/time matrices
	dist_mtx.resize(nbClients + 1, std::vector<double>(nbClients + 1, 0.0));
	time_mtx.resize(nbClients + 1, std::vector<double>(nbClients + 1, 0.0));

	// Look for DISTANCETIME_SECTION
	while (std::getline(inputFile, line)) {
		if (line == "DISTANCETIME_SECTION")
			break;
	}

	while (std::getline(inputFile, line)) {
		if (line.empty()) continue;

		std::stringstream ss(line);
		std::string fromStr, toStr, distStr, timeStr;

		if (!std::getline(ss, fromStr, ',')) continue;
		if (!std::getline(ss, toStr, ',')) continue;
		if (!std::getline(ss, distStr, ',')) continue;
		if (!std::getline(ss, timeStr, ',')) continue;

		try {
			int from = std::stoi(fromStr);
			int to = std::stoi(toStr);
			double dist = std::stod(distStr);
			double time = std::stod(timeStr);
			dist_mtx[from][to] = dist;
			time_mtx[from][to] = time;
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Invalid line in DISTANCETIME_SECTION: " << line << "\n";
			continue;
		}
	}
}
