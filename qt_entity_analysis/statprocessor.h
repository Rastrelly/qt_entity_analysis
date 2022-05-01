#include <cmath>
#include <vector>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/students_t.hpp>

using namespace std;

struct datpoint { float x, y; };


class statproc
{
private:
	vector <double> dataset;
	vector <int> hg;
	vector <double> nrm;
	double xa, delta, t, sdevsmp_d, sdev_d;
	double nxmin, nxmax, nymin, nymax;
public:
	double getxa()    { return xa; };
	double getdelta() { return delta; };
	double gett() { return t; };
	double getsdevsmp_d() { return sdevsmp_d; };
	double getsdev_d() { return sdev_d; };
	
	void setxa(double val)    { xa = val; };
	void setdelta(double val) { delta = val; };
	void sett(double val) { t = val; };
	void setsdev_d(double val) { sdev_d = val; };
	void setsdevsmp_d(double val) { sdevsmp_d = val; };

	void retreivenormaldist(vector <double> &ndh);
	void retreivehistogram(vector <int> &hih);
	
	double nordist(double x, double sigma, double mu);
	void genhistogramdata();
	double getdatasetavg();
	double sqdev(double xavg);
	double gettcoeff(double trust, int ssize);

	std::string outputdataset();

	statproc();
	void setdataset(vector <double> &exdataset);
	void calcxa(double trust);
};