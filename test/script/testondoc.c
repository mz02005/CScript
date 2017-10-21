function B()
{
	function InB()
	{
		println("InB");

		function C()
		{
			println("C");
		}

		function B(){println("the bbbbb");}

		C();
		B();
	}

	function InB2(x)
	{
		println("InB2");
		for (; x>=0; x-=1)
			println("INB2---" + x.toString());
	}

	InB();
	InB2(10);
}

int x = 1;
B();

array fibCache = CreateArray();
int fi = 0;
// 插入100个数据
for (fi = 0; fi < 100; fi+=1)
{
	fibCache.add(0, 0);
}

function Fibonacci(c)
{
 if (!c)
  return c;
 if (c == 1)
  return c;
 if (c >= 100)
	 return -1;
 if (fibCache[c])
	 return fibCache[c];
 fibCache[c] = Fibonacci(c - 1) + Fibonacci(c - 2);
 return fibCache[c];
}

println(time());
println(Fibonacci(40).toString());
println(time());

// 用普通方法实现得到斐波那契数列
function GetFibonacci(c)
{
 int i;
 int x = 0, y = 1, r;
 for (i = 2; i <= c; i +=1)
 {
  r = x + y;
  x = y; y = r;
 }
 return r;
}

function test(gf)
{
 return gf(20);
}

println("1");
println(GetFibonacci(40).toString());
println("2");
println(test(GetFibonacci).toString());

function NoUse()
{
 int i = 0;
 switch(i)
 {
  default: println("Oh,Oh");
  case 1: println("1,1");
   break;
  case 2: println("2,2"); break;
 }
}

NoUse();

