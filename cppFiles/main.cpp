#include<iostream>
#include "../hFiles/ResourceScheduler.h"
#include "../hFiles/Util.h"

int main() {
	int taskType=1;
	int caseID=1;
	ResourceScheduler rs(taskType,caseID);
	// generator(rs,taskType);
	for (int i = 0; i < 10; i++) {
		rs.reset();
		rs.randomSchedule();
		rs.simulatedAnnealing(100000, 0.999, 0.00001);
		rs.hillClimbing(10000);
	}

	rs.setBest();
	rs.outputSolutionFromBlock();
	rs.outputSolutionFromCore();
	// rs.outputSolutionFromBlockVerbose();
	// rs.outputSolutionFromCoreVerbose();
	return 0;
}

