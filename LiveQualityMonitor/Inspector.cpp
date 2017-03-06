#include "Inspector.h"
#include <stdio.h>
#include "brisque_revised/brisque.h"
#include <math.h>
#include <iomanip>

float rescale_vector[36][2];
std::string Inspector::prefix = "/home/water/projects/LiveQualityMonitor/brisque_revised/";

Inspector::Inspector()
: last_pts(0)
, index(0)
{

}

Inspector::~Inspector()
{

}

int Inspector::read_range_file(string prefix) {
	//check if file exists
	char buff[100];
	int i;
	string range_fname = prefix + "allrange";
	FILE* range_file = fopen(range_fname.c_str(), "r");
	if (range_file == NULL) return 1;
	//assume standard file format for this program	
	fgets(buff, 100, range_file);
	fgets(buff, 100, range_file);
	//now we can fill the array
	for (i = 0; i < 36; ++i) {
		float a, b, c;
		fscanf(range_file, "%f %f %f", &a, &b, &c);
		rescale_vector[i][0] = b;
		rescale_vector[i][1] = c;
	}
	return 0;
}

long get_current_time_with_ms(void)
{
	long            ms; // Milliseconds
	time_t          s;  // Seconds
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);

	s = spec.tv_sec;
	ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

	printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",
		(intmax_t)s, ms);

	return ms;
}

/* calculate Coefficient of Variation */
int calMean(vector<float>& x, float &cv)
{
	int i, n;
	float mean, sd, var, dev, sum = 0.0,
		sdev = 0.0;

	n = x.size();
	for (i = 0; i < n; ++i) {
		sum = sum + x[i];
	}

	mean = sum / n;

	for (i = 0; i < n; ++i) {
		dev = (x[i] - mean)*(x[i] - mean);
		sdev = sdev + dev;
	}

	var = sdev / n;
	sd = sqrt(var);
	cv = (sd / mean) * 100;

	cout << "Variance: ";
	cout << setprecision(5) << var << endl;
	cout << "Standard Deviation: ";
	cout << setprecision(5) << sd << endl;
	cout << "Coefficient of Variation: ";
	cout << setprecision(5) << cv << "\%" << endl;

	return 0;
}

bool Inspector::init(std::string recordFile)
{
	last_pts = 0;
	index = 0;

	m_record.open(recordFile.c_str());
	m_record << "timestamp, width, height, quality, variation" << std::endl;
}

void Inspector::unInit()
{
	m_record.close();
}

//回调RGB frame data,并计算画质分数
void Inspector::computeRGBScore(uint8_t * frameData, int width, int height, int64_t pts)
{
	std::cout << "frame width " << width << ", height " << height << ", pts " << pts << std::endl;

	if (last_pts == 0)
	{
		last_pts = pts;
	} else {
		int diff = pts - last_pts;
		last_pts = pts;
		diff_vec.push_back(diff);
	}

	if (index % 25 == 0)
	{
		long ms = get_current_time_with_ms();
		float qualityscore = computescore(Inspector::prefix, frameData, width, height);
		long ms2 = get_current_time_with_ms();

		//calculate Coefficient of Variation
		float cv;
		calMean(diff_vec, cv);
		std::cout << "score " << qualityscore << ", Coefficient of Variation " << cv << ", calculate time " << (ms2 - ms) << std::endl;
		diff_vec.clear();

		m_record << pts << ", " << width << ", " << height << ", " << qualityscore << "," << cv << std::endl;
	}

	index++;
}