#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdint>
#include <functional>
#include <vector>
#include <queue>
#include <stack>
#include <utility>
using namespace std;

const size_t N = 50;
const uint32_t INF = uint32_t(1e+9);

typedef pair < size_t, size_t > Edge;
Edge NullEdge(N, N);

enum class EdgeType
{
	Outgoing,
	Incoming
};

struct PartialSolution
{
	bool operator>(const PartialSolution& other) const
	{
		return LowerBoundTimesTwo > other.LowerBoundTimesTwo;
	}

	PartialSolution WithEdge(Edge pivot, uint32_t D[N][N])
	{
		auto i = pivot.first, j = pivot.second;
		
		PartialSolution child = *this;
		child.Cost += D[i][j];
		for (size_t k = 0; k < n; k++)
		{
			child.Constraints[i][k] = child.Constraints[k][j] = -1;
			child.Reduced[i][k] = child.Reduced[k][j] = INF;
		}
		child.EnabledEdges++;
		child.Constraints[i][j] = 1;
		child.Constraints[j][i] = -1;

		auto subpathTo = child.TraverseSubPath(i, EdgeType::Outgoing);
		auto subpathFrom = child.TraverseSubPath(i, EdgeType::Incoming);
		if (subpathTo.size() + subpathFrom.size() - 1 != n)
		{
			child.Constraints[subpathTo.back()][subpathFrom.back()] = -1;
			child.Reduced[subpathTo.back()][subpathFrom.back()] = INF;
		}

		child.Reduce();
		return child;
	}

	PartialSolution WithoutEdge(Edge pivot, uint32_t D[N][N])
	{
		auto i = pivot.first, j = pivot.second;
		
		PartialSolution child = *this;
		child.DisabledEdges++;
		child.Constraints[i][j] = -1;
		child.Reduced[i][j] = INF;
		child.Reduce(EdgeType::Outgoing, i);
		child.Reduce(EdgeType::Incoming, j);

		return child;
	}

	Edge ChoosePivotEdge()
	{
		auto minStride = [&](size_t except, size_t k, size_t kStride) {uint32_t m = INF; for (size_t i = 0; i < n; i++) if(i != except) m = min(m, *(&Reduced[0][0] + IK(i, k, kStride))); return m;  };
		auto rowMin = [&](size_t k, size_t except) {return minStride(except, k, N); };
		auto columnMin = [&](size_t k, size_t except) {return minStride(except, k, 1); };
		
		uint32_t bestIncrease = 0;
		Edge bestPivot = NullEdge;
		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = 0; j < n; j++)
			{
				if (Constraints[i][j] == 0 && Reduced[i][j] == 0)
				{
					auto increase = rowMin(i, j) + columnMin(j, i);
					if (increase > bestIncrease)
					{
						bestIncrease = increase;
						bestPivot = Edge(i, j);
					}
				}
			}
		}

		return bestPivot;
	}

	vector<size_t> TraverseSubPath(size_t cur, EdgeType edgeType)
	{
		auto stride = edgeType == EdgeType::Outgoing ? 1 : N;
		vector<size_t> subpath{ cur };
		for (size_t k = 0; k < n; k++)
		{
			size_t next = N;
			for (size_t i = 0; i < n; i++)
			{
				if (*(&Constraints[0][0] + IK(cur, i, stride)) == 1)
				{
					next = i;
					break;
				}
			}

			if (next == N)
				break;

			subpath.push_back(next);
			cur = next;
		}
		return subpath;
	}

	void Reduce(EdgeType edgeType, size_t i)
	{
		auto kStride = edgeType == EdgeType::Outgoing ? 1 : N;
		
		uint32_t m = INF;
		for (size_t k = 0; k < n; k++)
			if (*(&Constraints[0][0] + IK(i, k, kStride)) != -1)
				m = min(m, *(&Reduced[0][0] + IK(i, k, kStride)));
		
		if (m != INF)
		{
			for (size_t k = 0; k < n; k++)
				*(&Reduced[0][0] + IK(i, k, kStride)) -= m;
			LowerBoundTimesTwo += m;
		}
	}

	void Reduce()
	{
		for (size_t i = 0; i < n; i++)
			Reduce(EdgeType::Outgoing, i);

		for (size_t j = 0; j < n; j++)
			Reduce(EdgeType::Incoming, j);
	}

	void Print(uint32_t D[N][N])
	{
		std::cout << Cost - D[n - 1][0] << std::endl;
		//for (size_t i = 1; i < n; i++)
		//	std::cout << Path[i - 1] << " " << Path[i] << " " << D[Path[i - 1]][Path[i]] << std::endl;
	}

	bool IsComplete()
	{
		Path = TraverseSubPath(0, EdgeType::Outgoing);
		return Path.size() == n + 1 && Path[n - 1] == n - 1;
	}

	int IK(size_t i, size_t k, size_t kStride)
	{
		return (N + 1 - kStride)*i + kStride*k;
	}

	PartialSolution(size_t n, uint32_t D[N][N]) : n(n)
	{
		memcpy(Reduced, D, sizeof(Reduced[0][0]) * N * N);
		memset(Constraints, 0, sizeof(Constraints[0][0]) * N * N);
		for (size_t i = 0; i < n; i++)
		{
			Reduced[i][i] = INF;
			Constraints[i][i] = -1;
		}
		Cost = 0;

		Reduce();
	}

	PartialSolution()
	{
	}

	size_t EnabledEdges = 0, DisabledEdges = 0;
	size_t n = 0;
	uint32_t Cost = INF;
	uint32_t LowerBoundTimesTwo = 0;
	uint32_t Reduced[N][N];
	int8_t Constraints[N][N];
	vector<size_t> Path;
};

void branch_and_bound(size_t n, uint32_t D[N][N])
{
	PartialSolution bestCompleteSolution;
	PartialSolution root = PartialSolution(n, D).WithEdge(Edge(n - 1, 0), D);

	static priority_queue<PartialSolution, vector<PartialSolution>, greater<PartialSolution> > right;
	static stack<PartialSolution> left;
	left.push(root);

	while (!left.empty() || !right.empty())
	{
		auto currentSolution = !left.empty() ? left.top() : right.top();
		if (!left.empty())
			left.pop();
		else
			right.pop();
	
		if (currentSolution.IsComplete())
		{
			if (currentSolution.Cost < bestCompleteSolution.Cost)
			{
				bestCompleteSolution = currentSolution;
				bestCompleteSolution.Print(D);
			}
		}
		else if (currentSolution.LowerBoundTimesTwo < bestCompleteSolution.Cost)
		{
			auto pivot = currentSolution.ChoosePivotEdge();
			if (pivot != NullEdge)
			{
				auto withPivot = currentSolution.WithEdge(pivot, D);
				auto withoutPivot = currentSolution.WithoutEdge(pivot, D);

				if (withPivot.LowerBoundTimesTwo < bestCompleteSolution.Cost)
					left.push(withPivot);

				if (withoutPivot.LowerBoundTimesTwo < bestCompleteSolution.Cost)
					right.push(withoutPivot);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	freopen(argv[1], "r", stdin);
	
	size_t n;
	uint32_t D[N][N];

	std::cin >> n;
	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < n; j++)
			std::cin >> D[i][j];

	branch_and_bound(n, D);
}