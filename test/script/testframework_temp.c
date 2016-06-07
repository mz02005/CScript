function TestMain()
{
	function TestDecl()
	{
		return "true";
	}

	function TestFor()
	{
		int i;
		for (; ;i+=1)
		{
			if (i == 1000000)
				break;

			if (i % 10000 == 0)
			{
				string x = i.toString();
				debugbreak;
				if (x == "0" || substr(x, x.len - 4) == "0000")
				{
				}
				else
					return "false";
			}
		}
		return "true";
	}

	array toTest = CreateArray(
		TestDecl,
		TestFor);

	int i = 0; int l = toTest.size;
	for (; i < l; i+=1)
	{
		if (toTest[i]() != "true")
			println("Test " + toTest[i].toString() + " fail.");
		else
			println("Test " + toTest[i].toString() + " success.");
	}
}

TestMain();
