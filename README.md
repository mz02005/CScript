关于CScriptEng
==============

CScriptEng是一个适合嵌入到宿主程序中的脚本执行引擎，使用类似C89的语法（被改造得愈来愈像javascript了）。
它使得宿主程序具有执行脚本语言的能力，方便宿主程序发布后，由最终用户或者程序编写者扩充程序的功能，而无需重新编译宿主程序。

-------------------------------------------------------------------------

#语法和功能简介
##和C89的语法区别
CScript使用类似于C的语法，但是删减了作为嵌入脚本引擎不太会用到的功能。相比C89，下面这些功能是CScript所没有的：
* struct、enum、位域
* 指针声明、操作包括数组的相关操作
* 函数声明、定义和调用
* 编译预处理、编译指令
* 自增和自减运算符（++、--）
* 逻辑运算中每个部分都会被计算，例如表达式“1 && 0 && 3 && 4”中，所有式子都会被计算是否为0

##array类型
为了解决没有数组的缺陷，目前提供了array类型，可以如同声明整数（int）类型变量一样声明一个array类型变量。也可以用CreateArray函数来创建一个数组。像这样：<br>
```c++
array arr = CreateArray(0x80, \x11, "hello, world");
```
可以看出，数组元素不必是同一类型。也可以通过索引访问数组元素<br>
```c++
int x = arr[0];
```
可以为它添加元素<br>
```c++
arr.add(0, "hehe");
```
删除指定元素<br>
```c++
arr.del(1);
```
需要注意的是，array中储存的是对象的引用，而不是对象的复制体，使用
```c++
int v = 1;
arr.add(-1, v);
v = 2;
println(v[0]);
```
将显示2，如果希望插入的数据和源数据脱离关系，则对于上面的语句做如下的修改
```c++
arr.add(-1, v+0);
```

##可以在引用之前的任意位置声明变量，这同C++有点相似
如下这些代码都是可以编译通过的：<br>
```c++
int a = 0;
a += 1;
int b = a + 1;
```

##库函数
在脚本引擎中，提供了一些基本的函数供用户使用
* time函数返回了一个具有6个元素的数组array对象，依次表示调用time时的本地计算机‘年’、‘月’、‘日’、‘时’、‘分’、‘秒’数据。
这样可以用下面这样的代码来获得当前的年份
```c++
int year = time()[0];
```
* srand和rand也保留了C运行库中同名函数的意义。其中srand也可以没有参数，这种情况下就相当于在标准C中用srand((unsigned)time(NULL));。而rand返回的是[0, 1]之间的浮点数
* println和print用来向终端控制台输出文本，println带换行标志，print没有。目前没有实现格式化文本的操作。
* sin函数的意义和C运行库中的一样，可以
```c++
sin(3.1415926 / 2);
```
* sleep函数用来让虚拟机线程睡眠一会儿，参数是睡眠的时间长度，单位是ms。

