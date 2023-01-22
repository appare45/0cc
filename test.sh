#!/bin/bash
all=0
fail=0

assert() {
  expected="$1"
  input="$2"

  ./0cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  ((++all))

  if [ "$actual" = "$expected" ]; then
    echo "✅ Success: $input => $actual"
  else
    echo "🚫 Fail: $input => $expected expected, but got $actual"
    ((++fail))
    # exit 1
  fi
}

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 41 " 12 + 34 - 5 "
assert 47 "5+6*7"
assert 15 "5*(9-6)"
assert 4 "(3+5)/2"
assert 10 "-10+20"
assert 10 '- -10'
assert 10 '- - +10'

assert 1 "0==0"
assert 0 "0==1"
assert 0 "0!=0"
assert 1 "10!=0"
assert 1 "10>0"
assert 0 "10>20"
assert 0 "10>10"
assert 0 "10<0"
assert 1 "10<20"
assert 0 "10<10"
assert 1 "10>=10"
assert 1 "20>=10"
assert 0 "10>=20"
assert 1 "10<=10"
assert 0 "20<=10"
assert 1 "10<=20"

echo "Failed: $fail / $all"