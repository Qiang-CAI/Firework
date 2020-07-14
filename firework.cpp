// firework.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include<cmath>
#include <ctime>
#include<easyx.h>
#include<list>
using namespace std;

/*		*定义全局变量*		*/
long ret_useless = 0;
const int GW = 640;
const int GH = 480;
const double PI = 3.1415926;
const double g = 9.8;
const int max_len = 80;                                //射线最大长度
const int max_H = GH - max_len;                       //射线能到达最大高度，上升方向为y轴负方向
const int max_n = 5;                                 //最多同时能有几条射线
const double max_v = sqrt(2 * g* max_H / 10.0);     //最大速度

/*		*定义射线类*		*/
class fireline
{
private:
	int px;
	int py;
	int len;
	double v;
	clock_t ct = 0;
public:
	fireline(int, double);
	void Draw() const;
	void Move();
	BOOL stoped() const { return v == 0; }                             //射线停止
	BOOL overline()const { return py < max_H*max_n / (max_n + 1); }   //下一个烟花升起条件
	int getx()const { return px; }
	int gety()const { return py; }
};
fireline::fireline(int x = rand() % max_H + 40, double vv = (rand() % 20 + 76.0) / 100.0*max_v) :px(x), py(max_H)
{
	v = vv;
	len = int(v / max_v * max_len);                   //v:max_v = len:max_len
}
void fireline::Draw() const
{
	for (int i = py; i < py + len; ++i)
	{
		float hsv_v = 0.8f*(len - (i - py)) / len + 0.2f;
		setfillcolor(HSVtoRGB(300.0, 1.0f, hsv_v));             //粉色：300度、100%、100%
		solidcircle(px, i, 1);
	}
}
void fireline::Move()
{
	if (v == 0)
	{
		return;
	}
	if (ct == 0)
	{
		ct = clock();
		Draw();
		return;
	}
	clock_t t = clock() - ct;
	ct = clock();
	double v_cur = v - g * t / 1000.0;
	if (v_cur > 0)
	{
		py += int(10 * (v_cur*v_cur - v * v) / 2 / g);
		v = v_cur;
	}
	else
	{
		py -= int(10 * v*v / 2 / g);
		v = 0;
	}
	len = int(v / max_v * max_len);
	Draw();
}

/*		*定义粒子群*		*/
class ParticleSwarm
{
	struct particle
	{
		int x;
		int y;
		int z = 0;
		double vy;
		particle(int xx, int yy, double vv) :x(xx), y(yy), vy(vv) {}
	};
private:
	double vx;
	double vz;
	float hsv_h;
	clock_t ct = 0;
	list<particle> vec;
public:
	ParticleSwarm(int, int, float);
	void Draw() const;
	void Move();
	BOOL finish() const { return vec.size() <= 1; }
};
ParticleSwarm::ParticleSwarm(int x, int y, float colorh = float(rand() % 256))
{
	hsv_h = colorh + rand() % 20;
	hsv_h = hsv_h > 255 ? hsv_h - 256 : hsv_h;
	double vm = max_v / 2 * (rand() % 5 + 15.0) / 20.0;
	double radian_xz = (rand() % 360) * PI / 180;
	double radian_yx = (rand() % 90) * PI / 180 + PI / 2;
	vx = vm * cos(radian_yx) * cos(radian_xz);
	vz = vm * cos(radian_yx) * sin(radian_xz);
	double vy = vm * sin(radian_yx);
	int len = rand() % 30 + 80;
	while (len)
	{
		int xx = x + int(10 * vx * len / 200.0);
		int zz = int(10 * vz * len / 200.0);
		double cvy = vy - g * len / 200.0;
		int yy = y + int(10 * (cvy*cvy - vy * vy) / 2 / g);
		vec.push_back(particle(xx, yy, cvy));
		--len;
	}
}
void ParticleSwarm::Draw() const 
{
	int n = 0;
	auto size = vec.size();
	for (auto&x : vec)
	{
		if (x.x >= 0 && x.x < GW && x.y >= 0 && x.y < GH)
		{
			float cv = 0.2f + 0.8f * (size - n) / size - x.z / 40 * 0.1f;
			auto color = HSVtoRGB(hsv_h, 1.0f, cv > 0 ? cv : 0);
			if (x.z < 0)
			{
				setfillcolor(color);
				solidcircle(x.x, x.y, abs(x.z) / 80 > 1 ? 2 : 1);
			}
			else
				putpixel(x.x, x.y, color);
		}
		++n;
	}
}
void ParticleSwarm::Move()
{
	if (ct == 0)
	{
		ct = clock();
		Draw();
		return;
	}
	for (int i = 0; i < 3 && vec.size()>1; ++i)
	{
		vec.pop_back();
	}
	clock_t t = clock() - ct;
	ct = clock();
	for (auto&x : vec)
	{
		double vy_cur = x.vy - g * t / 1000.0;
		x.x += int(10 * vx * t / 1000.0);
		x.y += int(10 * (vy_cur * vy_cur - x.vy * x.vy) / 2 / g);
		x.z += int(10 * vz * t / 1000.0);
		x.vy = vy_cur;
	}
	Draw();
}

/*		*定义烟花类*		*/
class Firework
{
private:
	list<ParticleSwarm> vec;
public:
	Firework(int x,int y);
	void Move();
	BOOL Empty() const { return vec.empty(); }
};
Firework::Firework(int x, int y)
{
	bool colorful = rand() % 100 < 20 ? true : false;
	float h = float(rand() % 256);
	int n = rand() % 5 + 80;
	for (int i = 0; i < n; i++)
	{
		if (colorful)
			vec.push_back(ParticleSwarm(x, y));
		else
			vec.push_back(ParticleSwarm(x, y, h));
	}
}
void Firework::Move()
{
	list<decltype(vec.begin())> todel;                    //decltype:查询表达式类型
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if (it->finish())
		{
			todel.push_back(it);
			continue;
		}
		it->Move();
	}
	for (auto&x : todel)
	{
		vec.erase(x);
	}
}

/*		*主程序*		*/
int main()
{
	initgraph(GW, GH);
	setrop2(R2_MERGEPEN);
	srand((unsigned)time(NULL));
	clock_t ct = clock();

	//射线list:
	list<fireline> Line;
	Line.push_back(fireline());

	//烟花list：
	list<Firework> Fire;

	BeginBatchDraw();
	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))       //Esc 退出
	{
		if (clock() - ct > 50)
		{
			cleardevice();
			ct = clock();

			//射线操作：
			list<decltype(Line.begin())> todel;
			if (Line.size() == 0)
			{
				Line.push_back(fireline());
			}
			else if (Line.size() < max_n && rand() % 100 < 10 && (--Line.end())->overline())
			{
				Line.push_back(fireline());
			}
			for (auto it = Line.begin(); it != Line.end(); ++it)
			{
				if (it->stoped())
				{
					Fire.push_back(Firework(it->getx(), it->gety()));
					todel.push_back(it);
					continue;
				}
				it->Move();
			}
			for (auto&it : todel)
			{
				Line.erase(it);
			}

			//烟花操作：
			list<decltype(Fire.begin())> todel_2;
			for (auto it = Fire.begin(); it != Fire.end(); ++it)
			{
				if (it->Empty())
				{
					todel_2.push_back(it);
					continue;
				}
				it->Move();
			}
			for (auto&it : todel_2)
			{
				Fire.erase(it);
			}
			FlushBatchDraw();
		}
		Sleep(2);
	}
	EndBatchDraw();
	closegraph();
	return 1;
}


