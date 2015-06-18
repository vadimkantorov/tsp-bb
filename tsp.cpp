#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <functional>
#include <vector>
#include <bitset>
#include <queue>
#include <utility>
using namespace std;

const size_t N = 50;
const uint32_t INF = uint32_t(1e+6);
typedef pair < size_t, size_t > Edge;

struct PartialSolution
{
	enum class ReductionType
	{
		Row,
		Column,
	};

	size_t n = 0;
	uint32_t Cost = INF;
	uint32_t LowerBoundTimesTwo = 0;
	uint32_t Reduced[N][N];
	int8_t Constraints[N][N];
	vector<size_t> Path;

	

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

		child.Constraints[i][j] = 1;
		child.Constraints[j][i] = -1;

		auto traverseSubPath = [&](size_t cur, int stride)
		{
			vector<size_t> subpath{ cur };
			while (true)
			{
				size_t next = N;
				for (size_t i = 0; i < n; i++)
				{
					if (*(&child.Constraints[0][0] + IK(cur, i, stride)) == 1)
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
		};

		auto subpathTo = traverseSubPath(i, 1);
		auto subpathFrom = traverseSubPath(i, N);

		child.Constraints[subpathTo.back()][subpathFrom.back()] = -1;
		child.Reduced[subpathTo.back()][subpathFrom.back()] = INF;
		child.Reduce();
		return child;
	}

	PartialSolution WithoutEdge(Edge pivot, uint32_t D[N][N])
	{
		auto i = pivot.first, j = pivot.second;
		
		PartialSolution child = *this;
		child.Constraints[i][j] = -1;
		child.Reduced[i][j] = INF;
		child.Reduce(ReductionType::Row, i);
		child.Reduce(ReductionType::Column, j);

		return child;
	}

	bool operator>(const PartialSolution& other) const
	{
		return Cost > other.Cost;
	}

	Edge ChoosePivotEdge()
	{
		auto minStride = [&](size_t k, size_t kStride) {uint32_t m = INF; for (size_t i = 0; i < n; i++) m = min(m, *(&Reduced[0][0] + IK(i, k, kStride))); return m;  };
		auto rowMin = [&](size_t k) {return minStride(k, N); };
		auto columnMin = [&](size_t k) {return minStride(k, 1); };
		
		uint32_t bestIncrease = 0;
		Edge bestPivot = make_pair(-1, -1);
		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = 0; j < n; j++)
			{
				if (Constraints[i][j] == 0 && Reduced[i][j] == 0)
				{
					Reduced[i][j] = INF;
					auto increase = rowMin(i) + columnMin(j);
					if (increase > bestIncrease)
					{
						bestIncrease = increase;
						bestPivot = make_pair(i, j);
					}
					Reduced[i][j] = 0;
				}
			}
		}

		return bestPivot;
	}

	void Print()
	{
		printf("cost: %d [0", Cost);
		for (size_t i = 1; i < n; i++)
			printf("->%d", Path[i]);
		puts("]");
	}

	bool IsComplete()
	{
		bitset<N> visited;
		Path.resize(N);

		size_t from = 0;
		visited.set(from);
		Path[0] = from;
		for (size_t i = 0; i < n - 1; i++)
		{
			size_t to = N;
			for (size_t j = 0; j < n; j++)
			{
				if (j != from && Constraints[from][j] == 1)
				{
					if (visited[j] || to != N)
						return false;

					to = j;
				}
			}

			if (to == N)
				break;

			Path[i] = to;
			from = to;
			visited.set(to);
		}

		return from == n-1 && visited.count() == n;
	}

	void Reduce()
	{
		for (size_t i = 0; i < n; i++)
			Reduce(ReductionType::Row, i);

		for (size_t j = 0; j < n; j++)
			Reduce(ReductionType::Column, j);
	}

	void Reduce(PartialSolution::ReductionType reductionType, size_t i)
	{
		auto kStride = reductionType == ReductionType::Row ? 1 : N;
		
		uint32_t m = INF;
		for (size_t k = 0; k < n; k++)
			m = min(m, *(&Reduced[0][0] + IK(i, k, kStride)));

		for (size_t k = 0; k < n; k++)
			*(&Reduced[0][0] + IK(i, k, kStride)) -= m;
		LowerBoundTimesTwo += m;
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
};

void branch_and_bound(size_t n, uint32_t D[N][N])
{
	PartialSolution bestCompleteSolution;
	PartialSolution root = PartialSolution(n, D).WithEdge(make_pair(0, n - 1), D);
	
	priority_queue<PartialSolution, vector<PartialSolution>, greater<PartialSolution> > Q;
	Q.push(root);

	while (!Q.empty())
	{
		auto currentSolution = Q.top();
		Q.pop();

		if (currentSolution.IsComplete())
		{
			if (currentSolution.Cost < bestCompleteSolution.Cost)
			{
				bestCompleteSolution = currentSolution;
				bestCompleteSolution.Print();
			}
		}
		else if (currentSolution.LowerBoundTimesTwo / 2 < bestCompleteSolution.Cost)
		{
			auto pivot = currentSolution.ChoosePivotEdge();
			auto withPivot = currentSolution.WithEdge(pivot, D);
			auto withoutPivot = currentSolution.WithoutEdge(pivot, D);

			if (withPivot.LowerBoundTimesTwo / 2 < bestCompleteSolution.Cost)
				Q.push(withPivot);

			if (withoutPivot.LowerBoundTimesTwo / 2 < bestCompleteSolution.Cost)
				Q.push(withoutPivot);
		}
	}
}

int main()
{
	freopen("tests/asanov/input.txt", "r", stdin);

	size_t n;
	uint32_t D[N][N];

	scanf("%u", &n);
	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < n; j++)
			scanf("%u", &D[i][j]);

	branch_and_bound(n, D);
}