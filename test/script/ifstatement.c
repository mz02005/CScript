int a;
if (!a)
	println("OK");

if (a == 0)
	println("OK");

if (a != 0)
	return -1;

if (a > -1)
	println("OK");

if (a >= -1)
	println("OK");

if (a <= 1)
	println("OK");

if (a < 1)
	println("OK");

if (1);

if (0)
	debugbreak;

if (1) {
	if (1) {
		if (1)
			println("OK");
	}
}

if (1) if (1) if (1) println("OK");

if (0) if (1) if (1) debugbreak;

{
	int a = 100;
	if (a != 100)
		println("Error");
}
if (a == 100)
	println("Error");
println(a);

if (1 + 1 == 2)
	println("OK");
else
	return -2;

a = 1;
if (a * 2 == 2)
	println("OK");
else
	return -3;

a = 1;
if (a += 1 == 2)
	println("OK");
else
	return -4;

println("ifstatement.c finished\n\n");