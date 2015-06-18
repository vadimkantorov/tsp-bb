use std::env;
use std::cmp::Ordering;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::BinaryHeap;
use std::cmp::min;

const N : usize = 50;
const INF : u32 = 1e+9 as u32;

#[derive(Copy, Clone, Eq, PartialEq)]
struct Edge
{
	first : usize,
	second : usize
}

const NullEdge : Edge = Edge { first: N, second: N};

//TODO: should be a function/closure that does (arr, i, j) => arr[i][j] for Row and arr[j][i] for column
struct RowColumnAccessor;
const Row : RowColumnAccessor = RowColumnAccessor;
const Column : RowColumnAccessor = RowColumnAccessor;

#[derive(Eq, PartialEq)]
struct PartialSolution
{
	n : usize,
	Cost : u32,
	LowerBoundTimesTwo : u32,
	Reduced : Vec<Vec<u32>>,
	Constraints : Vec<Vec<i8>>,
	Path : Vec<usize>
}

impl Ord for PartialSolution
{
	fn cmp(&self, other: &PartialSolution) -> Ordering
	{
		self.LowerBoundTimesTwo.cmp(&other.LowerBoundTimesTwo)
	}
}

impl PartialOrd for PartialSolution
{
	fn partial_cmp(&self, other: &PartialSolution) -> Option<Ordering>
	{
		Some(self.cmp(other))
	}
}

impl Clone for PartialSolution
{
	//TODO: implement proper clone with cloning Reduced and Constraints
	fn clone(&self) -> PartialSolution
	{
		PartialSolution::InfiniteCostSolution()
	}
}

impl PartialSolution
{
	fn ReduceByType(&mut self, a : RowColumnAccessor, i : usize)
	{
		let m = INF;
		for k in 0..self.n
		{
//			if a(self.Constraints, i, k) != -1
			{
//				m = min(m, a(self.Reduced, i, k));
			}
		}
		
		if (m != INF)
		{
			for k in 0..self.n
			{
//				a(self.Reduced, i, k) -= m;
			}
			self.LowerBoundTimesTwo += m;
		}
	}

	fn Reduce(&mut self)
	{
		for i in 0..self.n
		{
			self.ReduceByType(Row, i);
		}

		for j in 0..self.n
		{
			self.ReduceByType(Column, j);
		}
	}

	fn InfiniteCostSolution() -> PartialSolution
	{
		PartialSolution {n : 0, Cost : INF, LowerBoundTimesTwo : 0, Path : vec![], Reduced : vec![vec![0u32; N]; N], Constraints : vec![vec![0i8; N]; N] }
	}

	fn new(n : usize, D : &Vec<Vec<u32>>) -> PartialSolution
	{
		let mut res = PartialSolution {n : n, Cost : 0, LowerBoundTimesTwo : 0, Path : vec![], Constraints : vec![vec![0i8; N]; N], Reduced : D.clone()};

		for i in 0..n
		{
			res.Reduced[i][i] = INF;
			res.Constraints[i][i] = -1;
		}
		
		res.Reduce();
		res
	}
	
	fn ChoosePivotEdge(&self) -> Edge
	{
		//let minRC = |except: usize, k : usize, a : RowColumnAccessor| {let mut m = INF; for i in 0..self.n { if i != except {m = min(m, a(self.Reduced, i, k));} } m };
		//let rowMin = |k : usize, except : usize| minRC(except, k, Row);
		//let columnMin = |k : usize, except : usize| minRC(except, k, Column);
		
		let mut bestIncrease = 0u32;
		let mut bestPivot = NullEdge;
		for i in 0..self.n
		{
			for j in 0..self.n
			{
				if self.Constraints[i][j] == 0 && self.Reduced[i][j] == 0
				{
					let increase = 0u32;//rowMin(i, j) + columnMin(j, i);
					if increase > bestIncrease
					{
						bestIncrease = increase;
						bestPivot = Edge { first : i, second : j};
					}
				}
			}
		}

		return bestPivot;
	}

