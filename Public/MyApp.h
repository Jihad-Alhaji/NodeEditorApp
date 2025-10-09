#pragma once
#include<ArcApp/ArcApp.h>
class MyApp :public ARC::App::ArcApp
{
	std::shared_ptr<class UIManager> mUIRoot;
public:
	bool Init()override;
	void Draw(double dt)override;
	void Tick(double dt)override;
};

