#include "../hFiles/ResourceScheduler.h"
#include "../hFiles/Util.h"

ResourceScheduler::ResourceScheduler(int tasktype,int caseID) {
	taskType = tasktype; // TODO use ../input
	string filePath = "input/task" + to_string(taskType) + "_case"+to_string(caseID)+".txt";
	// D printf("before freopen, tasktype=%d, caseID=%d\n", tasktype, caseID);
	// D cout << filePath << endl;
	auto _ = freopen(filePath.c_str(), "r", stdin);
	// D cout << "文件打开成功" << endl;
	cin >> numJob >> numHost >> alpha;
	// D cout << numJob << " " << numHost << " " << alpha << endl;

	if (taskType == 2)
		cin >> St;

	hostCore.resize(numHost);
	for (int i = 0; i < numHost; i++)
		cin >> hostCore[i];

	jobBlock.resize(numJob);
	for (int i = 0; i < numJob; i++)
		cin >> jobBlock[i];

	Sc.resize(numJob);
	for (int i = 0; i < numJob; i++)
		cin >> Sc[i];

	dataSize.resize(numJob);
	for (int i = 0; i < numJob; i++) {
		dataSize[i].resize(jobBlock[i]);
		for (int j = 0; j < jobBlock[i]; j++)
			cin >> dataSize[i][j];
	}

	location.resize(numJob);
	for (int i = 0; i < numJob; i++) {
		location[i].resize(jobBlock[i]);
		for (int j = 0; j < jobBlock[i]; j++)
			cin >> location[i][j];
	}
	reset();
	// jobsOrder.resize(numJob, 0);
	// allocatedCore.resize(numJob, set<pair<int, int>>());
	// jobStartTime.resize(numJob, 0); // job完成时间
	// jobFinishTime.resize(numJob, 0); // job完成时间
	// jobCore.resize(numJob); // job分配的core数量

	// runLoc.resize(numJob); // block角度：job number->block number->(hostID, coreID,rank) rank为该主机该核心上第几个运行的block
	// for (int i = 0; i < numJob; i++)
	// 	runLoc[i].resize(jobBlock[i]);

	// hostCoreTask.resize(numHost); // // 核心角度: host->core->task-> <job,block,startTime,endTime>
	// for (int i = 0; i < numHost; i++)
	// 	hostCoreTask[i].resize(hostCore[i]);

	// hostCoreFinishTime.resize(numHost); // host->core->finishTime
	// for (int i = 0; i < numHost; i++)
	// 	hostCoreFinishTime[i].resize(hostCore[i], 0);
}

void ResourceScheduler::reset() {
	jobsOrder.resize(numJob, 0);
	allocatedCore.resize(numJob, set<pair<int, int>>());
	jobStartTime.resize(numJob, 0); // job完成时间
	jobFinishTime.resize(numJob, 0); // job完成时间
	jobCore.resize(numJob); // job分配的core数量

	runLoc.resize(numJob); // block角度：job number->block number->(hostID, coreID,rank) rank为该主机该核心上第几个运行的block
	for (int i = 0; i < numJob; i++)
		runLoc[i].resize(jobBlock[i]);

	hostCoreTask.resize(numHost); // // 核心角度: host->core->task-> <job,block,startTime,endTime>
	for (int i = 0; i < numHost; i++)
		hostCoreTask[i].resize(hostCore[i]);

	hostCoreFinishTime.resize(numHost); // host->core->finishTime
	for (int i = 0; i < numHost; i++)
		hostCoreFinishTime[i].resize(hostCore[i], 0);
}

double ResourceScheduler::getMaxCoreFinishTime() {
	double result = 0;
	for (int hostId = 0; hostId < numHost; hostId++) 
		result = max(result, *max_element(this->hostCoreFinishTime[hostId].begin(), this->hostCoreFinishTime[hostId].end()));
	return result;
}

