#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 10 "-10+20;"
assert 10 "- -10;"
assert 10 "- (-10);"
assert 1 "10 == 10;"
assert 1 "10 < (5+24);"
assert 24 "(10 <= 5) + 24;"
assert 1 "10 >= 10;"
assert 1 "5 != 10;"
assert 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"
assert 18 "x = 10; y = x - 2; x + y;"
assert 1 "i=j=1;i*j;" 
assert 1 "k = 15; l = k / 2; k > l * 2;"
assert 6 "foo = 1; bar = 2 + 3; foo + bar;"
assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
assert 5 "return 5; return 8;"
assert 14 "return_1 = 3; return_2 = 5 * 6 - 8; return return_1 + return_2 / 2;"

echo OK

