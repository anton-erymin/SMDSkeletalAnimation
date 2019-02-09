////////////////////////////////////////////////////////////////////////////
// Author:		Anton Erymin										
// E-mail:		anton-erymin@mail.ru
// Version:		0.1
// Date:         10 October 2007
// Description:	Library for working with SMD(Half-Life) format file.
////////////////////////////////////////////////////////////////////////////

#ifndef smdmodel_h
#define smdmodel_h

#include "stdafx.h"
#include <stdio.h>

#include "vectors.h"
#include "str.h"
#include "matrices.h"

#define buflen	128

typedef struct
{
	float v[3];
} SMDVector;

typedef struct
{
	unsigned short node;
	float vertex[3], normal[3], texturecoords[2];
} SMDVertex;

typedef struct
{
	char texture[64];
	SMDVertex verts[3];
} SMDTriangle;

typedef struct
{
	SMDVector *rotation, *translation;
} SMDFrame;

typedef struct
{
	Matrix relative, absolute;
	SMDVector position;
	bool calculated;
} SMDNode;

void skip(FILE *f)
{
	char c;
	fread(&c, 1, 1, f);
	while (c != '"')
	{
		fread(&c, 1, 1, f);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class SMDAnimation
{
public:
	unsigned short numFrames;
	unsigned short numNodes;
	SMDFrame *frames;

public:
	SMDAnimation(unsigned short nodes);
	~SMDAnimation();
	bool LoadFromFile(char *filename);
};

SMDAnimation::SMDAnimation(unsigned short nodes)
{
	numFrames = 0;
	numNodes = nodes;
	frames = NULL;
};

SMDAnimation::~SMDAnimation()
{
	delete[] frames;
};

bool SMDAnimation::LoadFromFile(char *filename)
{
	FILE *f = fopen(filename, "rb");
	
	char s[64];

	ReadString(f, s);
	ReadString(f, s);
	ReadString(f, s);

	for (int i = 0; i < numNodes; i++)
	{
		ReadString(f, s);
		skip(f);
		skip(f);
		ReadString(f, s);
	}

	ReadString(f, s);
	ReadString(f, s);

	ReadString(f, s);
	while (strncmp(s, "end", 3) != 0)
	{
		if (strncmp(s, "time", 4) == 0) numFrames++;		
		ReadString(f, s);
	}

	rewind(f);

	frames = new SMDFrame[numFrames];
	for (int i = 0; i < numFrames; i++)
	{
		frames[i].rotation = new SMDVector[numNodes];
		frames[i].translation = new SMDVector[numNodes];
	}

	ReadString(f, s);
	ReadString(f, s);
	ReadString(f, s);

	for (int i = 0; i < numNodes; i++)
	{
		ReadString(f, s);
		skip(f);
		skip(f);
		ReadString(f, s);
	}

	ReadString(f, s);
	ReadString(f, s);
	ReadString(f, s);
	int node;

	int iframe = 0;

	while (strncmp(s, "end", 3) != 0)
	{
		if (strncmp(s, "time", 4) == 0) 
		{
			ReadString(f, s);
			ReadString(f, s);
			while (strncmp(s, "time", 4) != 0)
			{						
				node = atoi(s);

				ReadString(f, s);
				frames[iframe].translation[node].v[0] = atof(s);
				ReadString(f, s);
				frames[iframe].translation[node].v[1] = atof(s);
				ReadString(f, s);
				frames[iframe].translation[node].v[2] = atof(s);

				ReadString(f, s);
				frames[iframe].rotation[node].v[0] = atof(s);
				ReadString(f, s);
				frames[iframe].rotation[node].v[1] = atof(s);
				ReadString(f, s);
				frames[iframe].rotation[node].v[2] = atof(s);

				ReadString(f, s);
				if (strncmp(s, "end", 3) == 0) break;
			}
		}
		iframe++;
	}


	fclose(f);

	return true;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class SMDModel
{
private:	
	unsigned short numTriangles;
	int *links;	
	SMDFrame frame;
	SMDTriangle *triangles;
	SMDTriangle *converted;
	SMDNode *nodes;
	
	GLuint tex;

public:
	unsigned short numNodes;
	SMDAnimation *currentAnim;
	int curframe;

	SMDModel();
	~SMDModel();
	bool LoadFromFile(char *filename);
	void Draw(float interp);
	void Convert();
	Matrix *NewCalcSk(int numframe);
	void Calc(int numframe, float interp);
	void NewDrawSk();
	void Interpolate(float interp, float *v1, float *v2, float *v);
};

SMDModel::SMDModel()
{
	numNodes = 0;
	numTriangles = 0;

	links = NULL;
	triangles = NULL;
	nodes = NULL;
	currentAnim = NULL;
};

SMDModel::~SMDModel()
{
	delete[] links;
	delete[] triangles;
	delete[] nodes;
	currentAnim = NULL;
};

bool SMDModel::LoadFromFile(char *filename)
{
	FILE *f = fopen(filename, "rt");
	if (!f)
	{
		MessageBoxA(0, "Failed to open file", "SMD Loader", MB_ICONERROR);
		return false;
	}

	char s[buflen];

	fgets(s, buflen, f);
	if (strncmp(s, "version 1", 9) != 0) 
	{
		MessageBoxA(0, "Invalid format file", "SMD Loader", MB_ICONERROR);
		return false;
	}

	fgets(s, buflen, f);
	if (strncmp(s, "nodes", 5) != 0) 
	{
		MessageBoxA(0, "Invalid format file", "SMD Loader", MB_ICONERROR);
		return false;
	}

	fgets(s, buflen, f);
	while (strncmp(s, "end", 3) != 0)
	{
		numNodes++;
		fgets(s, buflen, f);
	}

	fgets(s, buflen, f);
	fgets(s, buflen, f);
	for (int i = 0; i < numNodes; i++)
	{
		fgets(s, buflen, f);
	}

	fgets(s, buflen, f);
	fgets(s, buflen, f);

	fgets(s, buflen, f);
	while (strncmp(s, "end", 3) != 0)
	{
		numTriangles++;
		fgets(s, buflen, f);
		fgets(s, buflen, f);
		fgets(s, buflen, f);
		fgets(s, buflen, f);
	}

	rewind(f);

	fgets(s, buflen, f);
	fgets(s, buflen, f);
	
	links = new int[numNodes];
	for (int i = 0; i < numNodes; i++)
	{
		ReadString(f, s);
		skip(f);
		skip(f);
		ReadString(f, s);
		links[i] = atoi(s);
	}

	fgets(s, buflen, f);
	fgets(s, buflen, f);
	fgets(s, buflen, f);

	frame.translation = new SMDVector[numNodes];
	frame.rotation = new SMDVector[numNodes];

	for (int i = 0; i < numNodes; i++)
	{
		ReadString(f, s);

		ReadString(f, s);
		frame.translation[i].v[0] = atof(s);
		ReadString(f, s);
		frame.translation[i].v[1] = atof(s);
		ReadString(f, s);
		frame.translation[i].v[2] = atof(s);
		
		ReadString(f, s);
		frame.rotation[i].v[0] = atof(s);
		ReadString(f, s);
		frame.rotation[i].v[1] = atof(s);
		ReadString(f, s);
		frame.rotation[i].v[2] = atof(s);
	}

	fgets(s, buflen, f);
	fgets(s, buflen, f);

	triangles = new SMDTriangle[numTriangles];
	for (int i = 0; i < numTriangles; i++)
	{
		ReadString(f, s);
		strcpy(triangles[i].texture, s);
		for (int j = 0; j < 3; j++)
		{
			ReadString(f, s);
			triangles[i].verts[j].node = atoi(s);
			
			ReadString(f, s);
			triangles[i].verts[j].vertex[0] = atof(s);
			ReadString(f, s);
			triangles[i].verts[j].vertex[1] = atof(s);
			ReadString(f, s);
			triangles[i].verts[j].vertex[2] = atof(s);

			ReadString(f, s);
			triangles[i].verts[j].normal[0] = atof(s);
			ReadString(f, s);
			triangles[i].verts[j].normal[1] = atof(s);
			ReadString(f, s);
			triangles[i].verts[j].normal[2] = atof(s);

			ReadString(f, s);
			triangles[i].verts[j].texturecoords[0] = atof(s);
			ReadString(f, s);
			triangles[i].verts[j].texturecoords[1] = atof(s);

		}
	}

	fclose(f);

	

	//LoadTexture(triangles[0].texture, &tex);

	//glEnable(GL_TEXTURE_2D);

	this->NewCalcSk(0);
	this->Convert();

	return true;
};


void SMDModel::Draw(float interp)
{
	if (currentAnim) this->Calc(curframe, interp);

	for (int i = 0; i < numTriangles; i++)
	{
		triangles[i] = converted[i];
		for (int j = 0; j < 3; j++)
		{
			MatrixApply(nodes[triangles[i].verts[j].node].absolute, 
							   &triangles[i].verts[j].vertex[0], 
							   &triangles[i].verts[j].vertex[1], 
							   &triangles[i].verts[j].vertex[2]);
			MatrixApply(nodes[triangles[i].verts[j].node].absolute, 
							   &triangles[i].verts[j].normal[0], 
							   &triangles[i].verts[j].normal[1], 
							   &triangles[i].verts[j].normal[2]);
		}
	}

	glLineWidth(0.1f);
	glBegin(GL_TRIANGLES);
		for (int i = 0; i < numTriangles; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				glNormal3fv(triangles[i].verts[j].normal);
				glTexCoord2fv(triangles[i].verts[j].texturecoords);
				glVertex3fv(triangles[i].verts[j].vertex);
			}
		}
	glEnd(); 
};

void SMDModel::Convert()
{
	converted = new SMDTriangle[numTriangles];
	for (int i = 0; i < numTriangles; i++)
	{
		converted[i] = triangles[i];
		for (int j = 0; j < 3; j++)
		{
			MatrixInvApply(nodes[converted[i].verts[j].node].absolute, 
									&converted[i].verts[j].vertex[0],
									&converted[i].verts[j].vertex[1],
									&converted[i].verts[j].vertex[2]);
			MatrixInvApply(nodes[converted[i].verts[j].node].absolute, 
									&converted[i].verts[j].normal[0],
									&converted[i].verts[j].normal[1],
									&converted[i].verts[j].normal[2]);
		}
	}
};

Matrix *SMDModel::NewCalcSk(int numframe)
{
	nodes = new SMDNode[numNodes];

	Matrix *res = new Matrix[numNodes];

	SMDVector rot, trans;

	for (int i = 0; i < numNodes; i++)
	{
		if (!currentAnim)
		{
			rot = frame.rotation[i];
			trans = frame.translation[i];
		}
		else 
		{
			rot = currentAnim->frames[numframe].rotation[i];
			trans = currentAnim->frames[numframe].translation[i];
		}

		nodes[i].calculated = false;

		Matrix R;
		MatrixRotation(&nodes[i].relative, rot.v[0], -rot.v[1], rot.v[2]);

		nodes[i].relative.m[3] = trans.v[0];
		nodes[i].relative.m[7] = trans.v[1];
		nodes[i].relative.m[11] = trans.v[2];

		if (links[i] == -1)
		{
			nodes[i].absolute = nodes[i].relative;
			nodes[i].calculated = true;
		}
	}

	bool flag = true;
	while (flag)
	{
		flag = false;

		for (int i = 0; i < numNodes; i++)
		{
			if (!nodes[i].calculated)
			{
				if (nodes[links[i]].calculated)
				{
					MatrixMult(&nodes[i].absolute, &nodes[links[i]].absolute, &nodes[i].relative);
					nodes[i].calculated = true;
					res[i] = nodes[i].absolute;
				}
				else flag = true;
			}
		}
	}
	
	/*for (int i = 0; i < numNodes; i++)
	{
		nodes[i].position.v[0] = 0.0f;
		nodes[i].position.v[1] = 0.0f;
		nodes[i].position.v[2] = 0.0f;

		MatrixApply(nodes[i].absolute, &nodes[i].position.v[0], &nodes[i].position.v[1], &nodes[i].position.v[2]);
	}*/

	return res;
};

void SMDModel::NewDrawSk()
{
	if (currentAnim) this->NewCalcSk(curframe);

	glPointSize(7);
	glLineWidth(2.0f);
	glBegin(GL_POINTS);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (int i = 0; i < numNodes; i++)
		{
			glVertex3fv(nodes[i].position.v);
		}
	glEnd();

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 0.0f);
	for (int i = 0; i < numNodes; i++)
		if (links[i] != -1)
		{
			glVertex3fv(nodes[i].position.v);
			glVertex3fv(nodes[links[i]].position.v);
		}
	glEnd();
};

void SMDModel::Calc(int numframe, float interp)
{
	Matrix *matrices1, *matrices2;
	
	matrices1 = this->NewCalcSk(numframe);
	if (numframe < currentAnim->numFrames - 1)
		matrices2 = this->NewCalcSk(numframe + 1);
	else matrices2 = matrices1;

	for (int i = 0; i < numNodes; i++)
	{
		MatrixIdentity(&nodes[i].absolute);
		MatrixInterpolate(interp, matrices1[i], matrices2[i], &nodes[i].absolute);

	}

};
#endif