double ResourceScheduler::evaluate() { 
	// 从jobsOrder和runLoc分析出其他参数
	// D cout << "**************开始评估**************" << endl;
	setRunLocRank(); // 从jobsOrder和runLoc得到runLoc的rank
	// 判断是否合法，得到jobsOrder
	// D cout << "**************开始判断是否合法**************" << endl;
	// set<int> removedJobId;
	// vector<vector<int>> removedPerCore(numHost); // hostId -> coreId -> removed number
	// for (int i = 0; i < numHost; i++)
	// 	removedPerCore[i].resize(hostCore[i], 0);
	// while (removedJobId.size() != numJob) {
	// 	D cout << "**********开始第" << removedJobId.size() + 1 << "次移除**********" << endl;
	// 	int jobIdToRemove = -1;
	// 	for (int jobId = 0; jobId < numJob; jobId++) {
	// 		const bool alreadyRemoved = removedJobId.find(jobId) != removedJobId.end();
	// 		if (alreadyRemoved) continue;
	// 		vector<tuple<int, int, int>> runLocPerJob(runLoc[jobId]);
	// 		sort(runLocPerJob.begin(), runLocPerJob.end());
	// 		D cout << "检查" << jobId << "能否被移除，要检查" << runLocPerJob.size() << "个数据块" << endl;
	// 		bool canRemove = true;
	// 		for (int i = 0; i < runLocPerJob.size(); i++) { 
	// 			// D cout << "检查第" << i << "个数据块" << endl;
	// 			auto [hostIdCur, coreIdCur, rankCur] = runLocPerJob[i]				
	// 			if (rankCur - removedPerCore[hostIdCur][coreIdCur] == 0) {
	// 				D cout << "检查数据块Host" << hostIdCur << " Core" << coreIdCur << " Rank" << rankCur << "-" << removedPerCore[hostIdCur][coreIdCur] << "=" << rankCur - removedPerCore[hostIdCur][coreIdCur] << "，减完为0" << endl;
	// 			}
	// 			else if (i != 0) {
	// 				auto [hostIdPre, coreIdPre, rankPre] = runLocPerJob[i - 1];
	// 				if (hostIdCur == hostIdPre && coreIdCur == coreIdPre && rankCur == rankPre + 1) {
	// 					D cout << "检查数据块Host" << hostIdCur << " Core" << coreIdCur << " Rank" << rankCur << "-" << removedPerCore[hostIdCur][coreIdCur] << "=" << rankCur - removedPerCore[hostIdCur][coreIdCur] << "，" << "HostPre" << hostIdPre << " CorePre" << coreIdPre << " RankPre" << rankPre << endl;
	// 				} else {
	// 					D cout << "检查数据块Host" << hostIdCur << " Core" << coreIdCur << " Rank" << rankCur << "-" << removedPerCore[hostIdCur][coreIdCur] << "=" << rankCur - removedPerCore[hostIdCur][coreIdCur] << "，" << jobId << "不能移除" << endl;
	// 					canRemove = false;
	// 					break;
	// 				}
	// 			} 
	// 			else {
	// 				D cout << "检查数据块Host" << hostIdCur << " Core" << coreIdCur << " Rank" << rankCur << "-" << removedPerCore[hostIdCur][coreIdCur] << "=" << rankCur - removedPerCore[hostIdCur][coreIdCur] << "，" << jobId << "不能移除" << endl;
	// 				canRemove = false;
	// 				break;
	// 			}
	// 		}
	// 		if (canRemove) { 
	// 			jobIdToRemove = jobId;
	// 			break;
	// 		} // else if (removedJobId.size() != numJob) return {};
	// 	}
	// 	if (jobIdToRemove == -1 && removedJobId.size() != numJob) return {}; 
	// 	D cout << "移除" << jobIdToRemove << "，要移除" << jobBlock[jobIdToRemove] << "个数据块" << endl;
	// 	for (int blockIdToRemove = 0; blockIdToRemove < jobBlock[jobIdToRemove]; blockIdToRemove++) {
	// 		auto [hostId, coreId, rank] = runLoc[jobIdToRemove][blockIdToRemove];
	// 		removedPerCore[hostId][coreId]++;
	// 		D cout << "移除Host" << hostId << " Core" << coreId << " Rank" << rank << "，removedPerCore[hostId][coreId]=" << removedPerCore[hostId][coreId] << endl;
	// 	}
	// 	jobsOrder[removedJobId.size()] = jobIdToRemove; // 按照移除的顺序执行任务
	// 	removedJobId.insert(jobIdToRemove);
	// }
	// D cout << "**************评估合法**************" << endl;

	// 计算每个工作每个数据块的开始和结束时间
	vector<vector<double>> nextFreeTime(numHost); // host->core->该主机核心下一次空闲时间
	for (int hostId = 0; hostId < numHost; hostId++)
		nextFreeTime[hostId].resize(hostCore[hostId], 0);

	vector<vector<vector<tuple<int, int, double, double>>>> hostCoreTaskOld(this->hostCoreTask); // 备份hostCoreTask，用于更新startTime和endTime
	this->hostCoreTask.clear();
	hostCoreTask.resize(numHost); // 核心角度: host->core->task-> <job,block,startTime,endTime>
	for (int i = 0; i < numHost; i++)
		hostCoreTask[i].resize(hostCore[i]);

	for (auto jobIdToFind : jobsOrder) {
		// D cout << "评估工作" << jobIdToFind << endl; s
		vector<vector<vector<pair<int, int>>>> hostCoreBlockPerJob(numHost); // 该工作在主机核心上的分布情况，host->core->vector<{job, block}>
		for (int hostId = 0; hostId < numHost; hostId++)
			hostCoreBlockPerJob[hostId].resize(hostCore[hostId], vector<pair<int, int>>());
		allocatedCore[jobIdToFind].clear(); // 更新该工作使用的核心

		// 在hostCoreTask中寻找该工作，更新hostCoreBlockPerJob和allocatedCore
		for (int hostId = 0; hostId < numHost; hostId++) {
			for (int coreId = 0; coreId < hostCore[hostId]; coreId++) {
				for (auto [jobId, blockId, startTime, endTime] : hostCoreTaskOld[hostId][coreId]) {
					if (jobId != jobIdToFind) continue;
					hostCoreBlockPerJob[hostId][coreId].push_back({jobIdToFind, blockId});
					allocatedCore[jobIdToFind].insert({hostId, coreId});	
				}
			}
		}
		// D cout << "寻找工作完毕，hostCoreBlockPerJob和allocatedCore[jobIdToFind]更新完成" << endl;

		this->jobCore[jobIdToFind] = allocatedCore[jobIdToFind].size(); // 更新该任务使用核心数量

		double startTime = 0; // 任务的开始时间，需要等待所有核心空闲后开始
		for (auto [hostId, coreId] : allocatedCore[jobIdToFind]) 
			startTime = max(startTime, nextFreeTime[hostId][coreId]);
		
		// D cout << "开始时间" << startTime << endl;

		double endTime = startTime; // 任务的结束时间
		for (auto [hostId, coreId] : allocatedCore[jobIdToFind]) { // 每个该工作用到的核心，该核心上可能有多个数据块
			double coreTime = startTime;
			for (auto [jobId, blockId] : hostCoreBlockPerJob[hostId][coreId]) { // 每个该工作该核心上的数据块
				double temp = coreTime;
				if (location[jobId][blockId] != hostId) coreTime += dataSize[jobId][blockId] / St; // 传输时间
				coreTime += dataSize[jobId][blockId] / (Sc[jobId] * g(this->jobCore[jobId]));
				// D cout << "插入之前" << this->hostCoreTask.size() << endl;
				this->hostCoreTask[hostId][coreId].push_back({jobId, blockId, temp, coreTime}); 
				// D cout << "插入之后" << endl;
			}
			endTime = max(endTime, coreTime);
		}

		// D cout << "结束时间" << endTime << endl;

		for (auto [hostId, coreId] : allocatedCore[jobIdToFind]) { // 每个用到的核心，更新下次空闲时间
			nextFreeTime[hostId][coreId] = endTime;
			hostCoreFinishTime[hostId][coreId] = endTime;
		}

		this->jobStartTime[jobIdToFind] = startTime;
		this->jobFinishTime[jobIdToFind] = endTime;
	}


	return getMaxCoreFinishTime();
}

