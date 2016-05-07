int i;
for (;i<100;i+=1);

int cycle = 100000;
for (i=1;;i+=1)
{
	if (i >= 1000000)
		break;
	float v = i;
	v /= cycle;
	if (v == i / cycle)
		println(i);
}

println("forstatement finished\n\n");