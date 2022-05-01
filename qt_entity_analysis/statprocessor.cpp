#include "statprocessor.h"


//constructor
statproc::statproc()
{}

double statproc::nordist(double x, double sigma, double mu)
{
	//x - current coord
	//sigma - stand deviation
	//mu - median point
	return (1 / sqrt(2 * 3.14*pow(sigma, 2))*exp(-pow((x - mu), 2) / (2 * pow(sigma, 2))));
}

void statproc::retreivenormaldist(vector <double> &ndh)
{
	ndh.clear();
	for (double val : nrm) ndh.push_back(val);
}
void statproc::retreivehistogram(vector <int> &hih)
{
	hih.clear();
	for (double val : hg) hih.push_back(val);
}

void statproc::genhistogramdata()
{
	vector<int>histogram = {};
	vector <double> sectresh = {};
	vector <double> sds = {}; //sprted dataset

	if (dataset.size() > 0)
	{
		//prepare dataset
		for (double samp : dataset) sds.push_back(samp);
		sort(sds.begin(), sds.end());
		
		//prepare histogram inputs
		int sectors = 0;
		if (sds.size() > 100)
		{
			sectors = 10;
		}
		else
		{
			sectors = 5;
		}
		double mean = (double)sectors / 2;
		double sigma = 1;

		//make sector limits
		double hw = sds[sds.size() - 1] - sds[0];
		double sc = hw / (sectors-1);
		double secstart = sds[0];

		for (int i = 0; i < sectors-1; i++)
		{
			sectresh.push_back(secstart + sc * i);
		}

		//fill sectors on histogram with 0
		for (int i = 0; i < sectors; i++)
		{
			histogram.push_back(0);
		}

		//fill histogram
		for (double val : sds)
		{
			int si=0;
			while (val > sectresh[si])
			{
				si++;
				if (si >= (histogram.size() - 1))
				{
					break;
				}
			}
			histogram[si]++;
		}

		hg.clear();
		for (int val : histogram)
		{
			hg.push_back(val);
		}

		//fill normal distribution
		nrm.clear();
		double ww = (double)sectors / 100;
		nrm.push_back(ww);
		for (int i = 0; i < 100; i++)
		{
			nrm.push_back(nordist(ww*i,sigma,mean));
		}

	}

}


void statproc::setdataset(vector <double> &exdataset)
{
	dataset.clear();
	for (int i = 0; i < exdataset.size(); i++)
	{
		dataset.push_back(exdataset[i]);
	}
}

double statproc::sqdev(double xavg)
{
	double s = 0;
	for (int i = 0; i < dataset.size(); i++)
	{
		s+=pow((dataset[i]-xavg),2);
	}
	return sqrt(s/(dataset.size()-1));
}


double statproc::getdatasetavg()
{
	double s = 0;
	for (int i = 0; i < dataset.size(); i++) s += dataset[i];
	return s / dataset.size();
}

double statproc::gettcoeff(double trust, int ssize)
{
	boost::math::students_t dist(ssize - 1);
	return boost::math::quantile(dist, trust);
}

std::string statproc::outputdataset()
{
	std::string k;
	k = "";
	for (int i = 0; i < dataset.size(); i++)
	{
		k += (std::to_string(dataset[i])+';');
	}
	return k;
}


void statproc::calcxa(double trust)
{
	double txa = 0; double tdelta = 0;

	double xavg = getdatasetavg();

	double sdev = sqdev(xavg);

	double sdevrnd = sdev / xavg;

	double sdevsmp = sdev / sqrt(dataset.size());

	double t = gettcoeff(trust, dataset.size());

	tdelta = t * sdevsmp;

	txa = xavg;

	setsdev_d(sdev);
	setsdevsmp_d(sdevsmp);
	sett(t);

	setdelta(tdelta);
	setxa(txa);
}