void ResourceScheduler::setRunLocRank() {
	// 从jobsOrder和runLoc得到runLoc的rank
	this->hostCoreTask.clear();
	hostCoreTask.resize(numHost); // // 核心角度: host->core->task-> <job,block,startTime,endTime>
	for (int i = 0; i < numHost; i++)
		hostCoreTask[i].resize(hostCore[i]);

	for (auto jobId: jobsOrder) { 
		for (int blockId = 0; blockId < jobBlock[jobId]; blockId++) {
			const auto [hostId, coreId, rank] = runLoc[jobId][blockId];
			// D cout << "hostCoreTask长度" << hostCoreTask.size();
			// D cout << "hostCoreTask[hostId]长度" << hostCoreTask[hostId].size();
			// D cout << "hostCoreTask[" << hostId << "][" << coreId << "]长度" << this->hostCoreTask[hostId][coreId].size() << endl;
			this->hostCoreTask[hostId][coreId].push_back({jobId, blockId, -8888, -1111});	
		}
	}
	for (int hostId = 0; hostId < numHost; hostId++) {
		for (int coreId = 0; coreId < hostCore[hostId]; coreId++) {
			int rank = 0;
			for (auto [jobId, blockId, startTime, endTime] : hostCoreTask[hostId][coreId]) {
				this->runLoc[jobId][blockId] = {hostId, coreId, rank++};
				// D cout << "Job" << jobId << "Block" << blockId << "是Host" << hostId << "Core" << coreId << "上的第" << rank << "个任务" << endl;
			}
		}
	}
}