	fn WithEdge(&self, pivot : Edge, D: &Vec<Vec<u32>>) -> PartialSolution
	{
		let Edge { first: i, second: j } = pivot;
		
		let mut child = self.clone();
		child.Cost += D[i][j];
		for k in 0..self.n
		{
			child.Constraints[i][k] = -1;
			child.Constraints[k][j] = -1;
			child.Reduced[i][k] = INF;
			child.Reduced[k][j] = INF;
		}

		child.Constraints[i][j] = 1;
		child.Constraints[j][i] = -1;

		let subpathTo = child.TraverseSubPath(i, Row);
		let subpathFrom = child.TraverseSubPath(i, Column);
		if subpathTo.len() + subpathFrom.len() - 1 != self.n
		{
			let toBack = *subpathTo.last().unwrap();
			let fromBack = *subpathFrom.last().unwrap(); 		
			child.Constraints[toBack][fromBack] = -1;
			child.Reduced[toBack][fromBack] = INF;
		}

		child.Reduce();
		child
	}

	fn WithoutEdge(&self, pivot : Edge, D: &Vec<Vec<u32>>) -> PartialSolution
	{
		let Edge { first: i, second: j } = pivot;
		
		let mut child = self.clone();
		child.Constraints[i][j] = -1;
		child.Reduced[i][j] = INF;
		child.ReduceByType(Row, i);
		child.ReduceByType(Column, j);

		return child;
	}

	fn TraverseSubPath(&self, mut cur : usize, a : RowColumnAccessor) -> Vec<usize>
	{
		let mut subpath = vec![cur];
		for k in 0..self.n
		{
			let mut next = N;
			for i in 0..self.n
			{
//				if a(Constraints, cur, i)
				{
					next = i;
					break;
				}
			}

			if next == N
			{
				break;
			}

			subpath.push(next);
			cur = next;
		}
		subpath
	}

	fn IsComplete(&mut self) -> bool
	{
		self.Path = self.TraverseSubPath(0, Row);
		self.Path.len() == self.n + 1 && self.Path[self.n - 1] == self.n - 1
	}

	fn Print(&self, D: &Vec<Vec<u32>>)
	{
		println!("{}", self.Cost - D[self.n - 1][0]);
		for i in 1..self.n
		{
			println!("{} {} {}\n", self.Path[i - 1], self.Path[i], D[self.Path[i - 1]][self.Path[i]])
		}
		println!("\n")
	}
}

fn branch_and_bound(n : usize, D : &Vec<Vec<u32>>)
{
	let mut bestCompleteSolution = PartialSolution::InfiniteCostSolution();
	let root = PartialSolution::new(n, D).WithEdge(Edge { first : n - 1, second : 0}, D);

	let mut Q = BinaryHeap::new();
	Q.push(root);

	while let Some(mut currentSolution) = Q.pop()
	{
		//TODO: possibly refactor to use Option, remove Path field from PartialSolution, make Print a free function instead of a method
		if currentSolution.IsComplete()
		{
			if currentSolution.Cost < bestCompleteSolution.Cost
			{
				bestCompleteSolution = currentSolution;
				bestCompleteSolution.Print(D);
			}
		}
		else if currentSolution.LowerBoundTimesTwo < 2 * bestCompleteSolution.Cost
		{
			let pivot = currentSolution.ChoosePivotEdge();
			if pivot != NullEdge
			{
				let withPivot = currentSolution.WithEdge(pivot, D);
				let withoutPivot = currentSolution.WithoutEdge(pivot, D);

				if withPivot.LowerBoundTimesTwo < 2 * bestCompleteSolution.Cost
				{
					Q.push(withPivot);
				}

				if withoutPivot.LowerBoundTimesTwo < 2 * bestCompleteSolution.Cost
				{
					Q.push(withoutPivot);
				}
			}
		}
	}
}

fn main()
{
	let mut args : Vec<String> = env::args().collect();
	if args.len() < 2
	{
		//TODO: must output 36
		args.push("tests/asanov/input.txt".to_string());
	}
	
	let mut f = BufReader::new(File::open(args[1].to_string()).unwrap());
	let mut num_line = String::new();
	f.read_line(&mut num_line).unwrap();
	let n: usize = num_line.trim().parse().unwrap();
	
	let D: Vec<Vec<u32>> = f.lines()
		.take(n)
		.map(|l| l.unwrap().split(char::is_whitespace)
		.take(n)
		.map(|number| number.parse().unwrap())
		.collect())
	.collect();

	branch_and_bound(n, &D);
}