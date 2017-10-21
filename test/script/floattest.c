function TopLevel()
{
	println("I am topLevel");
}

function Show()
{
	println("111");
	function PortialConvert(projMin, projMax, origMin, origMax)
	{
		TopLevel();
		if (origMax == origMin)
			return CreateArray(-1, -1);
		float k = (projMax - projMin) / (origMax - origMin);
		float b = projMax - k * origMax;
		return CreateArray(k, b);
	}
	println("222");
	function GetProjectValue(orig,portialData)
	{
		return portialData[0] * 1.f * orig + portialData[1];
	}

	TopLevel();

	println("333");
	array a = PortialConvert(0, 25.f, 0, 50);

	println("444");
	println(GetProjectValue(39, a).toString());
	println("555");
}

Show();