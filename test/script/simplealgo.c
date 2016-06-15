println("Show some simple algorithm");

function forEach(arr,DoWithElem)
{
	int s = arr.size;
	int i;
	for(i=0;i<s;i+=1)
	{
		DoWithElem(arr[i]);
	}
}

int x = 0;

function haha(obj)
{
	println(obj);
}
srand();
forEach(
	CreateArray(1, '1', "abc").add(-1,'9').add(-1,rand()), haha);

function strfind(s, m, b)
{
	int l = s.len;
	int lm = m.len;
	int i;
	if (l - b < lm)
		return -1;
	for(i=b;i<l;i+=1)
	{
		if (s.substr(i, lm) == m)
		{
			if (l - i >= lm)
				return i;
		}
	}
	return -1;
}

function strrfind(s, m)
{
	int l = s.len;
	int lm = m.len;

	if (l < lm)
		return -1;

	int i;
	for (i = l - lm; i >= 0; i-=1)
	{
		if (s.substr(i, lm) == m)
			return i;
	}
	return -1;
}

function strleft(s, c)
{
	if (c < 0) return "";
	if (s.len >= c)
		return s.substr(0, c);
	return s;
}

function strright(s, c)
{
	if (c < 0) return "";
	if (s.len >= c)
		return s.substr(s.len - c, c);
	return s;
}

// 将字符串解析为整数，如果失败则返回空字符串
function parseToInt(s)
{
	int r = 0;
	int i, l = s.len;
	for (i=0; i < l; i+=1)
	{
		int x = s[i];
		if (x >= 48 && x <= 57)
		{
			r *= 10;
			r += x - 48;
		}
		else
			return "";
	}
	return r;
}

string is = "19999";
if (parseToInt(is).toString() != is)
	return -1;

string a = "hello,world";
if (strfind(a, ",w", 0) != 5)
	return -2;

if (strleft(a, 10) != "hello,worl")
	return -3;
if (strright(a, 3) != "rld")
	return -4;

if (strfind(a, "l", 0) != 2)
	return -5;
if (strrfind(a, "l") != 9)
	return -6;

println("Test simple algorithm end.\n");