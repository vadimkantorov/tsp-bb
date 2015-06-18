#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <functional>
#include <vector>
#include <queue>
#include <utility>
using namespace std;

const size_t N = 50;
const uint32_t INF = uint32_t(1e+9);

struct Edge
{
	int i, j;
	enum class Type
	{
		Outgoing,
		Incoming
	};

	Edge(size_t i = N, size_t j = N) : i(i), j(j)
	{
	}

	bool IsNull()
	{
		return i == N && j == N;
	}
};

struct PartialSolution
{
	size_t n = 0;
	uint32_t Cost = INF;
	uint32_t LowerBoundTimesTwo = 0;
	uint32_t Reduced[N][N];
	int8_t Constraints[N][N];
	vector<size_t> Path;

	PartialSolution WithEdge(Edge pivot, uint32_t D[N][N])
	{
		auto i = pivot.i, j = pivot.j;
		
		PartialSolution child = *this;
		child.Cost += D[i][j];
		for (size_t k = 0; k < n; k++)
		{
			child.Constraints[i][k] = child.Constraints[k][j] = -1;
			child.Reduced[i][k] = child.Reduced[k][j] = INF;
		}

		child.Constraints[i][j] = 1;
		child.Constraints[j][i] = -1;

		auto subpathTo = child.TraverseSubPath(i, Edge::Type::Outgoing);
		auto subpathFrom = child.TraverseSubPath(i, Edge::Type::Incoming);
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
		auto i = pivot.i, j = pivot.j;
		
		PartialSolution child = *this;
		child.Constraints[i][j] = -1;
		child.Reduced[i][j] = INF;
		child.Reduce(Edge::Type::Outgoing, i);
		child.Reduce(Edge::Type::Incoming, j);

		return child;
	}

	bool operator>(const PartialSolution& other) const
	{
		return LowerBoundTimesTwo > other.LowerBoundTimesTwo;
	}

	Edge ChoosePivotEdge()
	{
		auto minStride = [&](size_t k, size_t kStride) {uint32_t m = INF; for (size_t i = 0; i < n; i++) m = min(m, *(&Reduced[0][0] + IK(i, k, kStride))); return m;  };
		auto rowMin = [&](size_t k) {return minStride(k, N); };
		auto columnMin = [&](size_t k) {return minStride(k, 1); };
		
		uint32_t bestIncrease = 0;
		Edge bestPivot;
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
						bestPivot = Edge(i, j);
					}
					Reduced[i][j] = 0;
				}
			}
		}

		return bestPivot;
	}

	void Print(uint32_t D[N][N])
	{
		printf("%d\n", Cost - D[n-1][0]);
		for (size_t i = 1; i < n; i++)
			printf("%d %d %d\n", Path[i-1], Path[i], D[Path[i-1]][Path[i]]);
		printf("\n\n");
	}

	vector<size_t> TraverseSubPath(size_t cur, Edge::Type edgeType)
	{
		auto stride = edgeType == Edge::Type::Outgoing ? 1 : N;
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

	bool IsComplete()
	{
		Path = TraverseSubPath(0, Edge::Type::Outgoing);
		return Path.size() == n + 1 && Path[n - 1] == n - 1;
	}

	void Reduce()
	{
		for (size_t i = 0; i < n; i++)
			Reduce(Edge::Type::Outgoing, i);

		for (size_t j = 0; j < n; j++)
			Reduce(Edge::Type::Incoming, j);
	}

	void Reduce(Edge::Type edgeType, size_t i)
	{
		auto kStride = edgeType == Edge::Type::Outgoing ? 1 : N;
		
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
	PartialSolution root = PartialSolution(n, D).WithEdge(Edge(n - 1, 0), D);

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
				bestCompleteSolution.Print(D);
			}
		}
		else if (currentSolution.LowerBoundTimesTwo < 2 * bestCompleteSolution.Cost)
		{
			auto pivot = currentSolution.ChoosePivotEdge();
			if (!pivot.IsNull())
			{
				auto withPivot = currentSolution.WithEdge(pivot, D);
				auto withoutPivot = currentSolution.WithoutEdge(pivot, D);

				if (withPivot.LowerBoundTimesTwo < 2 * bestCompleteSolution.Cost)
					Q.push(withPivot);

				if (withoutPivot.LowerBoundTimesTwo < 2 * bestCompleteSolution.Cost)
					Q.push(withoutPivot);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		argv = new char*[] {argv[1], "tests/asanov/input.txt"};
	}
	freopen(argv[1], "r", stdin);

	size_t n;
	uint32_t D[N][N];

	scanf("%u", &n);
	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < n; j++)
			scanf("%u", &D[i][j]);

	branch_and_bound(n, D);
}