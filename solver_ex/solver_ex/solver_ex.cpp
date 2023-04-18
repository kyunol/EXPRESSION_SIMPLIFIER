#include <iostream>
#include "exp_solver.h"

using namespace std;

int main() {
	cout << "| Welcome to expression solver developed by Jingyun Yang!" << endl;
	cout << "| To use this program, type in expressions or declarations for it to solve." << endl;
	cout << "| To quit, enter \"quit\" and press [Enter]." << endl;
	cout << "| Enjoy!" << endl << endl;

	ExpSolver mySolver = ExpSolver();

	string input;

	while (1) {
		cout << "| >> ";

		getline(cin, input);
		if (input == "quit") break;

		cout << "| ";

		string output = mySolver.solveExp(input);
		cout << output << endl << endl;
	}
	return 0;
}