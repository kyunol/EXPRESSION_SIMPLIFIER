#include <iostream>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <iomanip>
#include "exp_solver.h"

using namespace std;

ExpSolver::ExpSolver() {
	addPredefined();
}

string ExpSolver::solveExp(string exp) {
	exp = discardSpaces(exp);

	bool isDeclaration = false;
	string newVarName = "";
	bool declarationValid = checkDeclaration(exp, newVarName, isDeclaration);
	if (!declarationValid) {
		blocks.clear();
		return "Calculation aborted. ";
	}

	if (exp.length() == 0) {
		cerr << "Invalid expression! ";
		blocks.clear();
		return "Calculation aborted. ";
	}

	dealWithNegativeSign(exp);

	bool groupSucceed = groupExp(exp);
	if (!groupSucceed) {
		blocks.clear();
		return "Calculation aborted. ";
	}

	Value result = calculateExp(exp, 0, blocks.size());

	string output;
	if (result.getCalculability()) {
		if (isDeclaration) {
			bool variableCanBeDeclared = true;

			for (int i = 0; i < constants.size(); i++) {
				if (newVarName.compare(constants[i].name) == 0) {
					variableCanBeDeclared = false;
					cerr << "Constant \"" << newVarName << "\" cannot be declared! ";
					blocks.clear();
					return "Calculation aborted. ";
				}
			}

			bool variableDeclaredBefore = false;

			for (int i = 0; i < variables.size(); i++) {
				if (newVarName.compare(variables[i].name) == 0) {
					variableDeclaredBefore = true;
					variables[i] = Variable(newVarName, result);
				}
			}
			if (!variableDeclaredBefore) {
				variables.push_back(Variable(newVarName, result));
			}
			cout << newVarName << " = " << result.printValue();
		}
		else {
			constants[constants.size() - 1] = Variable("ans", result);

			output = "Ans = " + result.printValue();
		}
	}
	else {
		blocks.clear();
		return "Calculation aborted. ";
	}
	blocks.clear();

	return output;
}

void ExpSolver::addPredefined() {
	constants.push_back(Variable("e", Value(2.711828183)));
	constants.push_back(Variable("pi", Value(3.14159265)));
	constants.push_back(Variable("ans", Value()));
	functions.push_back(Function("sin", sin));
	functions.push_back(Function("cos", cos));
	functions.push_back(Function("tan", tan));
	functions.push_back(Function("exp", exp));
	functions.push_back(Function("sqrt", sqrt));
	functions.push_back(Function("floor", floor));
	functions.push_back(Function("ln", log));
	functions.push_back(Function("log", log10));
}

string ExpSolver::discardSpaces(string str) {
	string newStr = "";
	for (int i = 0; i < str.length(); i++) {
		if (!isspace(str[i])) newStr += str[i];
	}
	return newStr;
}
bool ExpSolver::checkDeclaration(string& exp, string& newVarName, bool& isDec) {
	int found = exp.find('=');

	if (found != string::npos) {
		newVarName = exp.substr(0, found);
		isDec = true;
		exp = exp.substr(found + 1);

		if (exp.find('=') != string::npos) {
			cerr << "Syntax error: Too many '='! ";
			return false;
		}

		if (newVarName.length() == 0 || !isalpha(newVarName[0])) {
			cerr << "Variable name invalid! ";
			return false;
		}
		for (int i = 1; i < newVarName.length(); i++) {
			if (!(isalnum(newVarName[i]) || newVarName[i] == '_')) {
				cerr << "Variable name invalid! ";
				return false;
			}
		}
	}

	return true;
}

