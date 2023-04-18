#include <iostream>
#include <math.h>
#include <string>

using namespace std;

#ifndef VALUE_H
#define VALUE_H

struct Fraction {
	int up, down;
	Fraction() : up(0), down(1) {}
	Fraction(int u, int d) : up(u), down(d) {
		for (int k = 2; k <= min(abs(up), abs(down)); k++) {
			while (abs(up) % k == 0 && abs(down) % k == 0) { up /= k; down /= k; }
		}
	}
};

class Value {
public:
	Value();
	Value(Fraction fv);
	Value(double dv);
	Value(string str);

	bool getDecimal() const;
	Fraction getFracValue() const;
	double getDecValue() const;
	bool getCalculability() const;

	string printValue() const;

	Value& operator+=(const Value z);
	Value& operator-=(const Value z);
	Value& operator*=(const Value z);
	Value& operator/=(const Value z);
	Value& powv(const Value z);
	friend Value operator+(Value a, const Value b) { return a += b; }
	friend Value operator-(Value a, const Value b) { return a -= b; }
	friend Value operator*(Value a, const Value b) { return a *= b; }
	friend Value operator/(Value a, const Value b) { return a /= b; }
	friend Value operator-(Value a) { return a *= -1; }
	friend Value powv(Value a, Value b) { return a.powv(b); }
	friend ostream& operator<<(ostream& out, const Value& m) {
		return out << m.printValue();
	}
private:
	bool isDecimal;
	Fraction fracValue;
	double decValue;
	bool calculability;
};

#endif