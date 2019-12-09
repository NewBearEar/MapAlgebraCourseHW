#pragma once
class DistanceTemplate  
{
private:
	int templateSize;
public:
	DistanceTemplate();
	~DistanceTemplate();
	virtual int GetSize() = 0;
	virtual float GetDist(int i) = 0;
	virtual int GetOffX(int i) = 0;
	virtual int GetOffY(int i) = 0;

};

