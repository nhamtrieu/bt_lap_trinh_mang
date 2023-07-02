#include <iostream>
#include <vector>
// #include <bits/stdc++.h>
using namespace std;

void multiply(vector<int> &result, int x)
{
    int carry = 0;
    for (int i = 0; i < result.size(); i++)
    {
        int product = result[i] * x + carry;
        result[i] = product % 10;
        carry = product / 10;
    }
    while (carry)
    {
        result.push_back(carry % 10);
        carry /= 10;
    }
}

int sumOfDigitsInFactorial(int N)
{
    vector<int> result(1, 1);
    for (int i = 2; i <= N; i++)
    {
        multiply(result, i);
    }
    int sum = 0;
    for (int i = 0; i < result.size(); i++)
    {
        sum += result[i];
    }
    return sum;
}

int main()
{
    int test;
    cin >> test;
    while (test--)
    {
        int N;
        cin >> N;
        int sum = sumOfDigitsInFactorial(N);
        cout << sum << endl;
    }
    return 0;
}