bool ExpSolver::groupExp(string exp) {
	Block newBlock;

	int start = 0, level = 0;

	BlockType currentType = Nil;


	for (int i = 0; i <= exp.length(); i++) {
		BlockType thisType = charType(exp[i]);

		bool needNewBlock = false;
		needNewBlock |= (currentType == BracL);
		needNewBlock |= (currentType == BracR);
		needNewBlock |= (currentType == Sym);
		needNewBlock |= (thisType != currentType);
		needNewBlock |= (i == exp.length());
		needNewBlock &= !(thisType == Num && currentType == Func);

		if (needNewBlock) {
			if (currentType == Func) {
				currentType = analyzeStrType(exp.substr(start, i - start));
				if (currentType == Nil) return false;
			}

			if (i != 0) {
				newBlock = Block(start, i, level, currentType);
				blocks.push_back(newBlock);
			}

			if (currentType == BracR) level--;

			currentType = thisType;
			start = i;
		}
		if (thisType == BracL) level++;
	}
	if (level != 0) {
		cerr << "Syntax error: Brackets not paired! ";
		return false;
	}

	return true;
}
BlockType ExpSolver::analyzeStrType(string str) {
	for (int i = 0; i < functions.size(); i++) {
		if (str.compare(functions[i].name) == 0) return Func;
	}
	for (int i = 0; i < constants.size(); i++) {
		if (str.compare(constants[i].name) == 0) return Constant;
	}
	for (int i = 0; i < variables.size(); i++) {
		if (str.compare(variables[i].name) == 0) return Var;
	}
	cerr << "String \"" + str + "\" not recognized! ";
	return Nil;
}

BlockType ExpSolver::charType(char c) {
	if (c == '_' || isalpha(c)) return Func;
	else if (c == '.' || isdigit(c)) return Num;
	else if (c == '(') return BracL;
	else if (c == ')') return BracR;
	else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') return Sym;
	else return Nil;
}

