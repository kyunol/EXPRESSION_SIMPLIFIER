#include <string>
#include <vector>
#include <stack>
#include "exp_value.h"

using namespace std;

#ifndef EXP_SOLVER_H
#define EXP_SOLVER_H

struct Variable {
	string name;
	Value value;
	Variable(string nm, Value val)
		: name(nm), value(val) {}
};

struct Function {
	string name;
	double (*func)(double);
	Function(string nm, double (*f)(double))
		: name(nm), func(f) {}
};

enum BlockType {
	Num, Sym, Func, Constant, Var, BracL, BracR, Nil
};

struct Block {
	int start, end, level;
	BlockType type;
	Block() : start(0), end(0), level(0), type(Nil) {}
	Block(int s, int e, int l, BlockType tp)
		: start(s), end(e), level(l), type(tp) {}
};

class ExpSolver {
public:
	ExpSolver(void);
	string solveExp(string);

private:
	vector<Block> blocks;

	vector<Variable> variables;
	vector<Variable> constants;
	vector<Function> functions;

	void addPredefined(void);
	string discardSpaces(string str);

	bool checkDeclaration(string& exp, string& newVarName, bool& isDec);

	bool groupExp(string exp);
	BlockType analyzeStrType(string str);

	BlockType charType(char c);

	void dealWithNegativeSign(string& exp);

	Value calculateExp(string exp, int startBlock, int endBlock);
	int findIndexOfBracketEnding(int blockId);

	void printStacks(stack<Value> values, stack<char> ops);
};

#endif