void ResourceScheduler::randomSchedule() {
	// 从jobsOrder和runLoc可以推出其他属性
	// 生成任务随机序列
	for (int i = 0; i < numJob; i++) 
		this->jobsOrder[i] = i;
	random_shuffle(this->jobsOrder.begin(), this->jobsOrder.end());

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distributionHost(0, numHost - 1); 

	for (auto jobId: jobsOrder) { 
		// 随机选择jobBlock[jobId]次{主机, 核心}，注意可能把多个数据块分配到同一个核心
		for (int blockId = 0; blockId < jobBlock[jobId]; blockId++) {
			int hostId = distributionHost(generator);
			std::uniform_int_distribution<int> distributionHostCore(0, hostCore[hostId] - 1);
			int coreId = distributionHostCore(generator);;
			this->runLoc[jobId][blockId] = {hostId, coreId, -7777};
		}
	}
}

void ResourceScheduler::simulatedAnnealing(double T = 100000, double theta = 0.97, double lowT = 0.00001) { // TODO 为什么SA后没有收敛还可以再爬山
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
	std::uniform_real_distribution<double> distribution(0.0, 1.0); 

	double ans = 0x3f3f3f3f;

	int turns = 0;
	while (T > lowT)
	{
		turns++;
		auto exchangeArgs = randomExchange();		  //产生新解
		double newAns = evaluate(); //评估新解
		
		if (newAns < bestAns) {
			bestAns = newAns;
			bestJobsOrder = vector<int>(jobsOrder);
			bestRunLoc = vector<vector<tuple<int, int, int>>>(runLoc);
		}

		double delta = newAns - ans;
		// D cout << "Prob of turn " << turns << ": " << setprecision(5) << exp(-delta / T) << " T=" << T << endl;
		if (exp(-delta / T) > distribution(generator)) ans = newAns;
		else rollBackExchange(exchangeArgs);	
		T = T * theta;			  //当前温度
	}

	// D cout << "Answer of SA after " << turns << " turns: " << ans << endl;

	jobsOrder = vector<int>(bestJobsOrder);
	runLoc = vector<vector<tuple<int, int, int>>>(bestRunLoc);
	ans = evaluate();
	// D cout << "Best Answer of SA after " << turns << " turns: " << ans << endl;

}

void ResourceScheduler::hillClimbing(int steps = 10000) {
	double ans = evaluate();
	// // cout << "Evaluate: " << evaluate() << endl;
	// D cout << "Best Answer of HC: " << ans << endl;
	for (int i = 0; i < steps; i++) {
		// D cout << i << endl;
		auto exchangeArgs = randomExchange();
		double newAns = evaluate();
		// D cout << "NewAns: " << newAns << endl;

		if (newAns > ans) {
			rollBackExchange(exchangeArgs);
		} else {
			ans = newAns;
			if (newAns < bestAns) {
				bestAns = newAns;
				bestJobsOrder = vector<int>(jobsOrder);
				bestRunLoc = vector<vector<tuple<int, int, int>>>(runLoc);
			}
		}
	}
	// D cout << "Best Answer of HC: " << ans << endl;
	// jobsOrder = vector<int>(bestJobsOrder);
	// runLoc = vector<vector<tuple<int, int, int>>>(bestRunLoc);
	// evaluate();
}

