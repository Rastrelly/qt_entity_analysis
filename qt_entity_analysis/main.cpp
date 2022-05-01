#include "stdafx.h"

//mutex
mutex m;

//info structures
struct vmeasurement
{
	QString name;
	double x, y, z, ap, ar, ay, sxa, sdelta, nominal; //coords
	int style;
	vector <double> samples; //list of samples
	vector <double> nrm; //normal dist
	vector <int> hist; //histogram
};

//global variables

int selmeas = 0; int selsample = 0;
int cww = 0, cwh = 0;
double ang = 0;
double trust = 0.95;
bool needrefresh = false;
int drawmode = 0; //0 - scheme, 1 - histogram
vector <vmeasurement> d_collection = {};
vmeasurement cmeas;
double camang = 45;
double camr = 45;

//gloabl UI declarations
QPlainTextEdit * pteLog;
QListWidget * lMeas;
QListWidget * lSamp;
QLineEdit * edMeasName;
QLineEdit * edValue;
QLineEdit * ebdValueAvg;
QLineEdit * edValueWeig;
QLineEdit * edNominalValue;
QLineEdit * measPosX;
QLineEdit * measPosY;
QLineEdit * measPosZ;
QLineEdit * measRotP;
QLineEdit * measRotR;
QLineEdit * measRotY;
QLineEdit * edCamPos;
QLineEdit * edCamAng;

//forward declarations
void cbDisplay();
void printtolog(QPlainTextEdit *pp, QString txt);
void addmeasurement(vmeasurement vm);
void removemeasurement(int id);
void addsample(int id, double sm);
void removesample(int mid, int id);
void refreshqtui();
QString dbltoqs(double val);
QString inttoqs(int val);


//UI FUNCS
void lMeasClick()
{
	selmeas = lMeas->currentRow();
	if (d_collection.size() > 0)
	{
		if ((selmeas >= 0) && (selmeas < d_collection.size()))
			cmeas = d_collection[selmeas];
	}
	refreshqtui();
	needrefresh = true;
}

void lSampClick()
{
	selsample = lSamp->currentRow();
	refreshqtui();
}

void bAddMeasClick()
{
	/*
	struct vmeasurement {
	QString name;
	double x, y, z, ap, ar, ay, sxa, sdelta, nominal; //coords
	int style;
	vector <double> samples; //list of samples
	vector <double> nrm; //normal dist
	vector <int> hist; //histogram };  */

	addmeasurement({ 
		edMeasName->text(),
		measPosX->text().toDouble(),
		measPosY->text().toDouble(),
		measPosZ->text().toDouble(),
		measRotP->text().toDouble(),
		measRotR->text().toDouble(),
		measRotY->text().toDouble(),
		0,
		0,
		edNominalValue->text().toDouble(),
		{},{},{} 
		});
}

void bRemoveMeasClick()
{
	int mid = lMeas->currentRow();
	removemeasurement(mid);
}

void bAddSampleClick()
{
	double cval = edValue->text().toDouble();
	addsample(selmeas,cval);
}

void bRemSampleClick()
{
	removesample(selmeas,selsample);
}


void bProcessClick()
{
	if (d_collection.size() > 0)
	{
		m.lock();
		statproc *sp = new statproc();
		for (int i = 0; i < d_collection.size(); i++)
		{
			if (d_collection[i].samples.size() > 0)
			{
				sp->setdataset(d_collection[i].samples);
				sp->calcxa(trust);
				sp->genhistogramdata();
				d_collection[i].sxa = sp->getxa();
				d_collection[i].sdelta = sp->getdelta();
				
				sp->retreivehistogram(d_collection[i].hist);
				sp->retreivenormaldist(d_collection[i].nrm);
				
			}
		}
		m.unlock();
		refreshqtui();
	}
}

void bShowHistClick()
{
	if (drawmode == 0) { drawmode = 1; printtolog(pteLog,"Set to draw histogram"); }
	else { drawmode = 0; printtolog(pteLog, "Set to draw scheme");	}
	needrefresh = true;
}

void bUseCamSettingsClick()
{
	camr = edCamPos->text().toDouble();
	camang = edCamAng->text().toDouble();
	needrefresh = true;
}

//non-connected UI funcs
void printtolog(QPlainTextEdit *pp, QString txt)
{
	pp->insertPlainText(txt+'\n');
}

