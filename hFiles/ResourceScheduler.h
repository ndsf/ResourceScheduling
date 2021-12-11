#pragma once
#include<vector>
#include<algorithm>
#include<iostream>
#include<iomanip>
#include<numeric>
#include<ctime>
#include<set>
#include <sstream>
#include <chrono>
#include <random>
using namespace std;

class ResourceScheduler {
public:
	int taskType; // 1 or 2
	int caseID;   // 1
	int numJob; // No. 0 ~ numJob-1
	int numHost;// No. 0 ~ numHost-1
	double St;  // Speed of Transimision 
	double alpha;   // g(e)=1-alpha(e-1) alpha>0, e is the number of cores allocated to a single job
	vector<int> hostCore;              // The number of cores for each host
	vector<int> jobBlock;              // The number of blocks for each job
	vector<double> Sc;                    // Speed of calculation for each job
	vector<vector<double>> dataSize;      // Job-> block number-> block size
	vector<vector<int>> location;         // Job-> block number-> block location (host number)

	// Job execution order by start time
	vector<int> jobsOrder;

	// 1. jobs顺序
	// 2. job用的core的顺序

	// Core allocated by job 改成vector<pair<int,int>> 按照顺序
	vector<set<pair<int, int>>> allocatedCore;
	
	// The start time of each job 
	vector<double> jobStartTime;     

	// The finish time of each job 
	vector<double> jobFinishTime;                 

	// The number of cores allocated to each job.
	vector<int> jobCore;                

	// Block perspective: job number->block number->(hostID, coreID,rank), rank=1 means that block is the first task running on that core of that host       
	vector<vector<tuple<int, int, int>>> runLoc; 

	// Core perspective: host->core->task-> <job,block,startTime,endTime>
	vector<vector<vector<tuple<int, int, double, double>>>> hostCoreTask; 

	// host->core->finishTime
	vector<vector<double>> hostCoreFinishTime; 

	double bestAns = 0x3f3f3f3f;
	vector<int> bestJobsOrder;
	vector<vector<tuple<int, int, int>>> bestRunLoc; 

	ResourceScheduler(int,int);
	void reset();
	void setBest();
	void setRunLocRank();
	double getMaxCoreFinishTime();
	double evaluate();
	void hillClimbing(int);
	void simulatedAnnealing(double, double, double);
	// void old_schedule();
	void randomSchedule();
	tuple<int, int, int, int> randomExchange();
	void rollBackExchange(tuple<int, int, int, int>);
	void outputSolutionFromBlock();
	void outputSolutionFromBlockVerbose();
	void outputSolutionFromCore();
	void outputSolutionFromCoreVerbose();
	void visualization(); // An optional fuction.
	double g(int);
};