tuple<int, int, int, int> ResourceScheduler::randomExchange() {
	/*交换方法：
	  1. 单个任务内部，交换不同数据块处理的核心
	  2. 单个任务，使用不同的核心
	  3. 交换任务顺序
	 */
	// 先只考虑使用不同的核心
	// 从jobsOrder和runLoc可以推出其他属性
	// time_t t;
	// srand(time(0));
	// 返回交换信息 {jobId1, jobId2}
	//             {jobId, blockId, newHostId, newCoreId}

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distributionExchangeType(0, 100); 
	int exchangeType = distributionExchangeType(generator);

	if (exchangeType < 30) {
		// 交换jobsOrder
		std::uniform_int_distribution<int> distributionNumJob(0, numJob - 1); 
		int jobId1 = distributionNumJob(generator);
		int jobId2 = distributionNumJob(generator);
		// D cout << "交换了" << jobsOrder[jobId1] << "和" << jobsOrder[jobId2] << "的工作顺序";
		swap(jobsOrder[jobId1], jobsOrder[jobId2]);
		return {jobId1, jobId2, -1, -1};
	} else {
		// 更改runLoc
		std::uniform_int_distribution<int> distributionNumJob(0, numJob - 1); 
		int jobId = distributionNumJob(generator);

		std::uniform_int_distribution<int> distributionJobBlock(0, jobBlock[jobId] - 1); 
		int blockId = distributionJobBlock(generator);
		
		auto [hostId, coreId, rank] = runLoc[jobId][blockId];

		std::uniform_int_distribution<int> distributionHost(0, numHost - 1); 
		int newHostId = distributionHost(generator);
		std::uniform_int_distribution<int> distributionHostCore(0, hostCore[newHostId] - 1); 
		int newCoreId = distributionHostCore(generator);

		runLoc[jobId][blockId] = {newHostId, newCoreId, rank};
		// D cout << "将Job" << jobId << "Block" << blockId << "从Host" << hostId << "Core" << coreId << "重新分配到Host" << newHostId << "Core" << newCoreId << endl;
		return {jobId, blockId, hostId, coreId};
	}
}

void ResourceScheduler::rollBackExchange(tuple<int, int, int, int> exchangeArgs) {
	auto [arg0, arg1, arg2, arg3] = exchangeArgs;
	if (arg2 == -1) { // 交换jobsOrder
		swap(arg0, arg1);
	} else { // 更改runLoc
		runLoc[arg0][arg1] = {arg2, arg3, -7777};
	}
}

void ResourceScheduler::outputSolutionFromBlock() {
	cout << "\nTask" << taskType << " Solution (Block Perspective) of Teaching Assistant:\n\n";
	for (int i = 0; i < numJob; i++) {
		double speed = g(jobCore[i]) * Sc[i];
		cout << "Job" << i << " obtains " << jobCore[i] << " cores (speed=" << speed << ") and finishes at time " << jobFinishTime[i] << ": \n";
		for (int j = 0; j < jobBlock[i]; j++) {
			cout << "\tBlock" << j << ": H" << get<0>(runLoc[i][j]) << ", C" << get<1>(runLoc[i][j]) << ", R" << get<2>(runLoc[i][j]) << " (time=" << fixed << setprecision(2) << dataSize[i][j] / speed << ")" << " \n";
		}
		cout << "\n";
	}

	cout << "The maximum finish time: " << *max_element(jobFinishTime.begin(), jobFinishTime.end()) << "\n";
	cout << "The total response time: " << accumulate(jobFinishTime.begin(), jobFinishTime.end(), 0.0) << "\n\n";
}

void ResourceScheduler::outputSolutionFromCore() {
	cout << "\nTask" << taskType << " Solution (Core Perspective) of Teaching Assistant:\n\n";
	double maxHostTime = 0, totalRunningTime = 0.0;
	for (int i = 0; i < numHost; i++) {
		double hostTime = *max_element(hostCoreFinishTime[i].begin(), hostCoreFinishTime[i].end());
		maxHostTime = max(hostTime, maxHostTime);
		totalRunningTime += accumulate(hostCoreFinishTime[i].begin(), hostCoreFinishTime[i].end(), 0.0);
		cout << "Host" << i << " finishes at time " << hostTime << ":\n\n";
		for (int j = 0; j < hostCore[i]; j++) {
			cout << "\tCore" << j << " has " << hostCoreTask[i][j].size() << " tasks and finishes at time " << hostCoreFinishTime[i][j] << ":\n";
			for (int k = 0; k < hostCoreTask[i][j].size(); k++) {
				cout << "\t\tJ" << setw(2) << setfill('0') << get<0>(hostCoreTask[i][j][k]) << ", B" << setw(2) << setfill('0') << get<1>(hostCoreTask[i][j][k]) << ", runTime " << fixed << setprecision(1) << setw(5) << setfill('0') << get<2>(hostCoreTask[i][j][k]) << " to " << fixed << setprecision(1) << setw(5) << setfill('0') << get<3>(hostCoreTask[i][j][k]) << "\n";
			}
			cout << "\n";
		}
		cout << "\n\n";
	}
	cout << "The maximum finish time of hosts: " << maxHostTime << "\n";
	cout << "The total efficacious running time: " << totalRunningTime << "\n";
	cout << "Utilization rate: " << totalRunningTime / accumulate(hostCore.begin(), hostCore.end(), 0.0) / maxHostTime << "\n\n";
}