void ExpSolver::dealWithNegativeSign(string& exp) {
	if (exp.length() > 0 && exp[0] == '-') {
		exp = '0' + exp;
	}
	for (int i = 1; i < exp.length(); i++) {
		if (exp[i] == '-' && exp[i - 1] == '(') {
			exp = exp.substr(0, i) + '0' + exp.substr(i);
		}
	}
}
Value ExpSolver::calculateExp(string exp, int startBlock, int endBlock) {

	stack<Value> values;
	stack<char> ops;

	for (int i = endBlock - 1; i >= startBlock; i--) {
		string blockStr = exp.substr(blocks[i].start, blocks[i].end - blocks[i].start);
		int iIncrement = 0;

		if (blocks[i].type == Num) {
			values.push(Value(blockStr));
		}

		else if (blocks[i].type == Func) {
			cerr << "Syntax Error: Need brackets after function name! ";
			return Value();
		}

		else if (blocks[i].type == Constant) {
			Value constValue;
			for (int i = 0; i < constants.size(); i++) {
				if (blockStr.compare(constants[i].name) == 0) {
					constValue = constants[i].value;

					if (!constValue.getCalculability()) {
						cerr << "Bad access: \"ans\" not defined currently! ";
						return Value();
					}

					break;
				}
			}
			values.push(constValue);
		}

		else if (blocks[i].type == Var) {
			Value varValue;
			for (int i = 0; i < variables.size(); i++) {
				if (blockStr.compare(variables[i].name) == 0) {
					varValue = variables[i].value;
					break;
				}
			}
			values.push(varValue);
		}
		else if (blocks[i].type == BracR) {
			int corBlock = findIndexOfBracketEnding(i);
			if (corBlock != 0 && blocks[corBlock - 1].type == Func) {
				double (*funcToUse)(double)=0;
				string funcName = exp.substr(blocks[corBlock - 1].start,
					blocks[corBlock - 1].end - blocks[corBlock - 1].start);
				for (int i = 0; i < functions.size(); i++) {
					if (funcName.compare(functions[i].name) == 0) {
						funcToUse = functions[i].func;
						break;
					}
				}
				Value valueInFunc = calculateExp(exp, corBlock + 1, i);
				if (!valueInFunc.getCalculability()) {
					return Value();
				}
				else if (valueInFunc.getDecValue() < 0 && funcName.compare("sqrt") == 0) {
					cerr << "Arithmatic error: Cannot square root a negative number! ";
					return Value();
				}
				double funcResult = (*funcToUse)(valueInFunc.getDecValue());

				Value newValue = Value(funcResult);
				values.push(newValue);

				iIncrement -= i - corBlock + 1;
			}
			else {
				Value valueToPush = calculateExp(exp, corBlock + 1, i);
				if (!valueToPush.getCalculability()) {
					return Value();
				}
				values.push(valueToPush);
				iIncrement -= i - corBlock;
			}
		}
		else if (blocks[i].type == Sym) {
			if (blockStr[0] == '*' || blockStr[0] == '/') {
				if (values.size() != ops.size() + 1) {
					cerr << "Invalid expression! ";
					return Value();
				}
				while (!ops.empty()) {
					char lastOp = ops.top();
					if (lastOp == '^') {
						ops.pop();
						Value op1 = values.top(); values.pop();
						Value op2 = values.top(); values.pop();
						values.push(powv(op1, op2));
					}
					else {
						break;
					}
				}
			}
			else if (blockStr[0] == '+' || blockStr[0] == '-') {
				if (values.size() != ops.size() + 1) {
					cerr << "Invalid expression! ";
					return Value();
				}
				while (!ops.empty()) {
					char lastOp = ops.top();
					if (!(lastOp == '+' || lastOp == '-')) {
						ops.pop();
						Value op1 = values.top(); values.pop();
						Value op2 = values.top(); values.pop();
						if (lastOp == '*') values.push(op1 * op2);
						else if (lastOp == '/') values.push(op1 / op2);
						else if (lastOp == '^') values.push(powv(op1, op2));
					}
					else {
						break;
					}
				}
			}
			ops.push(blockStr[0]);
		}
		else {
			cerr << "Encountered unknown character! ";
			return Value();
		}

		i += iIncrement;
	}

	if (values.size() != ops.size() + 1) {
		cerr << "Invalid expression! ";
		return Value();
	}
	while (!ops.empty()) {
		char lastOp = ops.top();
		ops.pop();
		Value op1 = values.top(); values.pop();
		Value op2 = values.top(); values.pop();
		if (lastOp == '+') values.push(op1 + op2);
		else if (lastOp == '-') values.push(op1 - op2);
		else if (lastOp == '*') values.push(op1 * op2);
		else if (lastOp == '/') values.push(op1 / op2);
		else if (lastOp == '^') values.push(powv(op1, op2));
	}
	if (values.size() > 1) {
		cerr << "Invalid expression! ";
		return Value();
	}

	Value returnValue = values.top();

	while (!ops.empty()) { ops.pop(); }
	while (!values.empty()) { values.pop(); }

	return returnValue;
}

int ExpSolver::findIndexOfBracketEnding(int blockId) {
	int levelToFind = blocks[blockId].level - 1, currentBlockId = blockId;
	while (blocks[currentBlockId].level != levelToFind) {
		if (currentBlockId == 0) return currentBlockId;
		currentBlockId--;
	}
	return currentBlockId + 1;
}
void ExpSolver::printStacks(stack<Value> values, stack<char> ops) {
	cout << left << setw(8) << "Values:";
	vector<Value> valuesv;
	while (!values.empty()) {
		valuesv.push_back(values.top());
		values.pop();
	}
	for (int i = valuesv.size() - 1; i >= 0; i--) {
		cout << setw(12) << valuesv[i];
	}
	cout << endl;
	cout << setw(8) << "Ops:";
	vector<char> opsv;
	while (!ops.empty()) {
		opsv.push_back(ops.top());
		ops.pop();
	}
	for (int i = opsv.size() - 1; i >= 0; i--) {
		cout << setw(12) << opsv[i];
	}
	cout << endl;
}