void refreshqtui()
{
	
	lMeas->clear();
	lSamp->clear();

	if (d_collection.size() > 0)
	{
		for (int i = 0; i < d_collection.size(); i++)
		{			
			new QListWidgetItem("M "+ inttoqs(i) + ": " +
								d_collection[i].name + " = "+
								dbltoqs(d_collection[i].sxa) + "+-" + dbltoqs(d_collection[i].sdelta),
								lMeas);
		}
		if ((selmeas >= 0) && (selmeas < d_collection.size()))
		{
			if (d_collection[selmeas].samples.size() > 0)
			{
				for (int i = 0; i < d_collection[selmeas].samples.size(); i++)
				{
					new QListWidgetItem(
						"Smp " + inttoqs(i) + 
						"<"+ dbltoqs(d_collection[selmeas].nominal) +">: " +
						dbltoqs(d_collection[selmeas].samples[i]),
						lSamp);
				}
			}
			edMeasName->setText(d_collection[selmeas].name);
			measPosX->setText(dbltoqs(d_collection[selmeas].x));
			measPosY->setText(dbltoqs(d_collection[selmeas].y));
			measPosZ->setText(dbltoqs(d_collection[selmeas].z));
			measRotP->setText(dbltoqs(d_collection[selmeas].ap));
			measRotR->setText(dbltoqs(d_collection[selmeas].ar));
			measRotY->setText(dbltoqs(d_collection[selmeas].ay));
			edNominalValue->setText(dbltoqs(d_collection[selmeas].nominal));
		}
	}	

	lMeas->setCurrentRow(selmeas);
	lSamp->setCurrentRow(selsample);

}

//data functions
QString dbltoqs(double val)
{	
	return QString::fromStdString(to_string(val));
}

QString inttoqs(int val)
{
	return QString::fromStdString(to_string(val));
}


void addmeasurement(vmeasurement vm)
{
	d_collection.push_back(vm);
	printtolog(pteLog,"Meas added");
	refreshqtui();
}

void removemeasurement(int id)
{
	if ((id >= 0) && (id < d_collection.size()))
	{
		if (id < d_collection.size() - 1)
		for (int i = id; i < d_collection.size() - 1; i++)
		{
			d_collection[i] = d_collection[i + 1];
		}
		d_collection.pop_back();
	}
	needrefresh = true;
	refreshqtui();
}

void addsample(int id, double sm)
{
	if (d_collection.size() > 0)
	{
		if ((id >= 0) && (id < d_collection.size()))
		{
			d_collection[id].samples.push_back(sm);
			printtolog(pteLog, "Sample added");
			refreshqtui();
		}
	}
}

void removesample(int mid, int id)
{
	if (d_collection.size() > 0)
	{
		if ((mid >= 0) && (mid < d_collection.size()))
		{
			if (d_collection[mid].samples.size()>0)
			if ((id >= 0) && (id < d_collection[mid].samples.size()))
			{
				if (id < d_collection[mid].samples.size() - 1)
				for (int i = id; i < d_collection[mid].samples.size() - 1; i++)
				{
					d_collection[mid].samples[i] = d_collection[mid].samples[i + 1];
				}
				d_collection[mid].samples.pop_back();
			}
		}
	}
	refreshqtui();
}

//GL FUNCS

void RenderString(float x, float y, void *font, string tstring, float r, float g, float b)
{

	glColor3f(r,g,b);
	glRasterPos2f(x, y);

	glutBitmapString(font, (const unsigned char*)tstring.c_str());
}

void oglrenderscheme()
{
	int dcsw = d_collection.size();
	if (dcsw > 0)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60,16/9,0,100);
		
		glm::vec3 cameraPos = glm::vec3(camr*cos(camang*3.14 / 180), camr*sin(camang*3.14 / 180), 40);
		glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
		glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

		gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, 
			cameraTarget.x, cameraTarget.y, cameraTarget.z,
			cameraUp.x, cameraUp.y, cameraUp.z);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBegin(GL_LINES);

		glColor3d(1,0,0);

		glVertex3i(-10,0,0);
		glVertex3i(10, 0, 0);

		glVertex3i(9, 1, 0);
		glVertex3i(10, 0, 0);

		glColor3d(0, 1, 0);

		glVertex3i(0, -10, 0);
		glVertex3i(0, 10, 0);

		glVertex3i(0, 9, 1);
		glVertex3i(0, 10, 0);

		glColor3d(0, 0, 1);

		glVertex3i(0, 0, -10);
		glVertex3i(0, 0, 10);

		glVertex3i(1, 0, 9);
		glVertex3i(0, 0, 10);

		glEnd();

		for (int i = 0; i < dcsw; i++)
		{
			vmeasurement cds = d_collection[i];
			
			glPushMatrix();

			glTranslated(cds.x, cds.y, cds.z);
			glRotated(cds.ap, 1,0,0);
			glRotated(cds.ar, 0, 1, 0);
			glRotated(cds.ay, 0, 0, 1);
			
			glBegin(GL_LINES);

			if (i == selmeas)
			{
				glLineWidth(2);
				glColor3d(0, 1, 1);
			}
			else
			{
				glLineWidth(1);
				glColor3d(1, 0.5, 0);
			}

			glVertex3d(-cds.nominal / 2, 0, 0);
			glVertex3d(cds.nominal / 2, 0, 0);

			string so = to_string(cds.sxa) + "+-" + to_string(cds.sdelta);
			RenderString(0,0, GLUT_BITMAP_HELVETICA_10,so,1,1,0);

			glEnd();		

			glPopMatrix();
		}
	}
}