void ResourceScheduler::outputSolutionFromBlockVerbose() {
	cout << "\nTask" << taskType << " Solution (Block Perspective) of Teaching Assistant:\n\n";
	cout << "Job order: [";
	for (auto jobId : jobsOrder)
		cout << jobId << ", ";
	cout << "]\n\n";
	for (int jobId = 0; jobId < numJob; jobId++) {
		double speed = Sc[jobId] * g(this->jobCore[jobId]);
		cout << "Job" << jobId << " obtains " << jobCore[jobId] << " cores [";
		for (auto [hostId, coreId] : allocatedCore[jobId])
			cout << "H" << hostId << "C" << coreId << ", ";
		cout <<  "] (speed=" << speed << ") and starts at time " << jobStartTime[jobId] << " and finishes at time " << jobFinishTime[jobId] << ": \n";
		
		for (int blockId = 0; blockId < jobBlock[jobId]; blockId++) {
			auto [ho, co, ra] = runLoc[jobId][blockId];
			auto [jo, bl, st, ed] = hostCoreTask[ho][co][ra];
			auto sz = dataSize[jobId][blockId];
			auto ti = sz / speed; // 不包括传输时间
			// cout << st << ed;
			cout << "\tBlock" << blockId << "(Size=" << sz << ") : H" << ho << ", C" << co << ", R" << ra << " (time=" << fixed << setprecision(2) << ti << ")" << " \n";
		}
		cout << "\n";
	}
	cout << "The maximum finish time: " << *max_element(jobFinishTime.begin(), jobFinishTime.end()) << "\n";
	cout << "The total response time: " << accumulate(jobFinishTime.begin(), jobFinishTime.end(), 0.0) << "\n\n";
}

void ResourceScheduler::outputSolutionFromCoreVerbose() {
	cout << "\nTask" << taskType << " Solution (Core Perspective) of Teaching Assistant:\n\n";
	double maxHostTime = 0, totalRunningTime = 0.0;
	for (int i = 0; i < numHost; i++) {
		double hostTime = *max_element(hostCoreFinishTime[i].begin(), hostCoreFinishTime[i].end());
		maxHostTime = max(hostTime, maxHostTime);
		totalRunningTime += accumulate(hostCoreFinishTime[i].begin(), hostCoreFinishTime[i].end(), 0.0);
		cout << "Host" << i << " finishes at time " << hostTime << ":\n\n";
		for (int j = 0; j < hostCore[i]; j++) {
			cout << "\tCore" << j << "(speed=" << Sc[i] << ") has " << hostCoreTask[i][j].size() << " tasks and finishes at time " << hostCoreFinishTime[i][j] << ":\n";
			for (int k = 0; k < hostCoreTask[i][j].size(); k++) {
				cout << "\t\tJ" << setw(2) << setfill('0') << get<0>(hostCoreTask[i][j][k]) << ", B" << setw(2) << setfill('0') << get<1>(hostCoreTask[i][j][k]) << ", Size=" << dataSize[get<0>(hostCoreTask[i][j][k])][get<1>(hostCoreTask[i][j][k])] << ", runTime " << fixed << setprecision(1) << setw(5) << setfill('0') << get<2>(hostCoreTask[i][j][k]) << " to " << fixed << setprecision(1) << setw(5) << setfill('0') << get<3>(hostCoreTask[i][j][k]) << "\n";
			}
			cout << "\n";
		}
		cout << "\n\n";
	}
	cout << "The maximum finish time of hosts: " << setprecision(4) << maxHostTime << "\n";
	cout << "The total efficacious running time: " << totalRunningTime << "\n";
	cout << "Utilization rate: " << totalRunningTime / accumulate(hostCore.begin(), hostCore.end(), 0.0) / maxHostTime << "\n\n";
}

void ResourceScheduler::visualization() {

}

double ResourceScheduler::g(int e) {
	return 1 - alpha * (e - 1);
}

void ResourceScheduler::setBest() {
	jobsOrder = vector<int>(bestJobsOrder);
	runLoc = vector<vector<tuple<int, int, int>>>(bestRunLoc);
	evaluate();
}