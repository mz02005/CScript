array toDisp = CreateArray(
	"Calucate pi."
	);
println(toDisp);

int count = 5600 * 2 + 1;
int c = count - 1;
int a=10000, b = 0, d, e = 0, g;

function for1Mid()
{
	d += f[b]*a;
	f[b] = d%(g-1);
	g-=1;
	d/=g;
	g-=1;
	b-=1;
	return b;
}

array f;
int i = 0;
for (i = 0; i < count; i += 1) f.add(-1, 0);
for(;b-c;){ f[b] = a/5; b+=1;}
for (;g=c*2;)
{
	for(b=c; for1Mid(); d*=b);
	c-=14;
	string data = (e+d/a).toString().substr(0, 4);
	data = "000" + data;
	print(data.substr(data.len - 4, 4));
	e=d%a;
	d=0;
}
