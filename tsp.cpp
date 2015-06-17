#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <functional>
#include <vector>
#include <bitset>
#include <queue>
#include <utility>
using namespace std;

const int N = 50;
const int INF = 1e+6;
typedef pair < int, int > Edge;

struct PartialSolution
{
	int n = -1;
	int32_t Cost = INF;
	int32_t LowerBound = 0.0;
	int32_t Reduced[N][N];
	int8_t Constraints[N][N];
	vector<int> Path;

	PartialSolution WithEdge(Edge pivot, int32_t D[N][N])
	{
		int i = pivot.first, j = pivot.second;
		
		PartialSolution child = *this;
		child.Cost += D[i][j];
		for (int k = 0; k < n; k++)
		{
			child.Constraints[i][k] = child.Constraints[k][j] = -1;
			child.Reduced[i][k] = child.Reduced[k][j] = INF;
		}
		child.Reduce();
		child.Constraints[i][j] = child.Constraints[j][i] = 1;
		
		return child;
	}

	PartialSolution WithoutEdge(Edge pivot, int32_t D[N][N])
	{
		int i = pivot.first, j = pivot.second;
		
		PartialSolution child = *this;
		child.Constraints[i][j] = child.Constraints[j][i] = -1;
		child.Reduced[i][j] = child.Reduced[j][i] = INF;
		child.Reduce("row", i);
		child.Reduce("column", j);

		return child;
	}

	bool operator>(const PartialSolution& other) const
	{
		return Cost > other.Cost;
	}

	Edge ChoosePivotEdge()
	{
		auto minStride = [&](int32_t* begin, int32_t* end, int stride) {int32_t m = INF; for (int32_t* p = begin; p < end; p += stride) if (*p < m) m = *p; return m; };
		auto rowMin = [&](int k) { return minStride(&Reduced[k][0], &Reduced[k][n], 1); };
		auto columnMin = [&](int k) { return minStride(&Reduced[0][k], &Reduced[n-1][n-1], N); };
		
		int bestIncrease = 0.0;
		Edge bestPivot = make_pair(-1, -1);
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				if (Constraints[i][j] == 0 && Reduced[i][j] == 0.0)
				{
					Reduced[i][j] = INF;
					int increase = rowMin(i) + columnMin(j);
					if (increase > bestIncrease)
					{
						bestIncrease = increase;
						bestPivot = make_pair(i, j);
					}
					Reduced[i][j] = 0.0;
				}
			}
		}

		return bestPivot;
	}

	void Print()
	{
		printf("cost: %8.2f [0", Cost);
		for (int i = 1; i < n; i++)
			printf("->%d", Path[i]);
		puts("]");
	}

	bool IsComplete()
	{
		bitset<N> visited;
		Path.resize(N);

		int from = 0;
		visited.set(from);
		Path[0] = from;
		for (int i = 0; i < n; i++)
		{
			int to = -1;
			for (int j = 0; j < n; j++)
			{
				if (j != from && Constraints[from][j] == 1)
				{
					if (visited.at(j) || to != -1)
						return false;

					to = j;
				}
			}

			Path[i] = to;
			from = to;
			visited.set(to);
		}

		return visited.count() == n;
	}

	void Reduce()
	{
		for (int i = 0; i < n; i++)
			Reduce(Row, i);

		for (int j = 0; j < n; j++)
			Reduce(Column, j);
	}

	void Reduce(PartialSolution::ReductionType reductionType, int i)
	{
		auto accessor = [&](int a, int b) -> int& {return reductionType == Row? Reduced[a][b] : Reduced[b][a]; };

		int m = INF;
		for (int j = 0; j < n; j++)
			m = min(m, accessor(i, j));
		for (int j = 0; j < n; j++)
			accessor(i, j) -= m;
		LowerBound += m;
	}

	PartialSolution(int n, int32_t D[N][N]) : n(n)
	{
		memcpy(Reduced, D, sizeof(Reduced[0][0]) * N * N);
		memset(Constraints, 0, sizeof(Constraints[0][0]) * N * N);
		for (int i = 0; i < n; i++)
			Reduced[i][i] = INF;

		Reduce();
	}

	PartialSolution()
	{

	}

	enum ReductionType
	{
		Row,
		Column
	};
};

void branch_and_bound(int n, int32_t D[N][N])
{
	PartialSolution bestCompleteSolution;
	PartialSolution root(n, D);
	
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
		else if (currentSolution.LowerBound < bestCompleteSolution.Cost)
		{
			auto pivot = currentSolution.ChoosePivotEdge();
			auto withPivot = currentSolution.WithEdge(pivot, D);
			auto withoutPivot = currentSolution.WithoutEdge(pivot, D);

			if (withPivot.LowerBound < bestCompleteSolution.Cost)
				Q.push(withPivot);

			if (withoutPivot.LowerBound < bestCompleteSolution.Cost)
				Q.push(withoutPivot);
		}
	}
}

int main()
{
	freopen("input.txt", "r", stdin);

	int n;
	int32_t D[N][N];

	scanf("%d", &n);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			scanf("%d", &D[i][j]);

	branch_and_bound(n, D);
}