##函数对象
###函数对象的一般用法
* 由于在某些嵌入场景下，需要用到回调等操作，比如注册一个事件，然后让宿主每隔一定时间调用一下
某些语句，这种需求在不使用回调函数的情景下，实现不直观——用户可能需要实例化多个虚拟机环境，
且相互还有共享的数据。
* 在嵌入脚本中有反复用到的代码段需要抽象。
基于上述原因，这里提供function关键字用于声明函数对象，语法是：<br>
```c++
function 函数名(参数列表)
{
 函数体
}
```
例如下面的程序<br>
```c++
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
println(test(GetFibonacci).toString());
```
上面的GetFibonacci函数用于计算斐波那契数列的第c项，而test函数就通过函数名来传入函数对象实现函数的间接调用。函数对象支持递归，比如上面计算斐波那契数列的函数也可以用如下的递归方式实现：<br>
```C++
function Fibonacci(c)
{
  if (c <= 1)
    return c;
  return Fibonacci(c - 1) + Fibonacci(c - 2);
}
```
然后就可以<br>
```C++
println(Fibonacci(20));
```
上面实现的递归计算斐波那契数列的算法非常耗费CPU，且当计算的值较大时可能会引起栈空间不足，届时就需要修改缺省的虚拟机配置，提高这个缺省的栈空间。<br>
结合array类型可以加快这个递归的速度<br>
```c++
array fibCache = CreateArray();
int fi = 0;
// 插入100个数据（我竟然没有做一个批量初始化，或者设置数组大小的函数）
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
 // 通过记录已经算好的数据，使得递归的速度大大加快
 if (fibCache[c])
	 return fibCache[c];
 fibCache[c] = Fibonacci(c - 1) + Fibonacci(c - 2);
 return fibCache[c];
}
println(time());
println(Fibonacci(40).toString());
println(time());
```
###函数对象的注意点
这一节其实应该是对象的访问作用域，放在函数对象这一章节是因为函数对象是对象的容器，有它在会对对象产生分级作用。
函数对象的访问作用域如下：<br>
* 顶级函数，包括库函数在任意位置都可以访问
* 除了访问顶级函数（对象），其它对象都只能被同级对象访问
* 如果想访问上级或者间隔级别下的对象，只能通过将对象放到函数对象（分割器）的参数列表中作为参数传入
这意味着除了顶级函数，其它级别的函数都不支持递归调用<br>
例如：
```c++
function topLevel()
{
	function Inner()
	{
	}
}
// 访问顶级（且是同级）对象
topLevel();
// 这个访问是不允许的
Inner();

// 但是可以这样写
function topLevel()
{
	function Inner()
	{
	}
	Inner();
}
// 在调用topLevel的同时，由topLevel调用了Inner函数
topLevel();
```
在来看看传入上级对象到下级的方法（就是将对象通过函数参数传入）
```c++
function topLevel(of)
{
	function Inner(outerFunc)
	{
		outerFunc();
		println("Inner");
	}
	Inner(of);
	println("topLevel");
}
function haha()
{
	println("aaa");
}
topLevel(haha);
```

#嵌入功能
脚本引擎最主要的功能就是嵌入到宿主程序中运行，宿主程序可以提供更多的功能给脚本编写者使用，同时和脚本执行环境进行适当的交互。<br>
** 需要的头文件和库
所需头文件为CScriptEng.h<br>
在linux上编译需要libnotstd.so、libCScriptEng.so两个库，另外还需要libzlib.so、libxml2.so、libiconv.so<br>
在windows上用vc编译需要链接notstd_$(Configuration)_$(Platform).lib、CScriptEng_$(Configuration)_$(Platform).lib两个库<br>
** 初始化和清理
用语句
```c++
scriptAPI::SimpleCScriptEng::Init();
scriptAPI::SimpleCScriptEng::Term();
```
两个函数来执行初始化和清理操作，这两个函数在分别在进程的开始和结束各调用一次即可<br>
** 执行脚本
*** 首先准备源代码，源代码通过FileStream流来获得
```c++
std::string fName = filePathName;
scriptAPI::FileStream fs(fName.c_str());
```
*** 然后是编译，成功会返回非空的句柄
```c++
HANDLE h = compiler.Compile(&fs, true);
```
*** 这就可以执行脚本了，下面的代码首先创建了一个运行环境上下文，然后调用它的Execute成员来执行编译后的代码
```c++
scriptAPI::ScriptRuntimeContext *runtimeContext
	= scriptAPI::ScriptRuntimeContext::CreateScriptRuntimeContext(1024, 512);
int exitCode = 0;
int er = runtimeContext->Execute(h, &exitCode);
```
以上er返回值如果是runtime::EC_Normal表示执行成功，否则表示执行失败。若执行成功且传入了exitCode，则exitCode返回顶级函数返回值的整数形式<br>
*** 最后需要做清理工作，释放编译结果和执行环境占用的资源
```c++
scriptAPI::ScriptCompiler::ReleaseCompileResult(h);
scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(runtimeContext);
```
** 扩展脚本的功能
** 使宿主程序可以回调脚本中的函数
** 引用计数陷阱
本章待完善<br>

======================================================================================================

#需要实现的功能和需修改的bug
- [ ] 编译时的错误没有给出错误位置
- [x] 函数对象的调用，如果出现了错误，则返回后还会继续上层代码的运行
- [ ] 某些操作需要优化，比如对于+=之类的操作符生成的代码，可以通过增加新的指令来优化目前生成的代码；只处理了整常数表达式，而没有处理浮点数常数表达式。
- [ ] 方便调试脚本代码

======================================================================================================