/*
 * @Author:   
 * @Date: 2023-04-24 11:36:37
 * @LastEditTime: 2023-05-16 15:19:55
 * @FilePath: /源码/Banker.cpp
 * @Description: 银行家算法
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>

using namespace std;

class Banker
{
public:
	~Banker();							  // 析构
	void InitBanker(const string &fname); // 初始化各项数据
	int SetRequest(int i);				  // 设置请求向量
	bool SafeCheck();					  // 银行家算法检查安全性
	bool RequestSafe(int i);			  // 检查进程Pi请求是否安全
	void PrintForm();					  // 打印当前状态表格
	void PrintList();					  // 打印安全序列

private:
	int m;				  // 资源种类
	int n;				  // 进程数
	int **Max;			  // 最大需求矩阵
	int **Need;			  // 需求矩阵
	int *Request;		  // 请求向量
	int *Available;		  // 可利用资源向量
	int **Allocation;	  // 已分配矩阵
	vector<int> SafeList; // 安全序列

	int RequestCheck(int i); // 检查进程资源请求是否合法(合法≠安全)
};

Banker::~Banker()
{
	delete[] Available;
	delete[] Request;
	for (int i = 0; i < n; i++)
	{
		delete[] Max[i];
		delete[] Allocation[i];
		delete[] Need[i];
	}
	delete[] Max;
	delete[] Allocation;
	delete[] Need;
	m = -1;
	n = -1;
	SafeList.clear();
}
// 初始化各项数据
void Banker::InitBanker(const string &fname)
{
	string line;
	ifstream ifs;
	stringstream ss;

	cout << "\nCopyright (c) 2023 by FWM , All Rights Reserved.\n"
		 << endl;

	// 打开文件
	ifs.open(fname);
	if (!ifs.is_open())
	{
		cout << "文件打开失败！" << endl;
		return;
	}
	cout << "读取文件内容如下：\n";

	// 读取文件
	// 资源种类数
	getline(ifs, line);
	m = stoi(line);
	Available = new int[m];
	vector<int> temp(m, 0);

	// 读取进程数
	getline(ifs, line);
	n = stoi(line);
	// 初始化动态数组
	Request = new int[m];
	Max = new int *[n];
	Allocation = new int *[n];
	Need = new int *[n];
	for (int i = 0; i < n; i++)
	{
		Max[i] = new int[m];
		Allocation[i] = new int[m];
		Need[i] = new int[m];
	}

	// 读取各进程最大需求
	cout << "==================" << endl;
	cout << "最大需求Max：\n";
	for (int i = 0; i < n; i++)
	{
		getline(ifs, line);
		ss.str(line);
		cout << (char)('a' + i) << "：";
		for (int j = 0; j < m; j++)
		{

			ss >> Max[i][j];
			cout << Max[i][j] << " ";
		}
		ss.clear();
		cout << endl;
	}

	// 读取各进程已分配资源数
	cout << "==================" << endl;
	cout << "已分配资源数Allocation：\n";
	for (int i = 0; i < n; i++)
	{
		getline(ifs, line);
		ss.str(line);
		cout << (char)('a' + i) << "：";
		for (int j = 0; j < m; j++)
		{
			ss >> Allocation[i][j];
			Need[i][j] = Max[i][j] - Allocation[i][j];
			temp[j] += Allocation[i][j];
			cout << Allocation[i][j] << " ";
		}
		ss.clear();
		cout << endl;
	}

	// 读取各类资源总量
	cout << "==================" << endl;
	cout << "各类资源总量：\n";
	getline(ifs, line);
	ss.str(line);
	for (int i = 0; i < m; i++)
	{
		cout << 'r' + to_string(i + 1) << "：";
		ss >> Available[i];
		cout << Available[i] << " ";
		Available[i] -= temp[i];
	}
	cout << endl;
	ss.clear();

	// 关闭文件
	ifs.close();
}
// 银行家算法安全检测
bool Banker::SafeCheck()
{
	bool flag = true;			// 标志，是否满足step2条件
	int *Work = new int[m];		// 工作向量（就是个temp）
	bool *Finish = new bool[n]; // 标志进程是否已经运行
	for (int i = 0; i < n; i++)
		Finish[i] = false;
	SafeList.clear();

	// step0：Work = Available
	for (int i = 0; i < m; i++)
		Work[i] = Available[i];
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			flag = true;
			// step1：检查进程Pj是否已经运行
			if (Finish[j])
				continue;
			// step2：检查进程Pj是否Need<=Available
			for (int k = 0; k < m; k++)
			{
				if (Need[j][k] <= Work[k])
				{
					continue;
				}
				else
				{
					flag = false;
					break;
				}
			}
			if (!flag)
				continue;
			// step3：分配资源，运行Pj，回收资源
			for (int k = 0; k < m; k++)
				Work[k] = Work[k] + Allocation[j][k];
			Finish[j] = true;
			// 将Pj添加进安全序列
			SafeList.push_back(j);
			// 回到step1
			break;
		}
	}
	// step4：检查所有进程的Finish[i]=true，是则安全，反之寄
	for (int i = 0; i < n; i++)
	{
		if (Finish[i])
			continue;
		else
		{
			delete[] Work;
			delete[] Finish;
			SafeList.clear();
			return false;
		}
	}
	delete[] Work;
	delete[] Finish;
	return true;
}
// 进程Pi发出请求
int Banker::SetRequest(int i)
{
	cout << "请输入请求资源量：";
	for (int i = 0; i < m; i++)
		cin >> Request[i];
	cout << endl;
	// 检查Request i是否合法
	return RequestCheck(i);
}
// 检查对进程Pi的请求是否合法
int Banker::RequestCheck(int i)
{
	for (int j = 0; j < m; j++)
	{
		// step1：检查Request<=Need
		if (!(Request[j] <= Need[i][j]))
			return -1;
		// step2：检查Request<=Available
		else if (!(Request[j] <= Available[j]))
			return -2;
	}
	return 0;
}
// 检查对进程Pi的请求是否安全
bool Banker::RequestSafe(int i)
{
	// 试分配
	for (int j = 0; j < m; j++)
	{
		Allocation[i][j] += Request[j];
		Need[i][j] -= Request[j];
		Available[j] -= Request[j];
	}
	PrintForm();
	// 安全性检测
	if (SafeCheck())
		return true;
	else
	{
		// 不安全，还原
		for (int j = 0; j < m; j++)
		{
			Allocation[i][j] -= Request[j];
			Need[i][j] += Request[j];
			Available[j] += Request[j];
		}
		return false;
	}
}
// 打印状态表
void Banker::PrintForm()
{
	cout << "Available = { ";
	for (int i = 0; i < m; i++)
	{
		cout << Available[i];
		if (i != m - 1)
			cout << ", ";
	}
	cout << " }\n"
		 << endl;
	cout << "Pi"
		 << " | "
		 << "Max \t|"
		 << " Allo\t\t|"
		 << " Need\t\t|" << endl;
	for (int i = 0; i < n; i++)
	{
		cout << (char)('a' + i) << "  | ";
		for (int j = 0; j < m; j++)
			cout << Max[i][j] << "  ";
		cout << "\t| ";
		for (int j = 0; j < m; j++)
			cout << Allocation[i][j] << "  ";
		cout << "\t| ";
		for (int j = 0; j < m; j++)
			cout << Need[i][j] << "  ";
		cout << "\t| ";
		cout << endl;
	}
}
// 打印安全序列
void Banker::PrintList()
{
	cout << "安全序列为：";
	cout << "< ";
	for (int i = 0; i < n; i++)
	{
		if (!SafeList.empty())
			cout << (char)('a' + SafeList[i]);
		else
			return;
		if (i != n - 1)
			cout << ", ";
	}
	cout << " >" << endl;
}

int main(int argc, char *argv[])
{
	// 读取参数
	if (argc != 2)
	{
		cout << "ERROR：参数输入错误！请检查后重试。" << endl;
		exit(-1);
	}
	string fname = argv[1];
	cout << "输入参数为：" << fname << endl;

	char val = 'y';

	// 初始化
	Banker B;
	B.InitBanker(fname);
	cout << "\n==================" << endl;
	cout << "T0时刻：\n";
	B.PrintForm();
	if (B.SafeCheck())
	{
		cout << "安全\n";
		B.PrintList();
	}
	else
	{
		cout << "不安全！\n";
		exit(-1);
	}
	while (val == 'y')
	{
		char c = '?';
		cout << "\n==================" << endl;
		cout << "请输入申请资源的进程号：";
		cin >> c;
		switch (B.SetRequest(c - 'a'))
		{
		case -1:
			cout << "进程请求不合法，请求大于需求！" << endl;
			break;
		case -2:
			cout << "进程请求不合法，请求大于系统拥有！" << endl;
			break;
		case 0:
			cout << "\n==================" << endl;
			cout << "进程请求合法" << endl;
			if (B.RequestSafe(c - 'a'))
			{
				cout << "请求安全！" << endl;
				B.PrintList();
			}
			else
			{
				cout << "请求不安全！" << endl;
				cout << "已恢复安全状态！\n\n";
			}
			break;
		}
		cout << "\n==================" << endl;
		cout << "是否继续(y/n)：";
		cin >> val;
	}
	system("pause");
	return 0;
}
