#include "stdafx.h"
#include "CScriptEng/CScriptEng.h"
#include "CScriptEng/vm.h"

#if _MSC_VER > 1600
// 最终要能编译通过这段代码
static const char codeSection[] = R"(
float temp = Y1;
if (temp > 500)
{
 temp = 0;
}
else
{
 Y2 = floor(temp / 10);
 temp += 0.8;
}
Y1 = temp;
for(temp = 500; temp > 0; temp-=10)
{
a += b * 0.8f + 6.f;
printf("Hello, world!\n");
/*
This is a test.
*/
}
)";
#endif

static const char *codes[] =
{
	"float + x;\r\n",
	"x = -100+(-9.7+(-bloc*100));\r\n",
	"Y[1] = IN[0] + 1;\r\n",
	"bloco=&blocos[rand()%7*4];\r\n",
	"sombra[263]=A=getchar();\r\n",
	"printf(\"\\033[%d; %dH\",(I=i)/12,i%12*2+28);\r\n",
	"pd = *(&p+1);\r\n",
	"*(a+b) = 100;\r\n",
	"dd=(ad-89.9)*(dd+89)-10086+floor(3.4);",
	"title = 0.f + 8.9F * (fun(8.3f*0.f)\r\n",
	"testc = \"\\thello, world!\\x67\", \'c\';\r\n",
	"a = 4.45+49.34*9;\r\n*28",
	"a+b-2;\r\n",
	"!a+b\r\n",
	"p=dd[a+b]*34\r\n",
	"a= e+b*(!c-b) / (16+23*(b-fun(a,b[y+16*test])));\r\n",
	"result = !a + 3.1415926 * pow(r, 2.f);",
};

int _tmain(int argc, _TCHAR* argv[])
{
	scriptAPI::SimpleCScriptEng::Init();
	scriptAPI::FileStream fs("c:\\work\\test.c");
	scriptAPI::ScriptCompiler compiler;

	HANDLE h = compiler.Compile(&fs, true);
	if (h)
	{
		scriptAPI::ScriptRuntimeContext *runtimeContext
			= scriptAPI::ScriptRuntimeContext::CreateScriptRuntimeContext(512, 512);
		runtimeContext->Execute(h);
		scriptAPI::ScriptCompiler::ReleaseCompileResult(h);
		scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(runtimeContext);
	}
	scriptAPI::SimpleCScriptEng::Term();
	return 0;
}
