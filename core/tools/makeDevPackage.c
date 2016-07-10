/*
	copyright mz02005@qq.com
	本脚本文件用于在编译完成后，将第三方开发所需的头文件、库文件等复制到特定的目录，方便发布
*/

// 第一个参数是$(SolutionDir)
// 第二个参数是体系结构
// 第三个参数是debug或者release
if (ARGV.size <= 2)
{
	println("Invalid parameter, solution dir must be identified");
	return 1;
}

string rootPath = ARGV[0];
if (rootPath.substr(rootPath.len - 1) != "\\")
	rootPath += "\\";

string devReleaseRoot = rootPath + "devRelease\\";

string arch = ARGV[1];
string conf = ARGV[2];

function mkdirHelper(dir)
{
	println("mkdir " + dir);
	system("mkdir " + dir);
	return 0;
}

function forEachInArray(arr, f)
{
	int i;
	int s = arr.size;
	for (i = 0; i < s; i += 1)
		f(i, arr[i]);
}

// 创建目录
array dirToCreate = CreateArray(
	devReleaseRoot,
	devReleaseRoot + "include",
	devReleaseRoot + "include\\notstd",

	devReleaseRoot + "lib",
	devReleaseRoot + "bin",

	devReleaseRoot + "lib\\" + arch,
	devReleaseRoot + "lib\\" + arch + "\\" + conf,

	devReleaseRoot + "bin\\" + arch,
	devReleaseRoot + "bin\\" + arch + "\\" + conf,
	);

forEachInArray(dirToCreate, function(i, e)
{
	mkdirHelper(e);
});

// 待复制的头文件
array incToCopy = CreateArray(
	"CScriptEng.h",
	"rtTypes.h",
	"objType.h",
	"arrayType.h",
	"cscriptBase.h",
	"config.h"
	);

function CopyFile(src, dest)
{
	println("copy " + src + " to " + dest);
	system("copy " + src + " " + dest);
}

forEachInArray(incToCopy, function(i, e)
{
	string destDir = devReleaseRoot + "include\\";
	string src = rootPath + "core\\CScriptEng\\" + e;
	CopyFile(src, destDir);
});

// notstd有一个特殊的文件要复制
CopyFile(rootPath + "core\\notstd\\config.h", devReleaseRoot + "include\\notstd");

string targetDir = rootPath + arch + "\\" + conf + "\\";

// 复制库文件
CopyFile(targetDir + "*.lib", devReleaseRoot + "lib\\" + arch + "\\" + conf);

// 复制动态库
CopyFile(targetDir + "*.dll", devReleaseRoot + "bin\\" + arch + "\\" + conf);

//zipFile(rootPath + "devRelease.zip", devReleaseRoot, CreateArray(1));

// 创建windows的shell菜单，用于方便地执行脚本文件
array shellMenuToWrite = 
	CreateArray(
		"Windows Registry Editor Version 5.00\r\n",
		"[HKEY_CLASSES_ROOT\\*\\shell\\用cscripteng打开\\command]",
		"@=\"" + rootPath + "win32\\debug\\testCScript.exe \\\"%1\\\"\"" + "\r\n");

string regFilePath = rootPath + "temp.reg";
DeleteFile(regFilePath);
object rf = csOpenFile(regFilePath, "wb");
if (rf != null)
{
	forEachInArray(shellMenuToWrite, function(i, e)
	{
		rf.WriteFile(e + "\r\n");
	});
	rf.Close();
	system("regedit " + regFilePath);
}
//DeleteFile(regFilePath);
