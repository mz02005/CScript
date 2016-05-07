int s = 0;
switch (s)
{
default:
	println("Error");
	debugbreak;
	break;

case 0:
	println("0");

case 1:
	println("1");

case 2:
	println("2");
	break;
}

switch (s)
{
default:
	debugbreak;

case 0:
	println("0");
	break;

case 1:
case 2:
	debugbreak;
}

switch (s)
{
case 0:
	println("0");

default:
	println("default");
	break;
}

switch (s)
{
case 0:
	println("0");
	break;

default:
	debugbreak;
}

println("switchstatement finished\n\n");