#include "Individual.h"

void Individual::evaluateCompleteCost(const Params & params) {
    eval = EvalIndiv();
    for (int r = 0; r < params.nbVehicles; r++) {
        if (!chromR[r].empty()) {
            double currentTime = 0.0;
            double routeDistance = 0.0;
            double routeLoad = 0.0;

            for (int i = 0; i < chromR[r].size(); i++) {
                int node = chromR[r][i];

                // Travel from previous node (depot for first node)
                if (i == 0) {
                    routeDistance += params.timeCost[0][node];  // Use timeCost (distance matrix)
                    currentTime += params.timeMatrix[0][node];   // Use timeMatrix (travel time)
                } else {
                    int prevNode = chromR[r][i-1];
                    routeDistance += params.timeCost[prevNode][node];
                    currentTime += params.timeMatrix[prevNode][node];
                }

                // Check time windows
                if (currentTime < params.readyTime[node]) {
                    currentTime = params.readyTime[node]; // Wait if early
                } else if (currentTime > params.dueTime[node]) {
                    eval.timeWarp += (currentTime - params.dueTime[node]);
                    currentTime = params.dueTime[node]; // Warp if late
                }

                // Add service time and demand
                currentTime += params.serviceTime[node];
                routeLoad += params.cli[node].demand;  // Access demand via cli[]
            }

            // Return to depot

        	if (currentTime > params.dueTime[0]) {  // Check depot closing time
        		eval.timeWarp += (currentTime - params.dueTime[0]);
        	}

        	routeDistance += params.timeCost[chromR[r].back()][0];
        	currentTime += params.timeMatrix[chromR[r].back()][0];

        	// Check depot time window (NEW)
        	if (currentTime > params.dueTime[0]) {  // Index 0 is depot
        		eval.timeWarp += (currentTime - params.dueTime[0]);
        	}

            // Update evaluations
            eval.distance += routeDistance;
            eval.nbRoutes++;
            if (routeLoad > params.vehicleCapacity) eval.capacityExcess += (routeLoad - params.vehicleCapacity);
            if (currentTime > params.durationLimit) eval.durationExcess += (currentTime - params.durationLimit);
        }
    }

    // Final cost calculation
    eval.penalizedCost = eval.distance
                       + eval.capacityExcess * params.penaltyCapacity
                       + eval.durationExcess * params.penaltyDuration
                       + eval.timeWarp * params.penaltyMultiplier;
    eval.isFeasible = (eval.capacityExcess < MY_EPSILON && eval.durationExcess < MY_EPSILON);
}

Individual::Individual(Params & params)
{
	successors = std::vector <int>(params.nbClients + 1);
	predecessors = std::vector <int>(params.nbClients + 1);
	chromR = std::vector < std::vector <int> >(params.nbVehicles);
	chromT = std::vector <int>(params.nbClients);
	for (int i = 0; i < params.nbClients; i++) chromT[i] = i + 1;
	std::shuffle(chromT.begin(), chromT.end(), params.ran);
	eval.penalizedCost = 1.e30;
}

Individual::Individual(Params & params, std::string fileName) : Individual(params)
{
	double readCost;
	chromT.clear();
	std::ifstream inputFile(fileName);
	if (inputFile.is_open())
	{
		std::string inputString;
		inputFile >> inputString;
		// Loops in the input file as long as the first line keyword is "Route"
		for (int r = 0; inputString == "Route"; r++)
		{
			inputFile >> inputString;
			getline(inputFile, inputString);
			std::stringstream ss(inputString);
			int inputCustomer;
			while (ss >> inputCustomer) // Loops as long as there is an integer to read in this route
			{
				chromT.push_back(inputCustomer);
				chromR[r].push_back(inputCustomer);
			}
			inputFile >> inputString;
		}
		if (inputString == "Cost") inputFile >> readCost;
		else throw std::string("Unexpected token in input solution");

		// Some safety checks and printouts
		evaluateCompleteCost(params);
		if ((int)chromT.size() != params.nbClients) throw std::string("Input solution does not contain the correct number of clients");
		if (!eval.isFeasible) throw std::string("Input solution is infeasible");
		if (eval.penalizedCost != readCost)throw std::string("Input solution has a different cost than announced in the file");
		if (params.verbose) std::cout << "----- INPUT SOLUTION HAS BEEN SUCCESSFULLY READ WITH COST " << eval.penalizedCost << std::endl;
	}
	else
		throw std::string("Impossible to open solution file provided in input in : " + fileName);
}