void oglrenderblock(vector<int> hg, vector<double>nor)
{
	m.lock();
	int hgs = 0;
	int hgm = 100;
	int hgl = 0; //always 0
	double bw = 10;
	double ow = 100;
	int norsize = 0;

	if (hg.size() > 0)
	{
		hgs = hg.size();
		hgm = hg[0];
		for (int val : hg)
		{
			if (val > hgm) hgm = val;			
		}
		ow = hgs * 10;
		bw = ow / hgs;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, ow, 0, hgm);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//printtolog(pteLog, "Building histogram");
	//printtolog(pteLog, "ow = "+dbltoqs(ow) + "; bw = " + dbltoqs(bw));
	//printtolog(pteLog, "hgm = " + dbltoqs(hgm) + "; hgs = " + dbltoqs(hgs));

	if (hgs > 0)
	{
		for (int i = 0; i < hgs; i++)
		{
			glBegin(GL_QUADS);

			switch (i)
			{
			case 0: glColor3d(1, 0, 0); break;
			case 1: glColor3d(0, 1, 0); break;
			case 2: glColor3d(0, 0, 1); break;
			case 3: glColor3d(1, 1, 0); break;
			case 4: glColor3d(0, 1, 1); break;
			case 5: glColor3d(1, 0, 1); break;
			case 6: glColor3d(1, 0.5, 0); break;
			case 7: glColor3d(0, 1, 0.5); break;
			case 8: glColor3d(0.5, 0, 1); break;
			case 9: glColor3d(1, 0.5, 0.5); break;
			default: glColor3d(0.5, 0.5, 0.5); break;
			}

			glVertex2d(i*bw+0.1*bw, 0);
			glVertex2d(i*bw + 0.9*bw, 0);

			switch (i)
			{
			case 0: glColor3d(.8, 0, 0); break;
			case 1: glColor3d(0, .8, 0); break;
			case 2: glColor3d(0, 0, .8); break;
			case 3: glColor3d(.8, .8, 0); break;
			case 4: glColor3d(0, .8, .8); break;
			case 5: glColor3d(.9, 0, .8); break;
			case 6: glColor3d(.9, 0.3, 0); break;
			case 7: glColor3d(0, .8, 0.3); break;
			case 8: glColor3d(0.3, 0, .8); break;
			case 9: glColor3d(.9, 0.3, 0.3); break;
			default: glColor3d(0.3, 0.3, 0.3); break;
			}
			
			if (hg[i] > 0)
			{
				glVertex2d(i*bw + 0.9*bw, hg[i]);
				glVertex2d(i*bw + 0.1*bw, hg[i]);
			}
			else
			{
				glVertex2d(i*bw + 0.9*bw, 0.05*hgm);
				glVertex2d(i*bw + 0.1*bw, 0.05*hgm);
			}

			printtolog(pteLog, inttoqs(i)+") "+inttoqs(hg[i]));

			glEnd();
		}
	}
	else
	{
		glBegin(GL_QUADS);

		glColor3d(1, 0, 0);
		glVertex2d(1, 1);
		glVertex2d(ow-1, 1);
		glVertex2d(ow-1, hgm-1);
		glVertex2d(1, hgm-1);

		glEnd();
	}

	if (nor.size() > 0)
	{
		norsize = nor.size();
		if (norsize > 0)
		{
			double ww = nor[0];
			double norm = nor[1];
			glColor3d(1, 1, 1);
			glLineWidth(4);
			glBegin(GL_LINE_STRIP);
			for (int i = 1; i < norsize; i++)
			{
				if (nor[i] > norm) norm = nor[i];
			}
			double nk = norm / hgm;
			for (int i = 1; i < norsize; i++)
			{
				glVertex2d(i*ww*10,nor[i]/nk);
				
			}
			glEnd();
		}
	}
	m.unlock();
}


void cbIdle()
{
	if (needrefresh) {
		needrefresh = false;
		glutPostRedisplay();
	}
}

void cbReshape(int ww, int wh)
{
	glViewport(0, 0, ww, wh);
	cww = ww; cwh = wh;
}

void cbDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1, 1, -1, 1); //temp ortho

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (drawmode == 0)
	{
		/*glPushMatrix();

		glRotatef(ang, 0, 0, 1);

		glBegin(GL_TRIANGLES);

		glColor3d(1, 0, 0);
		glVertex2d(0, 0.5);
		glColor3d(0, 1, 0);
		glVertex2d(0.5, -0.5);
		glColor3d(0, 0, 1);
		glVertex2d(-0.5, -0.5);

		glEnd();

		glPopMatrix();*/
		oglrenderscheme();
	}

	if (drawmode == 1)
	{
		oglrenderblock(cmeas.hist, cmeas.nrm);
	}

	glutSwapBuffers();

}


void initGLThread(int argc, char *argv[])
{
	cww = 800; cwh = 600;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(cww, cwh);
	glutCreateWindow("Render Output");

	glutIdleFunc(cbIdle);
	glutDisplayFunc(cbDisplay);
	glutReshapeFunc(cbReshape);

	glutMainLoop();
}


//MAIN
int main(int argc, char *argv[])
{
    //init and run GLUT thread
	std::thread glt(initGLThread, argc, argv);
	glt.detach();

	//do the needed qt stuff
	QApplication a(argc, argv);
	qt_entity_analysis w;

    w.show();
	
	//define ui element variables
	QPushButton * bAddMeas = w.window()->findChild<QPushButton*>("bAddMeas");
	QPushButton * bRemMeas = w.window()->findChild<QPushButton*>("bRemMeas");
	QPushButton * bAddSamp = w.window()->findChild<QPushButton*>("bAddSamp");
	QPushButton * bRemSamp = w.window()->findChild<QPushButton*>("bRemSamp");
	QPushButton * bProcess = w.window()->findChild<QPushButton*>("bProcess");
	QPushButton * bShowHist = w.window()->findChild<QPushButton*>("bShowHist");
	QPushButton * bUseCamSettings = w.window()->findChild<QPushButton*>("bUseCamSettings");
	
	edMeasName = w.window()->findChild<QLineEdit*>("edMeasName");
	edValue = w.window()->findChild<QLineEdit*>("edValue");
	ebdValueAvg = w.window()->findChild<QLineEdit*>("ebdValueAvg");
	edValueWeig = w.window()->findChild<QLineEdit*>("edValueWeig");
	edCamPos = w.window()->findChild<QLineEdit*>("edCamPos");
	edCamAng = w.window()->findChild<QLineEdit*>("edCamAng");

	edNominalValue = w.window()->findChild<QLineEdit*>("edNominalValue");
	measPosX = w.window()->findChild<QLineEdit*>("measPosX");
	measPosX->setStyleSheet("QLineEdit { background-color : white; color : red; }");
	measPosY = w.window()->findChild<QLineEdit*>("measPosY");
	measPosY->setStyleSheet("QLineEdit { background-color : white; color : green; }");
	measPosZ = w.window()->findChild<QLineEdit*>("measPosZ");
	measPosZ->setStyleSheet("QLineEdit { background-color : white; color : blue; }");
	measRotP = w.window()->findChild<QLineEdit*>("measRotP");
	measRotP->setStyleSheet("QLineEdit { background-color : white; color : red; }");
	measRotR = w.window()->findChild<QLineEdit*>("measRotR");
	measRotR->setStyleSheet("QLineEdit { background-color : white; color : green; }");
	measRotY = w.window()->findChild<QLineEdit*>("measRotY");
	measRotY->setStyleSheet("QLineEdit { background-color : white; color : blue; }");

	lMeas = w.window()->findChild<QListWidget*>("lMeas");
	lSamp = w.window()->findChild<QListWidget*>("lSamp");
	pteLog = w.window()->findChild<QPlainTextEdit*>("pteLogger");

	//make connections to ui element events
	QObject::connect(bAddMeas, &QPushButton::clicked, bAddMeasClick);
	QObject::connect(bRemMeas, &QPushButton::clicked, bRemoveMeasClick);
	QObject::connect(bAddSamp, &QPushButton::clicked, bAddSampleClick);
	QObject::connect(bRemSamp, &QPushButton::clicked, bRemSampleClick);
	QObject::connect(bProcess, &QPushButton::clicked, bProcessClick);
	QObject::connect(bShowHist, &QPushButton::clicked, bShowHistClick);
	QObject::connect(bUseCamSettings, &QPushButton::clicked, bUseCamSettingsClick);
	QObject::connect(lMeas, &QListWidget::clicked, lMeasClick);
	QObject::connect(lSamp, &QListWidget::clicked, lSampClick);

	printtolog(pteLog, "test\n");

    return a.exec();
}
