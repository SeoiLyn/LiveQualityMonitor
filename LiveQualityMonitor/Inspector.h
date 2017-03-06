#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include <string>
#include "inttypes.h"
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

extern float rescale_vector[36][2];

class Inspector
{
public:
	Inspector();
	~Inspector();

public:
	static int read_range_file(string prefix);
	static std::string prefix;

public:
	bool init(std::string recordFile);
	/* frameData is RGB24 image*/
	void computeRGBScore(uint8_t * frameData, int width, int height, int64_t pts);
	void unInit();

private:
	int64_t last_pts;
	vector<float> diff_vec;
	int index;

	ofstream m_record;
};

#endif /* __INSPECTOR_H__ */
